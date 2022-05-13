#include "PluginProcessor.h"

#include "modelSerialization.h"
#include "PluginEditor.h"
// 初始化均衡器参数
#if _OPEN_FILTERS
juce::String VoiceChanger_wczAudioProcessor::paramOutput("output");
juce::String VoiceChanger_wczAudioProcessor::paramType("type");
juce::String VoiceChanger_wczAudioProcessor::paramFrequency("frequency");
juce::String VoiceChanger_wczAudioProcessor::paramQuality("quality");
juce::String VoiceChanger_wczAudioProcessor::paramGain("gain");
juce::String VoiceChanger_wczAudioProcessor::paramActive("active");

namespace IDs
{
    juce::String editor{ "editor" };
    juce::String sizeX{ "size-x" };
    juce::String sizeY{ "size-y" };
}
// 根据滤波器序号返回ID（中文名字）
juce::String VoiceChanger_wczAudioProcessor::getBandID(size_t index)
{
    switch (index)
    {
    case 0: return juce::CharPointer_UTF8("\xe6\x9e\x81\xe4\xbd\x8e\xe9\xa2\x91");
    case 1: return juce::CharPointer_UTF8("\xe4\xbd\x8e\xe9\xa2\x91");
    case 2: return juce::CharPointer_UTF8("\xe4\xb8\xad\xe4\xbd\x8e\xe9\xa2\x91");
    case 3: return juce::CharPointer_UTF8("\xe4\xb8\xad\xe9\xab\x98\xe9\xa2\x91");
    case 4: return juce::CharPointer_UTF8("\xe9\xab\x98\xe9\xa2\x91");
    case 5: return juce::CharPointer_UTF8("\xe6\x9e\x81\xe9\xab\x98\xe9\xa2\x91");
    default: break;
    }
    return "unknown";
}
// 从ID推断出滤波器序号（数组下标）
int VoiceChanger_wczAudioProcessor::getBandIndexFromID(juce::String paramID)
{
    for (size_t i = 0; i < 6; ++i)
        if (paramID.startsWith(getBandID(i) + "-"))
            return int(i);

    return -1;
}
// 默认添加6组滤波器
std::vector<VoiceChanger_wczAudioProcessor::Band> createDefaultBands()
{
    std::vector<VoiceChanger_wczAudioProcessor::Band> defaults;
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe6\x9e\x81\xe4\xbd\x8e\xe9\xa2\x91")), juce::Colours::blue, VoiceChanger_wczAudioProcessor::HighPass, 20.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe4\xbd\x8e\xe9\xa2\x91")), juce::Colours::brown, VoiceChanger_wczAudioProcessor::LowShelf, 250.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe4\xb8\xad\xe4\xbd\x8e\xe9\xa2\x91")), juce::Colours::green, VoiceChanger_wczAudioProcessor::Peak, 500.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe4\xb8\xad\xe9\xab\x98\xe9\xa2\x91")), juce::Colours::coral, VoiceChanger_wczAudioProcessor::Peak, 1000.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe9\xab\x98\xe9\xa2\x91")), juce::Colours::orange, VoiceChanger_wczAudioProcessor::HighShelf, 5000.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS(juce::CharPointer_UTF8("\xe6\x9e\x81\xe9\xab\x98\xe9\xa2\x91")), juce::Colours::red, VoiceChanger_wczAudioProcessor::LowPass, 12000.0f, 0.707f));
    return defaults;
}
#endif
// 构建音频处理器参数树组
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;//总组
    {
        // 电平表组
        auto paramRight = std::make_unique<juce::AudioParameterFloat>("right", "Right", -60.f, 12.f, 0.f);
        auto paramLeft = std::make_unique<juce::AudioParameterFloat>("left", "Left", -60.f, 12.f, 0.f);
        auto paramRmsPeriod = std::make_unique<juce::AudioParameterInt>("rmsPeriod", "Period", 1, 500, 50);
        auto paramSmooth = std::make_unique<juce::AudioParameterBool>("smooth", "Enable Smoothing", true);
        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("meter", TRANS("Meter"), "|",
            std::move(paramRight),
            std::move(paramLeft),
            std::move(paramRmsPeriod),
            std::move(paramSmooth));
        params.push_back(std::move(group));
    }

    
    {
        //混响器组
        const auto range = juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f, 1.0f);
        const auto defaultValue = 50.0f;
        auto stringFromValue = [](float value, int)
        {
            if (value < 10.0f)
                return juce::String(value, 2) + " %";
            else if (value < 100.0f)
                return juce::String(value, 1) + " %";
            else
                return juce::String(value, 0) + " %";
        };
        auto reverbSize = std::make_unique<juce::AudioParameterFloat>(ParamNames::size,
            TRANS(ParamNames::size),
            range,
            defaultValue,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            stringFromValue,
            nullptr);
        auto reverbDamp = std::make_unique<juce::AudioParameterFloat>(ParamNames::damp,
            TRANS(ParamNames::damp),
            range,
            defaultValue,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            stringFromValue,
            nullptr);
        auto reverbWidth = std::make_unique<juce::AudioParameterFloat>(ParamNames::width,
            TRANS(ParamNames::width),
            range,
            defaultValue,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            stringFromValue,
            nullptr);
        auto reverbDrywet = std::make_unique<juce::AudioParameterFloat>(ParamNames::dryWet,
            TRANS(ParamNames::dryWet),
            range,
            defaultValue,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            stringFromValue,
            nullptr);
        auto reverbFreeze = std::make_unique<juce::AudioParameterBool>(ParamNames::freeze, TRANS(ParamNames::freeze), false);
        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("reverb", TRANS("Reverb"), "|",
            std::move(reverbSize),
            std::move(reverbDamp),
            std::move(reverbWidth),
            std::move(reverbDrywet),
            std::move(reverbFreeze));
        params.push_back(std::move(group));
    }
