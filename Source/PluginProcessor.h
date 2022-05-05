#pragma once
//预编译逻辑控制
#define _USE_MATH_DEFINES
#define _OPEN_FILTERS true
#define _OPEN_PEAK_PITCH true
#define _OPEN_WAHWAH false
#define _OPEN_DYNAMICS true
#define _OPEN_TEST false

#if _OPEN_PEAK_PITCH
#define _SHOW_SPEC true
#define USE_3rdPARTYPITCHSHIFT true

#if USE_3rdPARTYPITCHSHIFT
#define USE_RUBBERBAND true
#define USE_SOUNDTOUCH true
#endif
#endif

#include<juce_audio_processors/juce_audio_processors.h>
#include <JuceHeader.h>
#include"juce_audio_plugin_client//Standalone//juce_StandaloneFilterWindow.h"
#include"SoundTouch.h"
#include"PitchShifter.h"
#include"PeakShifter.h"
//#include"PluginFilter.h"
// #include "ManVoiceFilter.h"
//#include "ShapeInvariantPitchShifter.h"
#include"rubberband/RubberBandStretcher.h"
#include"rubberband/rubberband-c.h"
#include"PitchShifterRubberband.h"
#include"PitchShifterSoundTouch.h"
#include"TransportInformation.h"
#include"FIFO.h"
#include"TrainingTemplate.h"
#include"Analyser.h"
#include "EngineHelpers.h"
#include "ExtendedUIBehaviour.h"
#include"SocialButtons.h"
#include"VocoderForVoiceConversion.h"
#include"VoiceConversionBuffer.h"

#include"Utilities.h"
//混响参数
namespace ParamNames
{
    const juce::String size{ "reverbSize" };
    const juce::String damp{ "reverbDamp" };
    const juce::String width{ "reverbWidth" };
    const juce::String dryWet{ "reverbDw" };
    const juce::String freeze{ "reverbRreeze" };
}
//总构造函数
class VoiceChanger_wczAudioProcessor :
	 public juce::AudioProcessor//,public juce::AudioAppComponent//,public Filter
    ,public TransportInformation
    ,public juce::AudioProcessorValueTreeState::Listener
	,public juce::ChangeBroadcaster
{

public:
    //总控逻辑
    std::atomic<bool> isModelLoaded{ false }; //是否已经加载了模型
    std::atomic<bool> openReverb{ false };//混响开关
    std::atomic<bool> isDawStream{ false };//是否用了子引擎音频流
    std::atomic<bool> isInternalRecording{ false };//是否在内录状态
    std::atomic<bool> useFD{ false };//是否使用频域处理音调缩放
    //子引擎输入控制
    static void setupInputs(tracktion_engine::Edit& edit)
    {
        auto& dm = edit.engine.getDeviceManager();

        for (int i = 0; i < dm.getNumMidiInDevices(); i++)
        {
            auto dev = dm.getMidiInDevice(i);
            dev->setEnabled(true);
            dev->setEndToEndEnabled(true);
        }

        edit.playInStopEnabled = true;
        edit.getTransport().ensureContextAllocated(true);

        // 将MIDI输入添加到轨道1
        if (auto t = EngineHelpers::getOrInsertAudioTrackAt(edit, 0))
            if (auto dev = dm.getMidiInDevice(0))
                for (auto instance : edit.getAllInputDevices())
                    if (&instance->getInputDevice() == dev)
                        instance->setTargetTrack(*t, 0, true);

        // 将相同的midi输入添加到轨道2
        if (auto t = EngineHelpers::getOrInsertAudioTrackAt(edit, 1))
            if (auto dev = dm.getMidiInDevice(0))
                for (auto instance : edit.getAllInputDevices())
                    if (&instance->getInputDevice() == dev)
                        instance->setTargetTrack(*t, 0, false);


        edit.restartPlayback();
    }
    //子引擎输出控制
    static void setupOutputs(tracktion_engine::Edit& edit)
    {
        auto& dm = edit.engine.getDeviceManager();

        for (int i = 0; i < dm.getNumMidiOutDevices(); i++)
        {
            auto dev = dm.getMidiOutDevice(i);
            dev->setEnabled(true);
        }

        edit.playInStopEnabled = true;
        edit.getTransport().ensureContextAllocated(true);

        // Set track 2 to send to midi output
        if (auto t = EngineHelpers::getOrInsertAudioTrackAt(edit, 1))
        {
            auto& output = t->getOutput();
            output.setOutputToDefaultDevice(true);
        }

        edit.restartPlayback();
    }

    //子引擎IO设备控制
    class PluginEngineBehaviour : public tracktion_engine::EngineBehaviour
    {
    public:
        bool autoInitialiseDeviceManager() override { return false; }
    };

    //==============================================================================
    //编辑器子引擎封装模块
    struct EngineWrapper
    {
        EngineWrapper()
            : audioInterface(engine.getDeviceManager().getHostedAudioDeviceInterface())
        {
            JUCE_ASSERT_MESSAGE_THREAD
                audioInterface.initialise({});

            setupInputs(edit);
            setupOutputs(edit);
        }

        tracktion_engine::Engine engine{ ProjectInfo::projectName, std::make_unique<ExtendedUIBehaviour>(), std::make_unique<PluginEngineBehaviour>() };
        tracktion_engine::Edit edit{ engine, tracktion_engine::createEmptyEdit(engine), tracktion_engine::Edit::forEditing, nullptr, 0 };
        tracktion_engine::TransportControl& transport{ edit.getTransport() };
        tracktion_engine::HostedAudioDeviceInterface& audioInterface;
        tracktion_engine::ExternalPlayheadSynchroniser playheadSynchroniser{ edit };
    };

    template<typename Function>
    void callFunctionOnMessageThread(Function&& func)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            func();
        }
        else
        {
            jassert(!MessageManager::getInstance()->currentThreadHasLockedMessageManager());
            WaitableEvent finishedSignal;
            MessageManager::callAsync([&]
                {
                    func();
                    finishedSignal.signal();
                });
            finishedSignal.wait(-1);
        }
    }

    void ensureEngineCreatedOnMessageThread()
    {
        if (!engineWrapper)
            callFunctionOnMessageThread([&] { engineWrapper = std::make_unique<EngineWrapper>(); });
    }

    void ensurePrepareToPlayCalledOnMessageThread(double sampleRate, int expectedBlockSize)
    {
        jassert(engineWrapper);
        callFunctionOnMessageThread([&] { engineWrapper->audioInterface.prepareToPlay(sampleRate, expectedBlockSize); });
    }

    std::unique_ptr<EngineWrapper> engineWrapper;



