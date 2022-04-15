#pragma once
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
#include"SocialButtons.h"
#include"VocoderForVoiceConversion.h"
#include"VoiceConversionBuffer.h"

// #include"include/webrtc_vad.h"
//==============================================================================
/**
*/
class VoiceChanger_wczAudioProcessor :
	 public juce::AudioProcessor//,public juce::AudioAppComponent//,public Filter
    ,public TransportInformation
    ,public juce::AudioProcessorValueTreeState::Listener
	,public juce::ChangeBroadcaster
{
public:
    void stop();
    CriticalSection writerLock;
    bool openVoiceConversion{ false };
    bool isInternalRecording{ false };
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
    static juce::String paramOutput;
    static juce::String paramType;
    static juce::String paramFrequency;
    static juce::String paramQuality;
    static juce::String paramGain;
    static juce::String paramActive;

    static juce::String getBandID(size_t index);
    static juce::String getTypeParamName(size_t index);
    static juce::String getFrequencyParamName(size_t index);
    static juce::String getQualityParamName(size_t index);
    static juce::String getGainParamName(size_t index);
    static juce::String getActiveParamName(size_t index);
public:
    //==============================================================================
    VoiceChanger_wczAudioProcessor();
    ~VoiceChanger_wczAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void overallProcess(juce::AudioBuffer<float>& buffer);

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    //==============================================================================

    juce::AudioProcessorValueTreeState& getPluginState();

#if _OPEN_FILTERS
    size_t getNumBands() const;
    juce::String getBandName(size_t index) const;
    juce::Colour getBandColour(size_t index) const;

    void setBandSolo(int index);
    bool getBandSolo(int index) const;
    
    static juce::StringArray getFilterTypeNames();

	juce::AudioProcessorEditor* createEditor() override;


	bool hasEditor() const override;

    const std::vector<double>& getMagnitudes();

    void createFrequencyPlot(juce::Path& p, const std::vector<double>& mags, const juce::Rectangle<int> bounds, float pixelsPerDouble);

    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq, bool input);

    bool checkForNewAnalyserData();
    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void  getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::Point<int> getSavedSize() const;
    void setSavedSize(const juce::Point<int>& size);


    juce::Image& getSpectrumView();

    void setPitchShift(float pitch);
    void setPeakShift(float peak);
    bool useFD{ false };
    

    void setDynamicsThresholdShift(float threshold);
    void setDynamicsRatioShift(float ratio);
    void setDynamicsAttackShift(float attack);
    void setDynamicsReleaseShift(float release);
    void setDynamicsMakeupGainShift(float makeupGain);

    void setPlayAudioFilePosition(float position);

    float getPitchShift();
    float getPeakShift();

    float getDynamicsThresholdShift();
    float getDynamicsRatioShift();
    float getDynamicsAttackShift();
    float getDynamicsReleaseShift();
    float getDynamicsMakeupGainShift();

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

        juce::String name;
        juce::Colour colour;
        FilterType   type = BandPass;
        float        frequency = 1000.0f;
        float        quality = 1.0f;
        float        gain = 1.0f;
        bool         active = true;
        std::vector<double> magnitudes;
    };

    Band* getBand(size_t index);
    int getBandIndexFromID(juce::String paramID);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceChanger_wczAudioProcessor);
    void updateBand(const size_t index);

    void updateBypassedStates();

    void updatePlots();

    juce::UndoManager                  undo;
#endif

    juce::AudioProcessorValueTreeState state;
#if _OPEN_FILTERS
    std::vector<Band>    bands;

    std::vector<double> frequencies;
    std::vector<double> magnitudes;

    bool wasBypassed = true;

    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using Gain = juce::dsp::Gain<float>;
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, FilterBand, Gain> filter;
    


    double sampleRate = 48000;

    int soloed = -1;

    Analyser<float> inputAnalyser;
    Analyser<float> outputAnalyser;


    juce::Point<int> editorSize = { 900, 500 };
#endif


public:

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
    bool realtimeMode{ true };

    juce::AudioFormatManager formatManager;

    juce::AudioTransportSource transportSource;
    File currentlyLoadedFile;
    AudioThumbnailCache thumbnailCache;
    ScopedPointer<AudioFormatReaderSource>currentAudioFileSource;
    TimeSliceThread readAheadThread;
    void loadFileIntoTransport(const File& audioFile);


    juce::AudioSampleBuffer fileBuffer;
    int innerRecordSampleCount{ 0 };
    juce::AudioSampleBuffer sourceBuffer;
    juce::AudioSampleBuffer targetBuffer;


    // TrainingTemplate trainingTemplate;
    // std::vector<float> voiceChangerParameter;
    juce::AudioSampleBuffer* pPlayBuffer;
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


#if _OPEN_WAHWAH
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
    juce::OwnedArray<PitchShifter>pitchShifters;
    juce::OwnedArray<PeakShifter>peakShifters;


	//juce::OwnedArray<VocoderForVoiceConversion> vocodersForVoiceConversion;



#endif
#if _OPEN_TEST
    juce::OwnedArray<ShapeInvariantPitchShifter> shapeInvariantPitchShifters;
#endif
    void updateUIControls();

private:

    juce::AudioParameterFloat* nPitchShift{ 0 };
    juce::AudioParameterFloat* nPeakShift;


    juce::AudioParameterFloat* nDynamicsThreshold;
    juce::AudioParameterFloat* nDynamicsRatio;
    juce::AudioParameterFloat* nDynamicsAttack;
    juce::AudioParameterFloat* nDynamicsRelease;
    juce::AudioParameterFloat* nDynamicsMakeupGain;

    // bool updataParamFlag;
    void drawSpectrumGraph(juce::Image view, std::shared_ptr<float>level, juce::Colour colour, bool isLog);
    //void syncPluginParameter();
public:
    juce::Image spectrum;
private:
#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND
    std::unique_ptr<PitchShifterRubberband> rbs;
    const int rbOptions = RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;
#endif // USE_3rdPARTYPITCHSHIFT
#if USE_SOUNDTOUCH
    std::unique_ptr<PitchShifterSoundTouch> sts;
#endif
#endif
    std::unique_ptr<VoiceConversionBuffer> vcb;
    // VadInst* vadModule;
    // juce::AudioProcessorValueTreeState parameters;
    juce::LinearSmoothedValue<float> gainLeft, gainRight;

    std::vector<juce::LinearSmoothedValue<float>>rmsLevels;
    Utility::Fifo rmsFifo;
    juce::AudioBuffer<float>rmsCalculationBuffer;

    int rmsWindowSize = 50;

    int isSmoothed = true;
public:

    // juce::AudioProcessorValueTreeState& getApvts() { return parameters; }
    std::vector<float>getRmsLevels();
    float getRmsLevel(const int level);
    void processLevelValue(juce::LinearSmoothedValue<float>&, const float value)const;
    void processLevelInfo(juce::AudioBuffer<float>& buffer);
    //==============================================================================
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChanger_wczAudioProcessor)
};