#if _OPEN_FILTERS
    // setting defaults

    const float maxGain = juce::Decibels::decibelsToGain(24.0f);
    auto defaults = createDefaultBands();


    {
        //均衡器组：输出
        auto param = std::make_unique<juce::AudioParameterFloat>(VoiceChanger_wczAudioProcessor::paramOutput, TRANS("Output"),
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f,
            TRANS("Output level"),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("global", TRANS("Globals"), "|", std::move(param));
        params.push_back(std::move(group));
    }

    for (size_t i = 0; i < defaults.size(); ++i)
    {
        // 每单个滤波器就要加一组。一组里面封装了滤波器类型、品质因数、增益、中心频率、是否被激活
        auto prefix = "Q" + juce::String(i + 1) + ": ";

        auto typeParameter = std::make_unique<juce::AudioParameterChoice>(VoiceChanger_wczAudioProcessor::getTypeParamName(i),
            prefix + TRANS("Filter Type"),
            VoiceChanger_wczAudioProcessor::getFilterTypeNames(),
            defaults[i].type);

        auto freqParameter = std::make_unique<juce::AudioParameterFloat>(VoiceChanger_wczAudioProcessor::getFrequencyParamName(i),
            prefix + TRANS("Frequency"),
            juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, std::log(0.5f) / std::log(980.0f / 19980.0f)},
            defaults[i].frequency,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return (value < 1000.0f) ?
            juce::String(value, 0) + " Hz" :
            juce::String(value / 1000.0f, 2) + " kHz"; },
            [](juce::String text) { return text.endsWith(" kHz") ?
            text.dropLastCharacters(4).getFloatValue() * 1000.0f :
            text.dropLastCharacters(3).getFloatValue(); });

        auto qltyParameter = std::make_unique<juce::AudioParameterFloat>(VoiceChanger_wczAudioProcessor::getQualityParamName(i),
            prefix + TRANS("Quality"),
            juce::NormalisableRange<float> {0.1f, 10.0f, 1.0f, std::log(0.5f) / std::log(0.9f / 9.9f)},
            defaults[i].quality,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1); },
            [](const juce::String& text) { return text.getFloatValue(); });

        auto gainParameter = std::make_unique<juce::AudioParameterFloat>(VoiceChanger_wczAudioProcessor::getGainParamName(i),
            prefix + TRANS("Gain"),
            juce::NormalisableRange<float> {1.0f / maxGain, maxGain, 0.001f,
            std::log(0.5f) / std::log((1.0f - (1.0f / maxGain)) / (maxGain - (1.0f / maxGain)))},
            defaults[i].gain,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {return juce::String(juce::Decibels::gainToDecibels(value), 1) + " dB"; },
            [](juce::String text) {return juce::Decibels::decibelsToGain(text.dropLastCharacters(3).getFloatValue()); });

        auto actvParameter = std::make_unique<juce::AudioParameterBool>(VoiceChanger_wczAudioProcessor::getActiveParamName(i),
            prefix + TRANS("Active"),
            defaults[i].active,
            juce::String(),
            [](float value, int) {return value > 0.5f ? TRANS("active") : TRANS("bypassed"); },
            [](juce::String text) {return text == TRANS("active"); });

        auto group = std::make_unique<juce::AudioProcessorParameterGroup>("band" + juce::String(i), defaults[i].name, "|",
            std::move(typeParameter),
            std::move(freqParameter),
            std::move(qltyParameter),
            std::move(gainParameter),
            std::move(actvParameter));

        params.push_back(std::move(group));

    }
#endif
    return { params.begin(), params.end() };//封装后返回
}



//==============================================================================处理器总构造函数
VoiceChanger_wczAudioProcessor::VoiceChanger_wczAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
    , pPlayBuffer(&fileBuffer)//播放指针默认指向fileBuffer
    , ti(Stopped, Mainpage)// 播放控制初始化
    , TransportInformation(ti)
    // , spectrum(juce::Image::RGB, 512, 256, true)//主界面频谱画面初始化

#endif
    , thumbnailCache(1)//主界面离线文件频谱缩略图
	, readAheadThread("transport read ahead")//初始化预先读取的线程名
    // , trainingTemplate(sourceBufferAligned, targetBufferAligned, voiceChangerParameter)
    , state(*this, &undo, "PARAMS", createParameterLayout())//参数树状态初始化，包括：撤回方法、构建方法、处理器指向
	, internalWriteThread("internalWriteThread")//内录线程

{
    ensureEngineCreatedOnMessageThread();//初始化Tracktion子引擎
    
    formatManager.registerBasicFormats();//初始化文件格式读取器
    readAheadThread.startThread(3);//构建读取线程备用
#if _OPEN_FILTERS
    frequencies.resize(300);//每次传给均衡器分析器300个样本点
    for (size_t i = 0; i < frequencies.size(); ++i) {
        frequencies[i] = 20.0 * std::pow(2.0, i / 30.0);//按照log尺度来显示
    }
    magnitudes.resize(frequencies.size());//总幅频响应幅度尺寸与频率相同（每个频率点显示一个幅度）

    // needs to be in sync with the ProcessorChain filter
    bands = createDefaultBands();//构建6个滤波器
    //对每个滤波器，绑定它们的参数响应
    for (size_t i = 0; i < bands.size(); ++i)
    {
        bands[i].magnitudes.resize(frequencies.size(), 1.0);

        state.addParameterListener(getTypeParamName(i), this);
        state.addParameterListener(getFrequencyParamName(i), this);
        state.addParameterListener(getQualityParamName(i), this);
        state.addParameterListener(getGainParamName(i), this);
        state.addParameterListener(getActiveParamName(i), this);

    }
    //绑定外围的参数响应（混响器和均衡器里的输出增益控制）
    state.addParameterListener(paramOutput, this);

    state.addParameterListener("left", this);
    state.addParameterListener("right", this);
    state.addParameterListener("rmsPeriod", this);
    state.addParameterListener("smoothing", this);

    
    state.state = juce::ValueTree(JucePlugin_Name);
#endif
    // 增加不需要构建到树里的零碎参数
    addParameter(nPitchShift = new juce::AudioParameterFloat("PitchShift", "pitchShift", -12.0f, 12.0f, 0.0f));
    addParameter(nPeakShift = new juce::AudioParameterFloat("PeakShift", "peakShift", 0.5f, 2.0f, 1.f));

    addParameter(nDynamicsThreshold = new juce::AudioParameterFloat("DynamicsThreshold", "dynamicsThreshold", -50, 0.0, -3.0f));
    addParameter(nDynamicsRatio = new juce::AudioParameterFloat("DynamicsRatio", "dynamicsRatio", 1.0f, 25.0f, 5.0f));
    addParameter(nDynamicsAttack = new juce::AudioParameterFloat("DynamicsAttack", "dynammicsAttack", 0.00001f, 1.0f, 0.002f));
    addParameter(nDynamicsRelease = new juce::AudioParameterFloat("DynamicsRelease", "dynamicsRelease", 0.001f, 2.0f, 0.1f));
    addParameter(nDynamicsMakeupGain = new juce::AudioParameterFloat("DynamicsMakeupGain", "dynamicsMakeupGain", -60.0f, 30.0f, 0.0f));


    // addParameter(nPlayAudioFilePosition = new juce::AudioParameterFloat("PlayAudioFilePositioin", "playAudioFilePosition", 0, 10000000000, 0.0f));
}

VoiceChanger_wczAudioProcessor::~VoiceChanger_wczAudioProcessor()
{
    //parameters.removeParameterListener("left", this);
    //parameters.removeParameterListener("right", this);
    //parameters.removeParameterListener("rmsPeriod", this);
    //parameters.removeParameterListener("smoothing", this);
    transportSource.setSource(nullptr);
    mainpageAnalyser.stopThread(1000);
#if _OPEN_FILTERS
	inputAnalyser.stopThread(1000);
    outputAnalyser.stopThread(1000);
#endif
}