//====================================================================================

public:

    //内录模块
    void stop();
    CriticalSection modelWriterLock;
    CriticalSection writerLock;
    std::atomic<bool> openVoiceConversion{ false };
    // bool isInternalRecording{ false };
    TimeSliceThread internalWriteThread;
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
    File parentDir = File::getSpecialLocation(File::userDesktopDirectory);
    File lastRecording = parentDir.getNonexistentChildFile("VoiceChanger_wcz Recording", ".wav");

	//ScopedPointer<FileOutputStream> internalRecordingStream;
    //ScopedPointer<AudioFormatWriter> internalRecordWriter;
    // ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedInternalRecording;
    void startRecording(const File& file);
    void stopRecording();

    //=============================================================================================
    //均衡器模块参数
	enum FilterType
    {
        NoFilter = 0,
        HighPass,
        HighPass1st,
        LowShelf,
        BandPass,
        AllPass,
        AllPass1st,
        Notch,
        Peak,
        HighShelf,
        LowPass1st,
        LowPass,
        LastFilterID
    };
    // 变量索引名称
    static juce::String paramOutput;
    static juce::String paramType;
    static juce::String paramFrequency;
    static juce::String paramQuality;
    static juce::String paramGain;
    static juce::String paramActive;
    // 获取参数方法
    static juce::String getBandID(size_t index);
    static juce::String getTypeParamName(size_t index);
    static juce::String getFrequencyParamName(size_t index);
    static juce::String getQualityParamName(size_t index);
    static juce::String getGainParamName(size_t index);
    static juce::String getActiveParamName(size_t index);
