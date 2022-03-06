#include "PluginProcessor.h"
#include "PluginEditor.h"


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

juce::String VoiceChanger_wczAudioProcessor::getBandID(size_t index)
{
    switch (index)
    {
    case 0: return "Lowest";
    case 1: return "Low";
    case 2: return "Low Mids";
    case 3: return "High Mids";
    case 4: return "High";
    case 5: return "Highest";
    default: break;
    }
    return "unknown";
}

int VoiceChanger_wczAudioProcessor::getBandIndexFromID(juce::String paramID)
{
    for (size_t i = 0; i < 6; ++i)
        if (paramID.startsWith(getBandID(i) + "-"))
            return int(i);

    return -1;
}

std::vector<VoiceChanger_wczAudioProcessor::Band> createDefaultBands()
{
    std::vector<VoiceChanger_wczAudioProcessor::Band> defaults;
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("Lowest"), juce::Colours::blue, VoiceChanger_wczAudioProcessor::HighPass, 20.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("Low"), juce::Colours::brown, VoiceChanger_wczAudioProcessor::LowShelf, 250.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("Low Mids"), juce::Colours::green, VoiceChanger_wczAudioProcessor::Peak, 500.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("High Mids"), juce::Colours::coral, VoiceChanger_wczAudioProcessor::Peak, 1000.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("High"), juce::Colours::orange, VoiceChanger_wczAudioProcessor::HighShelf, 5000.0f, 0.707f));
    defaults.push_back(VoiceChanger_wczAudioProcessor::Band(TRANS("Highest"), juce::Colours::red, VoiceChanger_wczAudioProcessor::LowPass, 12000.0f, 0.707f));
    return defaults;
}
#endif

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;

    {
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
#if _OPEN_FILTERS
    // setting defaults

    const float maxGain = juce::Decibels::decibelsToGain(24.0f);
    auto defaults = createDefaultBands();

    {
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
    return { params.begin(), params.end() };
}



//==============================================================================
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
    , pPlayBuffer(&fileBuffer)
    , ti(Stopped, Mainpage)
    , TransportInformation(ti)
    , spectrum(juce::Image::RGB, 512, 256, true)

#endif

    , trainingTemplate(sourceBufferAligned, targetBufferAligned, voiceChangerParameter)
    , state(*this, &undo, "PARAMS", createParameterLayout())

{
#if _OPEN_FILTERS
    frequencies.resize(300);
    for (size_t i = 0; i < frequencies.size(); ++i) {
        frequencies[i] = 20.0 * std::pow(2.0, i / 30.0);
    }
    magnitudes.resize(frequencies.size());

    // needs to be in sync with the ProcessorChain filter
    bands = createDefaultBands();

    for (size_t i = 0; i < bands.size(); ++i)
    {
        bands[i].magnitudes.resize(frequencies.size(), 1.0);

        state.addParameterListener(getTypeParamName(i), this);
        state.addParameterListener(getFrequencyParamName(i), this);
        state.addParameterListener(getQualityParamName(i), this);
        state.addParameterListener(getGainParamName(i), this);
        state.addParameterListener(getActiveParamName(i), this);

    }
    state.addParameterListener(paramOutput, this);

    state.addParameterListener("left", this);
    state.addParameterListener("right", this);
    state.addParameterListener("rmsPeriod", this);
    state.addParameterListener("smoothing", this);

    

    state.state = juce::ValueTree(JucePlugin_Name);
	#endif


    formatManager.registerBasicFormats();
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
    pOversample.reset(new juce::dsp::Oversampling<float>(getTotalNumOutputChannels(), 3,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true));

    pOversample->initProcessing(static_cast<size_t>(samplesPerBlock));


#if _OPEN_FILTERS
    juce::dsp::ProcessSpec fspec;
    fspec.sampleRate = sampleRate;
    fspec.maximumBlockSize = juce::uint32(sampleRate);
    fspec.numChannels = juce::uint32(getTotalNumOutputChannels());

    for (size_t i = 0; i < bands.size(); ++i) {
        updateBand(i);
    }
    filter.get<6>().setGainLinear(*state.getRawParameterValue(paramOutput));

    updatePlots();

    filter.prepare(fspec);

    inputAnalyser.setupAnalyser(int(sampleRate), float(sampleRate));
    outputAnalyser.setupAnalyser(int(sampleRate), float(sampleRate));
#endif



    const auto numberOfChannels = getTotalNumInputChannels();
    rmsLevels.clear();
    for (auto i = 0; i < numberOfChannels; i++)
    {
        juce::LinearSmoothedValue<float> rms{ -100.0f };
        rms.reset(sampleRate, 0.5);
        rmsLevels.emplace_back(std::move(rms));
    }
    rmsFifo.reset(numberOfChannels, static_cast<int>(sampleRate) + 1);
    rmsCalculationBuffer.clear();
    rmsCalculationBuffer.setSize(numberOfChannels, static_cast<int>(sampleRate + 1));


    gainLeft.reset(sampleRate, 0.2);;
    gainLeft.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("left")->load()));

    gainRight.reset(sampleRate, 0.2);
    gainRight.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("right")->load()));

    rmsWindowSize = static_cast<int>(sampleRate * state.getRawParameterValue("rmsPeriod")->load()) / 1000;
    isSmoothed = false; //static_cast<bool>(state.getRawParameterValue("smooth")->load());

    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