//==============================================================================
const juce::String VoiceChanger_wczAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VoiceChanger_wczAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VoiceChanger_wczAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VoiceChanger_wczAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VoiceChanger_wczAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VoiceChanger_wczAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VoiceChanger_wczAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VoiceChanger_wczAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VoiceChanger_wczAudioProcessor::getProgramName (int index)
{
    return {};
}

void VoiceChanger_wczAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VoiceChanger_wczAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    ensurePrepareToPlayCalledOnMessageThread(sampleRate,samplesPerBlock);//同步子引擎的prepareToPlay信息

    this->samplesPerBlock = samplesPerBlock;
    fileBuffer.setSize(2, sampleRate * 60);
    internalWriteThread.startThread(7);//开辟内录线程
    //internalRecordingStream = new FileOutputStream(File::getSpecialLocation(File::userDesktopDirectory).getChildFile("test.wav"));
    //WavAudioFormat format;
    //internalRecordWriter = format.createWriterFor(internalRecordingStream, sampleRate, 2, 16, StringPairArray(), 0);
    //threadedInternalRecording = new AudioFormatWriter::ThreadedWriter(internalRecordWriter, internalWriteThread,4800000);

    setLatencySamples(44100 * 2);
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);//初始化播放控制模块
    offlineOnlineTransitBuffer.setSize(getNumInputChannels(), samplesPerBlock);
	#if _OPEN_FILTERS
    //初始化DSP模块
    juce::dsp::ProcessSpec fspec;
    fspec.sampleRate = sampleRate;
    fspec.maximumBlockSize = juce::uint32(sampleRate);
    fspec.numChannels = juce::uint32(getTotalNumOutputChannels());
    //根据默认参数初始化每个滤波器的参数响应
	for (size_t i = 0; i < bands.size(); ++i) {
        updateBand(i);
    }
    filter.get<6>().setGainLinear(*state.getRawParameterValue(paramOutput));

    updatePlots();
    //初始化DSP模块
    filter.prepare(fspec);
    reverb.prepare(fspec);
    inputAnalyser.setupAnalyser(int(sampleRate), float(sampleRate));//初始化输入输出分析器
    outputAnalyser.setupAnalyser(int(sampleRate), float(sampleRate));
#endif
    mainpageAnalyser.setupSpectralAnalyser(int(sampleRate), float(sampleRate));
    const auto numberOfChannels = getTotalNumInputChannels();
    rmsLevels.clear();
    for (auto i = 0; i < numberOfChannels; i++)
    {
        juce::LinearSmoothedValue<float> rms{ -100.0f };
        rms.reset(sampleRate, 0.5);
        rmsLevels.emplace_back(std::move(rms));//初始化电平表显示数组
    }
    rmsFifo.reset(numberOfChannels, static_cast<int>(sampleRate) + 1);
    rmsCalculationBuffer.clear();
    rmsCalculationBuffer.setSize(numberOfChannels, static_cast<int>(sampleRate + 1));


    gainLeft.reset(sampleRate, 0.2);;
    gainLeft.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("left")->load()));//初始化增益

    gainRight.reset(sampleRate, 0.2);
    gainRight.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("right")->load()));

    rmsWindowSize = static_cast<int>(sampleRate * state.getRawParameterValue("rmsPeriod")->load()) / 1000;//初始化电平表缓冲
    isSmoothed = false; //static_cast<bool>(state.getRawParameterValue("smooth")->load());


#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND //构建频域和时域音调缩放处理实例
    rbs = std::make_unique<PitchShifterRubberband>(getTotalNumInputChannels(), sampleRate, samplesPerBlock);
#endif
#if USE_SOUNDTOUCH
    sts = std::make_unique<PitchShifterSoundTouch>(getTotalNumInputChannels(), sampleRate, samplesPerBlock);
#endif
#endif
    // vcb = std::make_unique<VoiceConversionBuffer>(1, sampleRate, samplesPerBlock, model);
    // vadModule = WebRtcVad_Create();
    // int vaderr = WebRtcVad_Init(vadModule);
    // if (vaderr == -1)
    //     exit(0);
#if _OPEN_PEAK_PITCH
    pitchShifters.clear();
    peakShifters.clear();
#endif

#if _OPEN_WAHWAH
    filtersForWahWah.clear();
    for (int i = 0; i < getTotalNumInputChannels(); ++i)
    {
        envelopesForWahWah.add(0.0f);
    }
    twoPi = juce::MathConstants<float>::twoPi;
    inverseEForWahWah = 1.0f / juce::MathConstants<float>::euler;
    lfoPhaseForWahWah = 0.0f;
    inverseSampleRateForWahWah = 1.0f / (float)sampleRate;
#endif

#
#if _OPEN_TEST
    shapeInvariantPitchShifters.clear();
#endif

    for (int i = 0; i < getTotalNumInputChannels(); ++i)
    {
        //构建频域音调缩放和共振峰移动实例
#if _OPEN_WAHWAH
        filtersForWahWah.add(filter = new Filter());
#endif
#if _OPEN_PEAK_PITCH
        PitchShifter* pPitchShifter;
        pitchShifters.add(pPitchShifter = new PitchShifter());

        PeakShifter* pPeakShifter;
        peakShifters.add(pPeakShifter = new PeakShifter());


        //VocoderForVoiceConversion* pVocodersForVoiceConversion;
        //vocodersForVoiceConversion.add(pVocodersForVoiceConversion = new VocoderForVoiceConversion(sampleRate));

        const auto windows = pitchShifters[0]->getLatencyInSamples();
#endif

#if _OPEN_TEST
        ShapeInvariantPitchShifter* pShapeInvariantPitchShifter;
        shapeInvariantPitchShifters.add(pShapeInvariantPitchShifter = new ShapeInvariantPitchShifter());
        const auto windows1 = shapeInvariantPitchShifters[0]->getLatencyInSamples();
        setLatencySamples(windows1);
#endif
    }
 
#if _OPEN_DYNAMICS//初始化动态参数
    mixedDownInputDynamics.setSize(1, samplesPerBlock);
    inputLevelDynamics = 0.0f;
    ylPrevDynamics = 0.0f;
    inverseEDynamics = 1.0f / juce::MathConstants<double>::euler;
    inverseSampleRateDynamics = 1.0f / (float)sampleRate;
#endif
    
}