public:
    //==============================================================================
    //总构造函数，以下为juce封装接口代码
    VoiceChanger_wczAudioProcessor();
    ~VoiceChanger_wczAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    // 总处理接口
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void overallProcess(juce::AudioBuffer<float>& buffer);
    // 参数树变化响应
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    // 获取参数树状态
    juce::AudioProcessorValueTreeState& getPluginState();

    // 获取均衡器单滤波器参数
    size_t getNumBands() const;
    juce::String getBandName(size_t index) const;
    juce::Colour getBandColour(size_t index) const;

    void setBandSolo(int index);
    bool getBandSolo(int index) const;
    
    static juce::StringArray getFilterTypeNames();
    // 处理器到编辑器接口
	juce::AudioProcessorEditor* createEditor() override;

    // 返回true
	bool hasEditor() const override;
    // 获取幅频响应给均衡分析器
    const std::vector<double>& getMagnitudes();
    // 计算频谱绘图
    void createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble);
    // 计算滤波器频响绘图
    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input);
    // 检查是否有新的频谱输出
    bool checkForNewAnalyserData();

    //=========================================================
    //JUCE系统接口
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;


    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;


    void  getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==========================================================
    juce::Point<int> getSavedSize() const;
    //编辑器界面设置
    void setSavedSize(const juce::Point<int>& size);

    // 获取主界面频谱
    juce::Image& getSpectrumView();
    // 设置音调缩放程度
    void setPitchShift(float pitch);
    //设置共振峰移动程度
    void setPeakShift(float peak);
    
    //设置动态调节参数
    void setDynamicsThresholdShift(float threshold);
    void setDynamicsRatioShift(float ratio);
    void setDynamicsAttackShift(float attack);
    void setDynamicsReleaseShift(float release);
    void setDynamicsMakeupGainShift(float makeupGain);

    // 控制离线音频播放位置
    void setPlayAudioFilePosition(float position);
    // 获取音调缩放参数
    float getPitchShift();
    //获取共振峰移动参数
    float getPeakShift();

    //获取动态处理参数
    float getDynamicsThresholdShift();
    float getDynamicsRatioShift();
    float getDynamicsAttackShift();
    float getDynamicsReleaseShift();
    float getDynamicsMakeupGainShift();
    // 均衡器单带
    //==============================================================================
    struct Band {
        Band(const juce::String& nameToUse, juce::Colour colourToUse, FilterType typeToUse,
            float frequencyToUse, float qualityToUse, float gainToUse = 1.0f, bool shouldBeActive = true)
            : name(nameToUse),
            colour(colourToUse),
            type(typeToUse),
            frequency(frequencyToUse),
            quality(qualityToUse),
            gain(gainToUse),
            active(shouldBeActive)
        {}
        // 单个滤波器应有的参数
        juce::String name;
        juce::Colour colour;
        FilterType   type = BandPass;
        float        frequency = 1000.0f;
        float        quality = 1.0f;
        float        gain = 1.0f;
        bool         active = true;
        std::vector<double> magnitudes;
    };
    // 获取实例指针
    Band* getBand(size_t index);
    // 获取单带滤波器是第几个
    int getBandIndexFromID(juce::String paramID);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceChanger_wczAudioProcessor);
    // 更新IIR滤波器参数
    void updateBand(const size_t index);
    // 更新旁通状态
    void updateBypassedStates();
    // 更新幅频响应曲线
    void updatePlots();
    // 撤回操作
    juce::UndoManager                  undo;


    juce::AudioProcessorValueTreeState state;
#if _OPEN_FILTERS
    std::vector<Band>    bands; // 创建多个band实例

    std::vector<double> frequencies; // 创建滤波器频率参数
    std::vector<double> magnitudes; // 滤波器幅度参数

    bool wasBypassed = true; //是否被旁通
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using Gain = juce::dsp::Gain<float>;
    // 均衡器DSP处理链条
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> filter;
    


    double sampleRate = 44100;

    int soloed = -1;

    Analyser<float> inputAnalyser; // 输入频谱分析器
    Analyser<float> outputAnalyser; //输出频谱分析器


    juce::Point<int> editorSize = { 900, 500 }; // 默认均衡器界面大小
#endif

    //================================================================================
public:
    //动态处理参数组
#if _OPEN_DYNAMICS 
    juce::AudioBuffer<float> mixedDownInputDynamics;
    float xlDynamics;
    float ylDynamics;
    float xgDynamics;
    float ygDynamics;
    float controlDynamics;
    float inputLevelDynamics;
    float ylPrevDynamics;
    float inverseEDynamics;
    float inverseSampleRateDynamics;
    float calculateAttackOrReleaseForDynamics(float value);
    void processDynamics(
        juce::AudioBuffer<float>& buffer
        ,bool isExpanderOrCompressor
        ,float threshold
        ,float ratio
        ,float attack
        ,float release
        ,float makeupGain
    );
#endif