#if USE_3rdPARTYPITCHSHIFT
#if USE_RUBBERBAND
    rbs = std::make_unique<PitchShifterRubberband>(getTotalNumInputChannels(), sampleRate, samplesPerBlock);
#endif
#if USE_SOUNDTOUCH
    sts = std::make_unique<PitchShifterSoundTouch>(getTotalNumInputChannels(), sampleRate, samplesPerBlock);
#endif
#endif

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

#if _OPEN_WAHWAH
        filtersForWahWah.add(filter = new Filter());
#endif
#if _OPEN_PEAK_PITCH
        PitchShifter* pPitchShifter;
        pitchShifters.add(pPitchShifter = new PitchShifter());

        PeakShifter* pPeakShifter;
        peakShifters.add(pPeakShifter = new PeakShifter());


        VocoderForVoiceConversion* pVocodersForVoiceConversion;
        vocodersForVoiceConversion.add(pVocodersForVoiceConversion = new VocoderForVoiceConversion());

        const auto windows = pitchShifters[0]->getLatencyInSamples();
#endif

#if _OPEN_TEST
        ShapeInvariantPitchShifter* pShapeInvariantPitchShifter;
        shapeInvariantPitchShifters.add(pShapeInvariantPitchShifter = new ShapeInvariantPitchShifter());
        const auto windows1 = shapeInvariantPitchShifters[0]->getLatencyInSamples();
        setLatencySamples(windows1);
#endif
    }
 
#if _OPEN_DYNAMICS
    mixedDownInputDynamics.setSize(1, samplesPerBlock);
    inputLevelDynamics = 0.0f;
    ylPrevDynamics = 0.0f;
    inverseEDynamics = 1.0f / juce::MathConstants<double>::euler;
    inverseSampleRateDynamics = 1.0f / (float)sampleRate;
#endif
    
}

void VoiceChanger_wczAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
#if _OPEN_FILTERS
    inputAnalyser.stopThread(1000);
    outputAnalyser.stopThread(1000);