void VoiceChanger_wczAudioProcessor::releaseResources()
{
    // 停止播放时，使用该方法释放资源
    // isInternalRecording = false;
	// internalWriteThread.stopThread(1000);
    // threadedInternalRecording = nullptr;
    // internalRecordWriter = nullptr;
    transportSource.releaseResources();
#if _OPEN_FILTERS
    inputAnalyser.stopThread(1000);
    outputAnalyser.stopThread(1000);
#endif
    mainpageAnalyser.stopThread(1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VoiceChanger_wczAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{//juce默认接口
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VoiceChanger_wczAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (allFunc.load())
    {
        if (realtimeMode)//在实时模式
        {
            overallProcess(buffer);//不切换音频源
            //processLevelInfo(buffer);
        }
        else//在离线模式
        {
            // buffer.clear();
            for (int j = 0; j < buffer.getNumChannels(); j++)
            {
                offlineOnlineTransitBuffer.copyFrom(j, 0, buffer, j, 0, buffer.getNumSamples());
            }
            transportSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));//切换到离线音频源再处理

            for(int j = 0; j < buffer.getNumChannels(); j++)
				buffer.addFrom(j, 0, offlineOnlineTransitBuffer, j, 0, buffer.getNumSamples(), 0.8);
            
        	overallProcess(buffer);
            //if (!canReadSampleBuffer)
            //{
            //    if (pPlayBuffer)
            //        if (pPlayBuffer->getNumChannels())
            //            canReadSampleBuffer = true;
            //}
            //if (canReadSampleBuffer)
            //{
            //    if (pPlayBuffer->getNumChannels())
            //    {
            //        if (shouldProcessFile)
            //        {
            //            getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
            //            // transportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
            //            overallProcess(buffer);
            //            return;
            //        }
            //    }
            //}
            //spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
        }

        if (isDawStream.load())//如果子引擎与主引擎链接
        {
            engineWrapper->playheadSynchroniser.synchronise(*this);
            engineWrapper->audioInterface.processBlock(buffer, midiMessages);//使用子引擎的音频块
        }
        const ScopedLock s1(writerLock);//锁死内录线程
        if (activeWriter.load() != nullptr)
        {
            activeWriter.load()->write(buffer.getArrayOfReadPointers(), buffer.getNumSamples());//将当前音频块写入
        }
    }
    else
    {
        processDynamics(buffer,false, getDynamicsThresholdShift(),
            getDynamicsRatioShift(), getDynamicsAttackShift(),
            getDynamicsReleaseShift(), getDynamicsMakeupGainShift());
    }
    mainpageAnalyser.addAudioData(buffer, 0, getNumInputChannels());
    processLevelInfo(buffer);
}
void VoiceChanger_wczAudioProcessor::getNextAudioBlock(juce::AudioSourceChannelInfo& buffer)//手动获取下一个音频块
{//已用transportSource方法替代
    auto outputSamplesRemaining = buffer.buffer->getNumSamples();
    auto outputSamplesOffset = buffer.startSample;
    while (outputSamplesRemaining > 0)
    {
        auto bufferSamplesRemaining = pPlayBuffer->getNumSamples() - nPlayAudioFilePosition;// fileBuffer.getNumSamples() - nPlayAudioFilePosition;
        auto samplesThieTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);
        for (auto channel = 0; channel < getNumOutputChannels(); ++channel)
        {
            buffer.buffer->copyFrom(
                channel,
                outputSamplesOffset,
                *pPlayBuffer,
                channel % getNumInputChannels(),
                nPlayAudioFilePosition,
                samplesThieTime
            );
        }
        outputSamplesRemaining -= samplesThieTime;
        outputSamplesOffset += samplesThieTime;
        nPlayAudioFilePosition += samplesThieTime;
        if (nPlayAudioFilePosition == pPlayBuffer->getNumSamples())
            nPlayAudioFilePosition = 0;
    }
}

void VoiceChanger_wczAudioProcessor::overallProcess(juce::AudioBuffer<float>& buffer)//具体的总处理
{
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    int numSamples = buffer.getNumSamples();

    updateUIControls();//更新音调缩放控制
    updateReverbSettings();//更新混响控制
    if (openVoiceConversion.load() && isModelLoaded.load())
    {
        vcb->processBuffer(buffer);
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }
#if _OPEN_DYNAMICS
    processDynamics(buffer, false, getDynamicsThresholdShift(),
        getDynamicsRatioShift(), getDynamicsAttackShift(),
        getDynamicsReleaseShift(), getDynamicsMakeupGainShift());//处理动态数据
#endif
#if _OPEN_WAHWAH
    //processWahwah(juce::AudioBuffer<float>&buffer,float attackValue,float releaseValue,float MixLFOAndEnvelope,
    // float lfoFrequency,float mixRatio,float filterQFactor,float filterGain,float filterFreq)
    processWahwah(buffer, 0.02, 0.3, 0.8, 5, 0.5, 20, 2, 200.0f);
#endif
    //以下方法均直接调用接口
#if USE_3rdPARTYPITCHSHIFT
    {
#if USE_SOUNDTOUCH
    	if ( !useFD.load())
        	sts->processBuffer(buffer);
#endif
#if USE_RUBBERBAND
        else
    		rbs->processBuffer(buffer);
    }
#endif
#endif

#if _OPEN_FILTERS
    //if ( !openVoiceConversion)
    {
        if (getActiveEditor() != nullptr)
            inputAnalyser.addAudioData(buffer, 0, getTotalNumInputChannels());

        if (wasBypassed) {
            filter.reset();
            wasBypassed = false;
        }
        juce::dsp::AudioBlock<float>              ioBuffer(buffer);
        juce::dsp::ProcessContextReplacing<float> context(ioBuffer);
        filter.process(context);
        if (openReverb.load())
        {
            reverb.process(context);
        }
        if (getActiveEditor() != nullptr)
            outputAnalyser.addAudioData(buffer, 0, getTotalNumOutputChannels());
    }
#endif
    {
        for (int channel = 0; channel < getNumInputChannels(); ++channel)
        {
            auto channelDataFlt = buffer.getWritePointer(channel);
            // pitchShifters[channel]->process(channelDataFlt, numSamples);
            peakShifters[channel]->process(channelDataFlt, numSamples);
        }
    }

#if _OPEN_TEST
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        // spectrumFilter[channel]->process(channelData, numSamples);
        shapeInvariantPitchShifters[channel]->process(channelData, numSamples);

    }
#endif
    
}


void VoiceChanger_wczAudioProcessor::processLevelInfo(juce::AudioBuffer<float>& buffer)//电平表计算
{
    const auto numSamples = buffer.getNumSamples();
    {
        const auto startGain = gainLeft.getCurrentValue();//获取当前增益
        gainLeft.skip(numSamples);

        const auto endGain = gainLeft.getCurrentValue();
        //buffer.applyGainRamp(0, 0, numSamples, startGain, endGain);
    }
    {
        const auto startGain = gainRight.getCurrentValue();
        gainRight.skip(numSamples);
        const auto endGain = gainRight.getCurrentValue();
        //buffer.applyGainRamp(1, 0, numSamples, startGain, endGain);
    }
    for (auto& rmsLevel : rmsLevels)
        rmsLevel.skip(numSamples);
    rmsFifo.push(buffer);//将当前音频块送入电平表FIFO

}