public:
    int nPlayAudioFilePosition{ 0 };
    int nPlayAudioFileSampleNum{ 0 };
    bool realtimeMode{ true }; // 离线模式/实时模式切换

    juce::AudioFormatManager formatManager;// 用于将MP3/wav离线文件转换成音频流

    juce::AudioTransportSource transportSource; // 用于控制离线音流播放
    File currentlyLoadedFile; // 当前载入的文件
    AudioThumbnailCache thumbnailCache; //绘制离线音频缩略图
    ScopedPointer<AudioFormatReaderSource>currentAudioFileSource;//音频文件读取流
    TimeSliceThread readAheadThread;//离线文件的超前读取线程
    void loadFileIntoTransport(const File& audioFile);//将文件载入播放控制模块


    juce::AudioBuffer<float> fileBuffer;
    int innerRecordSampleCount{ 0 };
    juce::AudioBuffer<float> sourceBuffer;//存储源语音
    juce::AudioBuffer<float> targetBuffer;//存储目标语音
    
    HSMModel model;
    // TrainingTemplate trainingTemplate;
    // std::vector<float> voiceChangerParameter;
    juce::AudioBuffer<float>* pPlayBuffer;
    TransportInformation ti;
    void setTarget(TransportFileType ft)override;
    void setState(TransportState newState)override;
    float inputAudioFileLength{ 300.0f };
    void getNextAudioBlock(juce::AudioSourceChannelInfo& buffer);
    // int readFilePosition;
    bool shouldProcessFile{ false };
    bool canReadSampleBuffer{ false };
    //void alignBuffer(juce::AudioSampleBuffer& s, juce::AudioSampleBuffer& t);
    float getPlayAudioFilePosition();


#if _OPEN_WAHWAH//WAHWAH效果参数组
    juce::OwnedArray<Filter> filtersForWahWah;
    void processWahwah(juce::AudioBuffer<float>& buffer,
        float attackValue,
        float releaseValue,
        float maxLFOAndEnvelope,
        float lfoFrequency,
        float mixRatio,
        float filterQFactor,
        float filterGain,
        float filterFreq
    );
    float centerFrequencyForWahWah;
    float lfoPhaseForWahWah;
    float inverseSampleRateForWahWah;
    float twoPi;

    juce::Array<float> envelopesForWahWah;
    float inverseEForWahWah;
    float calculateAttackOrReleaseForWahWah(float value);

#endif
#if _OPEN_PEAK_PITCH
    juce::OwnedArray<PitchShifter>pitchShifters;// 音调缩放类的调用
    juce::OwnedArray<PeakShifter>peakShifters; //共振峰移动类的调用


	//juce::OwnedArray<VocoderForVoiceConversion> vocodersForVoiceConversion;



#endif
#if _OPEN_TEST
    juce::OwnedArray<ShapeInvariantPitchShifter> shapeInvariantPitchShifters;
#endif
    void updateUIControls();

private:
    // 音调缩放参数
    juce::AudioParameterFloat* nPitchShift{ 0 };
    juce::AudioParameterFloat* nPeakShift;
    // 动态处理参数
    juce::AudioParameterFloat* nDynamicsThreshold;
    juce::AudioParameterFloat* nDynamicsRatio;
    juce::AudioParameterFloat* nDynamicsAttack;
    juce::AudioParameterFloat* nDynamicsRelease;
    juce::AudioParameterFloat* nDynamicsMakeupGain;

    // 绘制频谱曲线
    void drawSpectrumGraph(juce::Image view, std::shared_ptr<float>level, juce::Colour colour, bool isLog);
    //void syncPluginParameter();
public:
    juce::Image spectrum;// 频谱绘画对象
private:
#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND
    std::unique_ptr<PitchShifterRubberband> rbs;// 频域处理调用接口
    const int rbOptions = RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;
#endif // USE_3rdPARTYPITCHSHIFT
#if USE_SOUNDTOUCH
    std::unique_ptr<PitchShifterSoundTouch> sts;//soundtouch调用接口
#endif
#endif
public:
    std::unique_ptr<VoiceConversionBuffer> vcb;// 有模板变声调用接口
    int samplesPerBlock;
private:
    //电平表模块参数
    juce::LinearSmoothedValue<float> gainLeft, gainRight;//左声道和右声道增益
    std::vector<juce::LinearSmoothedValue<float>>rmsLevels;//均方根电平
    Utility::Fifo rmsFifo;
    juce::AudioBuffer<float>rmsCalculationBuffer;//缓冲

    int rmsWindowSize = 50;

    int isSmoothed = true;
public:
    //电平表模块方法
    // juce::AudioProcessorValueTreeState& getApvts() { return parameters; }
    std::vector<float>getRmsLevels();
    float getRmsLevel(const int level);
    void processLevelValue(juce::LinearSmoothedValue<float>&, const float value)const;
    void processLevelInfo(juce::AudioBuffer<float>& buffer);
    //==============================================================================
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChanger_wczAudioProcessor)
private:
    //更新混响设置
    void updateReverbSettings();
    //混响参数存储
    dsp::Reverb::Parameters reverbParams;
    dsp::Reverb reverb;
};