#endif
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VoiceChanger_wczAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
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
    if (realtimeMode)
    {
		if(!openVoiceConversion)
			processLevelInfo(buffer);
        overallProcess(buffer);
    }
    else
    {
        buffer.clear();
        
        if (!canReadSampleBuffer)
        {
            if (pPlayBuffer)
                if (pPlayBuffer->getNumChannels())
                    canReadSampleBuffer = true;
        }
        if (canReadSampleBuffer)
        {
            if (pPlayBuffer->getNumChannels())
            {
                if (shouldProcessFile)
                {
                    getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
                    // transportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
                    overallProcess(buffer);
                    return;
                }
            }
            else
                spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
        }
        spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
    }
}
void VoiceChanger_wczAudioProcessor::getNextAudioBlock(juce::AudioSourceChannelInfo& buffer)
{
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

void VoiceChanger_wczAudioProcessor::overallProcess(juce::AudioBuffer<float>& buffer)
{
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    int numSamples = buffer.getNumSamples();

    updateUIControls();

#if _OPEN_DYNAMICS

    processDynamics(buffer, false, getDynamicsThresholdShift(),
        getDynamicsRatioShift(), getDynamicsAttackShift(),
        getDynamicsReleaseShift(), getDynamicsMakeupGainShift());
#endif
#if _OPEN_WAHWAH
    //processWahwah(juce::AudioBuffer<float>&buffer,float attackValue,float releaseValue,float MixLFOAndEnvelope,
    // float lfoFrequency,float mixRatio,float filterQFactor,float filterGain,float filterFreq)
    processWahwah(buffer, 0.02, 0.3, 0.8, 5, 0.5, 20, 2, 200.0f);
#endif

#if _OPEN_PEAK_PITCH
#if USE_3rdPARTYPITCHSHIFT
    if (!openVoiceConversion)
    {
#if USE_SOUNDTOUCH
    	if (!useFD)
        	sts->processBuffer(buffer);
#if USE_RUBBERBAND
    	if (useFD)
    		rbs->processBuffer(buffer);
    }
#endif

#endif
#endif
    
#if _SHOW_SPEC
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        if (openVoiceConversion)
        {
            //pOversample->setUsingIntegerLatency(true);
            //setLatencySamples(static_cast<int>(pOversample->getLatencyInSamples()));
            //dsp::AudioBlock<float>downSampleBlock(buffer);
            //dsp::AudioBlock<float>downSampleRateBlock = pOversample->processSamplesUp(downSampleBlock);
            //pOversample->processSamplesDown(downSampleBlock);

            // AudioDataConverters::convertFloatToFloat32LE(buffer.getReadPointer(channel), dblBuffer.getReadPointer(channel), buffer.getNumSamples());


        	float* channelDataFlt = buffer.getWritePointer(channel);
        	// double* channelData = dblBuffer.getWritePointer(channel);
            vocodersForVoiceConversion[channel]->process(channelDataFlt, numSamples);
            // buffer.clear();

        }
        auto channelDataFlt = buffer.getWritePointer(channel);
        pitchShifters[channel]->process(channelDataFlt, numSamples);
        // peakShifters[channel]->process(channelData, numSamples);
    }
#endif
#endif
#if _OPEN_FILTERS

    if (getActiveEditor() != nullptr)
        inputAnalyser.addAudioData(buffer, 0, getTotalNumInputChannels());

    if (wasBypassed) {
        filter.reset();
        wasBypassed = false;
    }
    juce::dsp::AudioBlock<float>              ioBuffer(buffer);
    juce::dsp::ProcessContextReplacing<float> context(ioBuffer);
    filter.process(context);

    if (getActiveEditor() != nullptr)
        outputAnalyser.addAudioData(buffer, 0, getTotalNumOutputChannels());


#endif
#if _OPEN_TEST

    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        // spectrumFilter[channel]->process(channelData, numSamples);
        shapeInvariantPitchShifters[channel]->process(channelData, numSamples);

    }
#endif
    
}


void VoiceChanger_wczAudioProcessor::processLevelInfo(juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    {
        const auto startGain = gainLeft.getCurrentValue();
        gainLeft.skip(numSamples);

        const auto endGain = gainLeft.getCurrentValue();
        buffer.applyGainRamp(0, 0, numSamples, startGain, endGain);
    }
    {
        const auto startGain = gainRight.getCurrentValue();
        gainRight.skip(numSamples);
        const auto endGain = gainRight.getCurrentValue();
        buffer.applyGainRamp(1, 0, numSamples, startGain, endGain);
    }
    for (auto& rmsLevel : rmsLevels)
        rmsLevel.skip(numSamples);
    rmsFifo.push(buffer);

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
void VoiceChanger_wczAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
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

std::vector<float> VoiceChanger_wczAudioProcessor::getRmsLevels()
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
    rmsFifo.pull(rmsCalculationBuffer.getWritePointer(channel), channel, rmsWindowSize);
    processLevelValue(rmsLevels[channel], juce::Decibels::gainToDecibels(rmsCalculationBuffer.getRMSLevel(channel, 0, rmsWindowSize)));
    return rmsLevels[channel].getCurrentValue();
}
void VoiceChanger_wczAudioProcessor::processLevelValue(juce::LinearSmoothedValue<float>& smoothedValue, const float value)const
{
    if (isSmoothed)
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

void VoiceChanger_wczAudioProcessor::updateBypassedStates()
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

VoiceChanger_wczAudioProcessor::Band* VoiceChanger_wczAudioProcessor::getBand(size_t index)
{
    if (juce::isPositiveAndBelow(index, bands.size()))
        return &bands[index];
    return nullptr;
}

juce::StringArray VoiceChanger_wczAudioProcessor::getFilterTypeNames()
{
    return {
        TRANS("No Filter"),
        TRANS("High Pass"),
        TRANS("1st High Pass"),
        TRANS("Low Shelf"),
        TRANS("Band Pass"),
        TRANS("All Pass"),
        TRANS("1st All Pass"),
        TRANS("Notch"),
        TRANS("Peak"),
        TRANS("High Shelf"),
        TRANS("1st Low Pass"),
        TRANS("Low Pass")
    };
}

void VoiceChanger_wczAudioProcessor::updateBand(const size_t index)
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
                // minimise lock scope, get<0>() needs to be a  compile time constant
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
        updateBypassedStates();
        updatePlots();
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

    sendChangeMessage();
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
        inputAnalyser.createPath(p, bounds.toFloat(), minFreq);
    else
        outputAnalyser.createPath(p, bounds.toFloat(), minFreq);
}