#if _OPEN_FILTERS

juce::AudioProcessorValueTreeState& VoiceChanger_wczAudioProcessor::getPluginState()
{
    return state;
}

juce::String VoiceChanger_wczAudioProcessor::getTypeParamName(size_t index)
{
    return getBandID(index) + "-" + paramType;
}

juce::String VoiceChanger_wczAudioProcessor::getFrequencyParamName(size_t index)
{
    return getBandID(index) + "-" + paramFrequency;
}

juce::String VoiceChanger_wczAudioProcessor::getQualityParamName(size_t index)
{
    return getBandID(index) + "-" + paramQuality;
}

juce::String VoiceChanger_wczAudioProcessor::getGainParamName(size_t index)
{
    return getBandID(index) + "-" + paramGain;
}

juce::String VoiceChanger_wczAudioProcessor::getActiveParamName(size_t index)
{
    return getBandID(index) + "-" + paramActive;
}
#endif
void VoiceChanger_wczAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)//参数树响应
{
    if (parameterID.equalsIgnoreCase("left"))
        gainLeft.setTargetValue(juce::Decibels::decibelsToGain(newValue));
    if (parameterID.equalsIgnoreCase("right"))
    {
        gainRight.setTargetValue(juce::Decibels::decibelsToGain(newValue));
    }
    if (parameterID.equalsIgnoreCase("rmsPeriod"))
    {
        rmsWindowSize = static_cast<int>(getSampleRate() * newValue) / 1000;
    }
    if (parameterID.equalsIgnoreCase("smoothing"))
    {
        isSmoothed = static_cast<bool>(newValue);
    }

    if (parameterID.equalsIgnoreCase(ParamNames::size))
    {
        reverbParams.roomSize = newValue;
        reverb.setParameters(reverbParams);
    }
    if (parameterID.equalsIgnoreCase(ParamNames::width))
    {
        reverbParams.width = newValue;
        reverb.setParameters(reverbParams);
    }
    if (parameterID.equalsIgnoreCase(ParamNames::damp))
    {
        reverbParams.damping = newValue;
        reverb.setParameters(reverbParams);
    }
    if (parameterID.equalsIgnoreCase(ParamNames::dryWet))
    {
        reverbParams.wetLevel = newValue;
        reverbParams.dryLevel = 1 - newValue;
        reverb.setParameters(reverbParams);
    }
    if (parameterID.equalsIgnoreCase(ParamNames::freeze))
    {
        reverbParams.freezeMode = newValue;
        reverb.setParameters(reverbParams);
    }


#if _OPEN_FILTERS
    if (parameterID == paramOutput) {
        filter.get<6>().setGainLinear(newValue);
        updatePlots();
        return;
    }

    int index = getBandIndexFromID(parameterID);
    if (juce::isPositiveAndBelow(index, bands.size()))
    {
        auto* band = getBand(size_t(index));
        if (parameterID.endsWith(paramType)) {
            band->type = static_cast<FilterType> (static_cast<int> (newValue));
        }
        else if (parameterID.endsWith(paramFrequency)) {
            band->frequency = newValue;
        }
        else if (parameterID.endsWith(paramQuality)) {
            band->quality = newValue;
        }
        else if (parameterID.endsWith(paramGain)) {
            band->gain = newValue;
        }
        else if (parameterID.endsWith(paramActive)) {
            band->active = newValue >= 0.5f;
        }

        updateBand(size_t(index));
    }
#endif
}

std::vector<float> VoiceChanger_wczAudioProcessor::getRmsLevels()//获取双通道的RMS电平值
{
    rmsFifo.pull(rmsCalculationBuffer, rmsWindowSize);
    std::vector<float>levels;
    for (auto channel = 0; channel < rmsCalculationBuffer.getNumChannels(); channel++)
    {
        processLevelValue(rmsLevels[channel], juce::Decibels::gainToDecibels(rmsCalculationBuffer.getRMSLevel(channel, 0, rmsWindowSize)));
        levels.push_back(rmsLevels[channel].getCurrentValue());
    }
    return levels;
}
float VoiceChanger_wczAudioProcessor::getRmsLevel(const int channel)
{
    rmsFifo.pull(rmsCalculationBuffer.getWritePointer(channel), channel, rmsWindowSize);//先从FIFO送入计算rms值的音频块然后在那里获取电平值
    processLevelValue(rmsLevels[channel], juce::Decibels::gainToDecibels(rmsCalculationBuffer.getRMSLevel(channel, 0, rmsWindowSize)));
    return rmsLevels[channel].getCurrentValue();
}
void VoiceChanger_wczAudioProcessor::processLevelValue(juce::LinearSmoothedValue<float>& smoothedValue, const float value)const
{
    if (isSmoothed)//是否需要平滑电平
    {
        if (value < smoothedValue.getCurrentValue())
        {
            smoothedValue.setTargetValue(value);
            return;
        }
    }
    smoothedValue.setCurrentAndTargetValue(value);
}



#if _OPEN_FILTERS

size_t VoiceChanger_wczAudioProcessor::getNumBands() const
{
    return bands.size();
}

juce::String VoiceChanger_wczAudioProcessor::getBandName(size_t index) const
{
    if (juce::isPositiveAndBelow(index, bands.size()))
        return bands[size_t(index)].name;
    return TRANS("unknown");
}
juce::Colour VoiceChanger_wczAudioProcessor::getBandColour(size_t index) const
{
    if (juce::isPositiveAndBelow(index, bands.size()))
        return bands[size_t(index)].colour;
    return juce::Colours::silver;
}

bool VoiceChanger_wczAudioProcessor::getBandSolo(int index) const
{
    return index == soloed;
}

void VoiceChanger_wczAudioProcessor::setBandSolo(int index)
{
    soloed = index;
    updateBypassedStates();
}

void VoiceChanger_wczAudioProcessor::updateBypassedStates()//根据全局soloed值控制滤波器组激活
{
    if (juce::isPositiveAndBelow(soloed, bands.size())) {
        filter.setBypassed<0>(soloed != 0);
        filter.setBypassed<1>(soloed != 1);
        filter.setBypassed<2>(soloed != 2);
        filter.setBypassed<3>(soloed != 3);
        filter.setBypassed<4>(soloed != 4);
        filter.setBypassed<5>(soloed != 5);
    }
    else {
        filter.setBypassed<0>(!bands[0].active);
        filter.setBypassed<1>(!bands[1].active);
        filter.setBypassed<2>(!bands[2].active);
        filter.setBypassed<3>(!bands[3].active);
        filter.setBypassed<4>(!bands[4].active);
        filter.setBypassed<5>(!bands[5].active);
    }
    updatePlots();
}

