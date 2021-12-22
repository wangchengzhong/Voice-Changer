#pragma once
#define _USE_MATH_DEFINES
#define _OPEN_FILTERS true
#define _OPEN_PEAK_PITCH true
#define _OPEN_WAHWAH false
#define _OPEN_DYNAMICS true
#define _OPEN_TEST false
#define _FILTER_NUM 5
#define _FILTER_PROCESS false

#if _OPEN_PEAK_PITCH
#define _SHOW_SPEC true

#define USE_3rdPARTYPITCHSHIFT true
#if USE_3rdPARTYPITCHSHIFT
#define USE_RUBBERBAND false
#define USE_SOUNDTOUCH true
#endif

#endif


#include <JuceHeader.h>
#include"juce_audio_plugin_client//Standalone//juce_StandaloneFilterWindow.h"
#include"SoundTouch.h"
#include"PitchShifter.h"
#include"PeakShifter.h"
//#include"PluginFilter.h"
#include<cmath>
#include "PluginFilter.h"
// #include "ManVoiceFilter.h"
#include "ShapeInvariantPitchShifter.h"
#include"rubberband/RubberBandStretcher.h"
#include"rubberband/rubberband-c.h"
#include"PitchShifterRubberband.h"
#include"PitchShifterSoundTouch.h"

//==============================================================================
/**
*/
class VoiceChanger_wczAudioProcessor : public juce::AudioProcessor//,public Filter
{
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

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::Image& getSpectrumView();

    void setPitchShift(float pitch);
    void setPeakShift(float peak);



    void setDynamicsThresholdShift(float threshold);
    void setDynamicsRatioShift(float ratio);
    void setDynamicsAttackShift(float attack);
    void setDynamicsReleaseShift(float release);
    void setDynamicsMakeupGainShift(float makeupGain);

    float getPitchShift();
    float getPeakShift();

    float getDynamicsThresholdShift();
    float getDynamicsRatioShift();
    float getDynamicsAttackShift();
    float getDynamicsReleaseShift();
    float getDynamicsMakeupGainShift();
#if _OPEN_FILTERS
    void setFilterFreqShift(float freq, int filterIndex);
    void setFilterQFactorShift(float q, int filterIndex);
    void setFilterTypeShift(int filterType, int filterIndex);
    void setFilterGainShift(float gain, int filterIndex);

    double getFilterFreqShift(int filterIndex);
    double getFilterQFactorShift(int filterIndex);
    double getFilterGainShift(int filterIndex);
    int getFilterTypeShift(int filterIndex);

    juce::StringArray filterIndex = {
        "1",
        "2",
        //"3",
        //"4",
        //"5",
        //"6",
        //"7",
    };
    juce::StringArray createFilterIndexArray(int filterNum)
    {
        juce::StringArray filterIndex;
        for (int i = 1; i <= filterNum; i++)
        {
            filterIndex.add(juce::String(i));
        }
        return filterIndex;
    }
    const int nFiltersPerChannel{ _FILTER_NUM };
    int currentFilterIndex{ 1 };
    juce::StringArray filterTypeItemsUI = {
        "low-pass",
        "High-pass",
        "Low-shelf",
        "High-shelf",
        "Band-pass",
        "Band-stop",
        "Peaking&Notch",
        "ResonantLowPass"
    };
    juce::OwnedArray<Filter>* filters;
    //enum filterTypeIndex {
    //    filterTypeLowPass = 1,
    //    filterTypeHighPass,
    //    filterTypeLowShelf,
    //    filterTypeHighShelf,
    //    filterTypeBandPass,
    //    filterTypeBandStop,
    //    filterTypePeakingNotch,
    //    filterTypeResonantLowPass
    //};
private:
    juce::AudioParameterFloat* nFilterQFactor;
    juce::AudioParameterFloat* nFilterFreq;
    juce::AudioParameterFloat* nFilterGain;

    juce::AudioParameterInt* nFilterType;
    juce::AudioParameterInt* nFilter2Type;
    juce::AudioParameterInt* nFilterIndex;
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
    juce::Image spectrum;
#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND
    std::unique_ptr<PitchShifterRubberband> rbs;
    const int rbOptions = RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency;
#endif // USE_3rdPARTYPITCHSHIFT
#if USE_SOUNDTOUCH
    std::unique_ptr<PitchShifterSoundTouch> sts;
#endif
#endif

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChanger_wczAudioProcessor)
};