bool VoiceChanger_wczAudioProcessor::checkForNewAnalyserData()
{
    return inputAnalyser.checkForNewData() || outputAnalyser.checkForNewData();
}
//==============================================================================
void VoiceChanger_wczAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
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

#if _OPEN_DYNAMICS
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
//bool VoiceChanger_wczAudioProcessor::isUpdateParameter()
//{
//    bool result = updataParamFlag;
//    updataParamFlag = false;
//    return result;
//}
//void VoiceChanger_wczAudioProcessor::syncPluginParameter()
//{
//    if (*nPitchShift != postPitchShift)
//    {
//        pitchShifter.setPitchRatio(*nPitchShift);
//        pitchShifterRight.setPitchRatio(*nPitchShift);
//        postPitchShift = *nPitchShift;
//        updataParamFlag = true;
//    }
//    if (*nPeakShift != postPeakShift)
//    {
//        peakShifter.setPitchRatio(*nPeakShift);
//        postPeakShift = *nPeakShift;
//        updataParamFlag = true;
//    }
//}
juce::Image& VoiceChanger_wczAudioProcessor::getSpectrumView()
{
    if (pitchShifters[0]->getProcessFlag())
    {
        spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
        auto level = pitchShifters[0]->getSpectrumInput();
        drawSpectrumGraph(spectrum, level, juce::Colours::hotpink, true);//juce::Colour(0, 255, 0), true);
        pitchShifters[0]->setProcessFlag(false); 
    }
    return spectrum;
}
void VoiceChanger_wczAudioProcessor::drawSpectrumGraph(juce::Image view, std::shared_ptr<float>power, juce::Colour color, bool isLog)
{
    int postPoint = 0;
    float postLevel = 0.0f;
    juce::Graphics g(view);
    for (int x = 1; x < 512; x++)
    {
        float skewedProportionX = 0.0f;
        if (isLog)
        
        {
            skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)x / 512.0f) * 0.2f);
        }
        else
        {
            skewedProportionX = (float)x / 512.0f;
        }
        auto fftDataIndex = juce::jlimit(0, 1024, (int)(skewedProportionX * 1024.));
        auto lv = power.get()[fftDataIndex];
        if ((std::fabs(postLevel - lv) > 0.000001) || (x == 511) || (!isLog))
        {
            g.setColour(color);
            g.setOpacity(1.0);
            g.drawLine(
                (float)postPoint,
                juce::jmap(postLevel, 0.0f, 1.0f, 256.0f, 0.0f),
                (float)x,
                juce::jmap(lv, 0.0f, 1.0f, 256.0f, 0.0f)

            );
            {
                g.setOpacity(0.3);
                juce::Path pen;
                pen.startNewSubPath(juce::Point<float>((float)postPoint, juce::jmap(postLevel, 0.0f, 1.0f, 256.0f, 0.0f)));
                pen.lineTo(juce::Point<float>((float)x, juce::jmap(lv, 0.0f, 1.0f, 256.0f, 0.0f)));
                pen.lineTo(juce::Point<float>((float)x, 256.0f));
                pen.lineTo(juce::Point<float>((float)postPoint, 256.0f));
                pen.closeSubPath();
                g.fillPath(pen);
            }
            postPoint = x;
            postLevel = lv;
        }
    }
    g.setOpacity(1.0);
    // syncPluginParameter();
}

void VoiceChanger_wczAudioProcessor::updateUIControls()
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
    for (int i = 0; i < peakShifters.size(); ++i)
    {
        // peakShifters[i]->setPitchRatio(peakRatio);
    }
#endif
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
void VoiceChanger_wczAudioProcessor::alignBuffer(juce::AudioSampleBuffer& s, juce::AudioSampleBuffer& t)
{
    if(s.getNumChannels()>=2)
        if (s.getNumChannels() == t.getNumChannels())
        {
        
        }
}