VoiceChanger_wczAudioProcessor::Band* VoiceChanger_wczAudioProcessor::getBand(size_t index)//获取单滤波器指针
{
    if (juce::isPositiveAndBelow(index, bands.size()))
        return &bands[index];
    return nullptr;
}

juce::StringArray VoiceChanger_wczAudioProcessor::getFilterTypeNames()//获取滤波器名字显示
{
    return {
        TRANS(juce::CharPointer_UTF8("\xe6\x97\xa0\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe9\xab\x98\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe4\xb8\x80\xe9\x98\xb6\xe9\xab\x98\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe4\xbd\x8e\xe9\xa2\x91\xe6\x90\x81\xe6\x9e\xb6\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe5\xb8\xa6\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe5\x85\xa8\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe4\xb8\x80\xe9\x98\xb6\xe5\x85\xa8\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe9\x99\xb7\xe6\xb3\xa2\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe5\xb3\xb0\xe5\x80\xbc\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe9\xab\x98\xe9\xa2\x91\xe6\x90\x81\xe6\x9e\xb6\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe4\xb8\x80\xe9\x98\xb6\xe4\xbd\x8e\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8")),
        TRANS(juce::CharPointer_UTF8("\xe4\xbd\x8e\xe9\x80\x9a\xe6\xbb\xa4\xe6\xb3\xa2\xe5\x99\xa8"))
    };
}

void VoiceChanger_wczAudioProcessor::updateBand(const size_t index)//按照滤波器信息获取IIR系数
{
    if (sampleRate > 0) {
        juce::dsp::IIR::Coefficients<float>::Ptr newCoefficients;
        switch (bands[index].type) {
        case NoFilter:
            newCoefficients = new juce::dsp::IIR::Coefficients<float>(1, 0, 1, 0);
            break;
        case LowPass:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, bands[index].frequency, bands[index].quality);
            break;
        case LowPass1st:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(sampleRate, bands[index].frequency);
            break;
        case LowShelf:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, bands[index].frequency, bands[index].quality, bands[index].gain);
            break;
        case BandPass:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, bands[index].frequency, bands[index].quality);
            break;
        case AllPass:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, bands[index].frequency, bands[index].quality);
            break;
        case AllPass1st:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderAllPass(sampleRate, bands[index].frequency);
            break;
        case Notch:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, bands[index].frequency, bands[index].quality);
            break;
        case Peak:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, bands[index].frequency, bands[index].quality, bands[index].gain);
            break;
        case HighShelf:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, bands[index].frequency, bands[index].quality, bands[index].gain);
            break;
        case HighPass1st:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(sampleRate, bands[index].frequency);
            break;
        case HighPass:
            newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, bands[index].frequency, bands[index].quality);
            break;
        case LastFilterID:
        default:
            break;
        }

        if (newCoefficients)
        {
            {
                // 最小化线程锁范围，get<0>（）需要是编译时常量
                juce::ScopedLock processLock(getCallbackLock());
                if (index == 0)
                    *filter.get<0>().state = *newCoefficients;
                else if (index == 1)
                    *filter.get<1>().state = *newCoefficients;
                else if (index == 2)
                    *filter.get<2>().state = *newCoefficients;
                else if (index == 3)
                    *filter.get<3>().state = *newCoefficients;
                else if (index == 4)
                    *filter.get<4>().state = *newCoefficients;
                else if (index == 5)
                    *filter.get<5>().state = *newCoefficients;
            }
            newCoefficients->getMagnitudeForFrequencyArray(frequencies.data(),
                bands[index].magnitudes.data(),
                frequencies.size(), sampleRate);

        }
        updateBypassedStates();//更新完后仍要更新旁通状态
        updatePlots();//更新总的幅频响应画法
    }
}

void VoiceChanger_wczAudioProcessor::updatePlots()
{
    auto gain = filter.get<6>().getGainLinear();
    std::fill(magnitudes.begin(), magnitudes.end(), gain);

    if (juce::isPositiveAndBelow(soloed, bands.size())) {
        juce::FloatVectorOperations::multiply(magnitudes.data(), bands[size_t(soloed)].magnitudes.data(), static_cast<int> (magnitudes.size()));
    }
    else
    {
        for (size_t i = 0; i < bands.size(); ++i)
            if (bands[i].active)
                juce::FloatVectorOperations::multiply(magnitudes.data(), bands[i].magnitudes.data(), static_cast<int> (magnitudes.size()));
    }

    sendChangeMessage();//向编辑器广播：有新数据可用了；如果没有构建编辑器，listener为空指针也不会报错
}
#endif
//==============================================================================
bool VoiceChanger_wczAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VoiceChanger_wczAudioProcessor::createEditor()
{
    return new VoiceChanger_wczAudioProcessorEditor (*this);
    //return new FrequalizerAudioProcessorEditor(*this);
}

#if _OPEN_FILTERS

const std::vector<double>& VoiceChanger_wczAudioProcessor::getMagnitudes()
{
    return magnitudes;
}

void VoiceChanger_wczAudioProcessor::createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble)
{
    // 画每个滤波器的幅频响应，方法放在处理器，但调用时在编辑器，传入的引用也都是编辑器的
    p.startNewSubPath(float(bounds.getX()), mags[0] > 0 ? float(bounds.getCentreY() - pixelsPerDouble * std::log(mags[0]) / std::log(2.0)) : bounds.getBottom());
    const auto xFactor = static_cast<double> (bounds.getWidth()) / frequencies.size();
    for (size_t i = 1; i < frequencies.size(); ++i)
    {
        p.lineTo(float(bounds.getX() + i * xFactor),
            float(mags[i] > 0 ? bounds.getCentreY() - pixelsPerDouble * std::log(mags[i]) / std::log(2.0) : bounds.getBottom()));
    }
}

void VoiceChanger_wczAudioProcessor::createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input)
{
    if (input)
        inputAnalyser.createPath(p, bounds.toFloat(), minFreq);//更新的实时的频谱
    else
        outputAnalyser.createPath(p, bounds.toFloat(), minFreq);
}

bool VoiceChanger_wczAudioProcessor::checkForNewAnalyserData()
{
    return inputAnalyser.checkForNewData() || outputAnalyser.checkForNewData();//检查是否有新的分析器数据可以了，而后传给编辑器
}
//==============================================================================
void VoiceChanger_wczAudioProcessor::getStateInformation (juce::MemoryBlock& destData)//便于保存当前状态下次加载
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("VoiceChangerWczParam"));

    xml->setAttribute("pitchShift", (double)*nPitchShift);
    xml->setAttribute("peakShift", (double)*nPeakShift);

    copyXmlToBinary(*xml, destData);


#if _OPEN_FILTERS

    auto editor = state.state.getOrCreateChildWithName(IDs::editor, nullptr);
    editor.setProperty(IDs::sizeX, editorSize.x, nullptr);
    editor.setProperty(IDs::sizeY, editorSize.y, nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.state.writeToStream(stream);
#endif
}

void VoiceChanger_wczAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement>xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("VoiceChangerWczParam"))
        {

            *nPitchShift = (float)xmlState->getDoubleAttribute("pitchShift", 1.0);
            *nPeakShift = (float)xmlState->getDoubleAttribute("peakShift", 1.0);
        }
    }
#if _OPEN_FILTERS
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid()) {
        state.state = tree;

        auto editor = state.state.getChildWithName(IDs::editor);
        if (editor.isValid())
        {
            editorSize.setX(editor.getProperty(IDs::sizeX, 900));
            editorSize.setY(editor.getProperty(IDs::sizeY, 500));
            if (auto* thisEditor = getActiveEditor())
                thisEditor->setSize(editorSize.x, editorSize.y);
        }
    }

#endif
}


juce::Point<int> VoiceChanger_wczAudioProcessor::getSavedSize() const
{
    return editorSize;
}

void VoiceChanger_wczAudioProcessor::setSavedSize(const juce::Point<int>& size)
{
    editorSize = size;
}
#endif



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoiceChanger_wczAudioProcessor();
}

#if _OPEN_WAHWAH
float VoiceChanger_wczAudioProcessor::calculateAttackOrReleaseForWahWah(float value)
{
    if (value == 0.0f)
        return 0.0f;
    else
        return pow(inverseEForWahWah, inverseSampleRateForWahWah / value);
}
void VoiceChanger_wczAudioProcessor::processWahwah(
    juce::AudioBuffer<float>& buffer,
    float attackValue,
    float releaseValue,
    float MixLFOAndEnvelope,
    float lfoFrequency,
    float mixRatio,
    float filterQFactor,
    float filterGain,
    float filterFreq
    )
{
    // const double smoothTime = 1e-3;
    float phase;
    const int numSamples = buffer.getNumSamples();
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        
        float* channelData = buffer.getWritePointer(channel);
        phase = lfoPhaseForWahWah;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float in = channelData[sample];
            float absIn = fabs(in);
            float envelope;
            float attack = calculateAttackOrReleaseForWahWah(attackValue);
            float release = calculateAttackOrReleaseForWahWah(releaseValue);


            if (absIn > envelopesForWahWah[channel])
            {
                envelope = attack * envelopesForWahWah[channel] + (1.0f - attack) * absIn;
            }
            else
            {
                envelope = release * envelopesForWahWah[channel] + (1.0f - release) * absIn;
            }

            envelopesForWahWah.set(channel, envelope);

            // if(modeAutomatic)
            {
                float centerFrequencyLFO = 0.5f + 0.5f * sinf(twoPi * phase);
                float centerFrequencyEnv = envelopesForWahWah[channel];
                centerFrequencyForWahWah = centerFrequencyLFO;

                centerFrequencyForWahWah *= (10000 - 200.f);//freqMaxValue - freqMinValue;
                centerFrequencyForWahWah += 200.f;

                phase += lfoFrequency * inverseSampleRateForWahWah;
                if (phase > 1.0f)
                {
                    phase -= 1.0f;
                }

                // setFilterFreqShift(centerFrequencyForWahWah);
                // updateFilterForWahWah();
                double discreteFrequency = twoPi * (double)centerFrequencyForWahWah / getSampleRate();
                double qFactor = (double)filterQFactor;
                double gain = pow(10.0, (double)filterGain * 0.05);
                int type = 8;
                for (int i = 0; i < filtersForWahWah.size(); ++i)
                {
                    filtersForWahWah[i]->updateCoefficients(discreteFrequency, qFactor, gain, type);
                }
                
            }
            float filtered = filtersForWahWah[channel]->processSingleSampleRaw(in);
            float out = in + mixRatio * (filtered - in);
            // float out = in + mixRatio * (filtered - in);
            channelData[sample] = out;
        }
    }
    lfoPhaseForWahWah = phase;
}
#endif

#if _OPEN_DYNAMICS// 动态处理模块：ADSR
void VoiceChanger_wczAudioProcessor::processDynamics(
    juce::AudioBuffer<float>& buffer
    ,bool isExpanderOrCompressor
    ,float threshold
    ,float ratio
    ,float attack
    ,float release
    ,float makeupGain
    )
{
    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    mixedDownInputDynamics.clear();
    for (int channel = 0; channel < numInputChannels; ++channel)
    {
        // DBG("have run at this place for " << channel << " times");
        mixedDownInputDynamics.addFrom(0, 0, buffer, channel, 0, numSamples, 1.0f / numInputChannels);
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        bool expander = isExpanderOrCompressor;
        float T = threshold;
        float R = ratio;
        float alphaA = calculateAttackOrReleaseForDynamics(attack);
        float alphaR = calculateAttackOrReleaseForDynamics(release);
        // float makeupGain = makeupGain;


        float inputSquared = powf(mixedDownInputDynamics.getSample(0, sample), 2.0f);
        if (expander)
        {
            const float averageFactor = 0.9999f;
            inputLevelDynamics = averageFactor * inputLevelDynamics + (1.0f - averageFactor) * inputSquared;
        }
        else
        {
            inputLevelDynamics = inputSquared;
        }
        xgDynamics = (inputLevelDynamics <= 1e-6f) ? -60.0f : 10.0f * log10f(inputLevelDynamics);
        if (expander)
        {
            if (xgDynamics > T)
            {
                ygDynamics = xgDynamics;
            }
            else
            {
                ygDynamics = T + (xgDynamics - T) * R;
            }
            xlDynamics = xgDynamics - ygDynamics;
            if (xlDynamics < ylPrevDynamics)
            {
                ylDynamics = alphaA * ylPrevDynamics + (1.0f - alphaA) * xlDynamics;
            }
            else
            {
                ylDynamics = alphaA * ylPrevDynamics + (1.0f - alphaA) * xlDynamics;
            }
        }
        else
        {
            if (xgDynamics < T)
            {
                ygDynamics = xgDynamics;
            }
            else
            {
                ygDynamics = T + (xgDynamics - T) / R;
            }
            xlDynamics = xgDynamics - ygDynamics;
            if (xlDynamics > ylPrevDynamics)
            {
                ylDynamics = alphaA * ylPrevDynamics + (1.0f - alphaA) * xlDynamics;
            }
            else
            {
                ylDynamics = alphaR * ylPrevDynamics + (1.0f - alphaR) * xlDynamics;
            }
        }
        controlDynamics = powf(10.0f, (makeupGain - ylDynamics) * 0.05f);
        ylPrevDynamics = ylDynamics;

        for (int channel = 0; channel < numInputChannels; ++channel)
        {
            float newValue = buffer.getSample(channel, sample) * controlDynamics;
            buffer.setSample(channel, sample, newValue);
        }
    }
}
float VoiceChanger_wczAudioProcessor::calculateAttackOrReleaseForDynamics(float value)
{

    if (value == 0.0f)
        return 0.0f;
    else
        return pow(inverseEDynamics, inverseSampleRateDynamics / value);
}
#endif

//参数设置（音调缩放和动态控制）
void VoiceChanger_wczAudioProcessor::setPitchShift(float pitch)
{
    
    // state.("PitchShift");
    *nPitchShift = pitch;
}
void VoiceChanger_wczAudioProcessor::setPeakShift(float peak)
{
    *nPeakShift = peak;
}



void VoiceChanger_wczAudioProcessor::setDynamicsThresholdShift(float threshold)
{
    *nDynamicsThreshold = threshold;
}
void VoiceChanger_wczAudioProcessor::setDynamicsRatioShift(float ratio)
{
    *nDynamicsRatio = ratio;
}
void VoiceChanger_wczAudioProcessor::setDynamicsAttackShift(float attack)
{
    *nDynamicsAttack = attack;
}
void VoiceChanger_wczAudioProcessor::setDynamicsReleaseShift(float release)
{
    *nDynamicsRelease = release;
}
void VoiceChanger_wczAudioProcessor::setDynamicsMakeupGainShift(float makeupGain)
{
    *nDynamicsMakeupGain = makeupGain;
}
void VoiceChanger_wczAudioProcessor::setPlayAudioFilePosition(float position)
{
    // transportSource.setPosition(position);
    nPlayAudioFilePosition = position;
}
float VoiceChanger_wczAudioProcessor::getPitchShift()
{
    return *nPitchShift;
}
float VoiceChanger_wczAudioProcessor::getPeakShift()
{
    return *nPeakShift;
}
float VoiceChanger_wczAudioProcessor::getDynamicsThresholdShift()
{
    return *nDynamicsThreshold;
}
float VoiceChanger_wczAudioProcessor::getDynamicsRatioShift()
{
    return *nDynamicsRatio;
}
float VoiceChanger_wczAudioProcessor::getDynamicsAttackShift()
{
    return *nDynamicsAttack;
}
float VoiceChanger_wczAudioProcessor::getDynamicsReleaseShift()
{
    return *nDynamicsRelease;
}
float VoiceChanger_wczAudioProcessor::getDynamicsMakeupGainShift()
{
    return *nDynamicsMakeupGain;
}
float VoiceChanger_wczAudioProcessor::getPlayAudioFilePosition()
{
    return nPlayAudioFilePosition / nPlayAudioFileSampleNum;
}

void VoiceChanger_wczAudioProcessor::updateUIControls()//更新音调缩放参数控制
{

#if _OPEN_PEAK_PITCH
    float pitchRatio = getPitchShift();
    float peakRatio = getPeakShift();


#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND
    rbs->setSemitoneShift(pitchRatio);
#endif
#if USE_SOUNDTOUCH
    sts->setSemitoneShift(pitchRatio);
#endif
#else
    for (int i = 0; i < pitchShifters.size(); ++i)
    {
        pitchShifters[i]->setPitchRatio(pow(2,(pitchRatio/12)));
        // shapeInvariantPitchShifters[i]->setPitchRatio(pitchRatio);
    }
#endif
    for (int i = 0; i < peakShifters.size(); ++i)
    {
        peakShifters[i]->setPitchRatio(peakRatio);
    }

#endif
#if _OPEN_TEST
    float pitchRatioTest = getPitchShift();
    for (int i = 0; i < pitchShifters.size(); ++i)
    {
        shapeInvariantPitchShifters[i]->setPitchRatio(pitchRatioTest);
    }
#endif
}

void VoiceChanger_wczAudioProcessor::setTarget(TransportFileType st)
{
    switch (st)
    {
    case Source:
        pPlayBuffer = &sourceBuffer;
        break;
    case Target:
        pPlayBuffer = &targetBuffer;
        break;
    case Mainpage:
        pPlayBuffer = &fileBuffer;
        break;
    default:
        break;
    }
}
void VoiceChanger_wczAudioProcessor::setState(TransportState newState)
{
    switch (newState)
    {
    case Starting:
        shouldProcessFile = true;
        break;
    case Stopping:
        shouldProcessFile = false;
        break;
    default:
        break;
    }
}

void VoiceChanger_wczAudioProcessor::loadFileIntoTransport(const File& audioFile)//将读取的文件放入播放控制模块
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;

    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);//获取对应类型的读取器
    currentlyLoadedFile = audioFile;

    if (reader != nullptr)
    {
        currentAudioFileSource = new AudioFormatReaderSource(reader, true);

        transportSource.setSource(
            currentAudioFileSource,
            32768,
            &readAheadThread,
            reader->sampleRate
        );//设置播放控制参数
    }
}
void VoiceChanger_wczAudioProcessor::stop()
{
    {//停止内录
        const ScopedLock s1(writerLock);
        activeWriter = nullptr;
    }
    threadedWriter = nullptr;
}

void VoiceChanger_wczAudioProcessor::startRecording(const File& file)
{
    stop();
    if (sampleRate > 0)
    {
        file.deleteFile();
        if (auto fileStream = std::unique_ptr<FileOutputStream>(file.createOutputStream()))
        {
            WavAudioFormat wavFormat;
            if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 2, 16, {}, 0))
            {
                fileStream.release();
                threadedWriter.reset(new AudioFormatWriter::ThreadedWriter
                (writer, internalWriteThread, 32768));

                const ScopedLock s1(writerLock);//抢互斥锁
                activeWriter = threadedWriter.get();//获取写入线程
            }
        }
    }
}
void VoiceChanger_wczAudioProcessor::stopRecording()
{
    stop();
    lastRecording = parentDir.getNonexistentChildFile("VoiceChanger_wcz Recording", ".wav");//更新文件名避免下一次录制覆盖
}
void VoiceChanger_wczAudioProcessor::updateReverbSettings()
{
	reverbParams.roomSize = state.getParameter(ParamNames::size)->getValue();
    reverbParams.damping = state.getParameter(ParamNames::damp)->getValue();
    reverbParams.width = state.getParameter(ParamNames::width)->getValue();
    reverbParams.wetLevel = state.getParameter(ParamNames::dryWet)->getValue();
    reverbParams.dryLevel = 1.0f - state.getParameter(ParamNames::dryWet)->getValue();
    reverbParams.freezeMode = state.getParameter(ParamNames::freeze)->getValue();
    reverb.setParameters(reverbParams);
}



