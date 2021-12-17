/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    , spectrum(juce::Image::RGB, 512, 256, true)
    , filters(new juce::OwnedArray<Filter>[nFiltersPerChannel])
    , filterIndex(createFilterIndexArray(_FILTER_NUM))
    //, updataParamFlag(false)
    // , bandpassFilter(juce::dsp::IIR::Coefficients<float>::makeBandPass(22050, 5000.0f, 0.1))

#endif
{
    addParameter(nPitchShift = new juce::AudioParameterFloat("PitchShift", "pitchShift", -12.0f, 12.0f, 0.0f));
    addParameter(nPeakShift = new juce::AudioParameterFloat("PeakShift", "peakShift", 0.5f, 2.0f, 1.f));
    addParameter(nFilterFreq = new juce::AudioParameterFloat("FilterFreq", "filterFreq", 80, 8000, 1));
    addParameter(nFilterQFactor = new juce::AudioParameterFloat("FilterQFactor", "filterQFactor", 0.01, 5.0, 1.f));
    addParameter(nFilterGain = new juce::AudioParameterFloat("FilterGain", "filterGain", -50, 1.0, 0.5f));

    addParameter(nFilterType = new juce::AudioParameterInt("FilterType", "filterType", 1, 8, 6));
    // addParameter(nFilter2Type = new juce::AudioParameterInt("Filter2Type", "filter2Type", 1, 8, 6));
    addParameter(nFilterIndex = new juce::AudioParameterInt("FilterIndex", "filterIndex", 0, nFiltersPerChannel, 1));

    addParameter(nDynamicsThreshold = new juce::AudioParameterFloat("DynamicsThreshold", "dynamicsThreshold", -50, 0.0, -3.0f));
    addParameter(nDynamicsRatio = new juce::AudioParameterFloat("DynamicsRatio", "dynamicsRatio", 1.0f, 25.0f, 5.0f));
    addParameter(nDynamicsAttack = new juce::AudioParameterFloat("DynamicsAttack", "dynammicsAttack", 0.00001f, 1.0f, 0.002f));
    addParameter(nDynamicsRelease = new juce::AudioParameterFloat("DynamicsRelease", "dynamicsRelease", 0.001f, 2.0f, 0.1f));
    addParameter(nDynamicsMakeupGain = new juce::AudioParameterFloat("DynamicsMakeupGain", "dynamicsMakeupGain", -5.0f, 30.0f, 0.0f));
}

VoiceChanger_wczAudioProcessor::~VoiceChanger_wczAudioProcessor()
{
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

    rbs = std::make_unique<PitchShifterRubberband>(getTotalNumInputChannels(), sampleRate, samplesPerBlock);

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
//#if _OPEN_FILTERS
    for (int i = 0; i < nFiltersPerChannel; i++)
        filters[i].clear();
//#endif
#
#if _OPEN_TEST
    shapeInvariantPitchShifters.clear();
#endif

    for (int i = 0; i < getTotalNumInputChannels(); ++i)
    {
//#if _OPEN_FILTERS
        Filter* filter;
        for (int j = 0; j < nFiltersPerChannel; j++)
            filters[j].add(filter = new Filter());
//#endif
#if _OPEN_WAHWAH
        filtersForWahWah.add(filter = new Filter());
#endif
#if _OPEN_PEAK_PITCH
        
        PitchShifter* pPitchShifter;
        pitchShifters.add(pPitchShifter = new PitchShifter());

        PeakShifter* pPeakShifter;
        peakShifters.add(pPeakShifter = new PeakShifter());

        const auto windows = pitchShifters[0]->getLatencyInSamples();

#endif
        // SpectrumFilter* pSpectrumFilter;
        // spectrumFilter.add(pSpectrumFilter = new SpectrumFilter());
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
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    int numSamples = buffer.getNumSamples();

    updateUIControls();
    // DBG(rbs->getPitchScale());
#if _OPEN_DYNAMICS
    
    processDynamics(buffer, false ,getDynamicsThresholdShift(), 
        getDynamicsRatioShift(), getDynamicsAttackShift(), 
        getDynamicsReleaseShift(), getDynamicsMakeupGainShift());
#endif
#if _OPEN_WAHWAH
    //processWahwah(juce::AudioBuffer<float>&buffer,float attackValue,float releaseValue,float MixLFOAndEnvelope,
    // float lfoFrequency,float mixRatio,float filterQFactor,float filterGain,float filterFreq)
    processWahwah(buffer, 0.02, 0.3, 0.8, 5, 0.5, 20, 2, 200.0f);
#endif
#if _OPEN_FILTERS
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        for (int j = 0; j < nFiltersPerChannel; ++j)
        {
            filters[j][channel]->processSamples(channelData, numSamples);
        }
    }
#endif
#if _OPEN_PEAK_PITCH

    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        pitchShifters[channel]->process(channelData, numSamples);
        //peakShifters[channel]->process(channelData, numSamples);
    }
    float* channelData = buffer.getWritePointer(0);
    auto readPointers = buffer.getArrayOfWritePointers();
    auto writePointers = buffer.getArrayOfWritePointers();
    // auto samplesAvailableFromStretcher = rb->available();
    
    //rb->process(writePointers, buffer.getNumSamples(), false);
    rbs->processBuffer(buffer);
    
    // auto rbOutput = rb->retrieve(writePointers, buffer.getNumSamples());
    // transportSource.getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
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
    

//==============================================================================
bool VoiceChanger_wczAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VoiceChanger_wczAudioProcessor::createEditor()
{
    return new VoiceChanger_wczAudioProcessorEditor (*this);
}

//==============================================================================
void VoiceChanger_wczAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("VoiceChangerWczParam"));

    xml->setAttribute("pitchShift", (double)*nPitchShift);
    xml->setAttribute("peakShift", (double)*nPeakShift);
    xml->setAttribute("filterFreq", (double)*nFilterFreq);
    xml->setAttribute("filterQFactor", (double)*nFilterQFactor);
    xml->setAttribute("filterGain", (double)*nFilterGain);
    xml->setAttribute("filterType", (int)*nFilterType);

    copyXmlToBinary(*xml, destData);
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
            *nFilterFreq = (float)xmlState->getDoubleAttribute("filterFreq", 5000);
            *nFilterQFactor = (float)xmlState->getDoubleAttribute("filterQFactor", 1.0);
            *nFilterGain = (float)xmlState->getDoubleAttribute("filterGain", 1.0);
            *nFilterType = (int)xmlState->getIntAttribute("filterType", 1);

            //postPitchShift = *nPitchShift = (float)xmlState->getDoubleAttribute("pitchShift", 1.0);
            //postPeakShift = *nPeakShift = (float)xmlState->getDoubleAttribute("peakShift", 1.0);
        }
    }
    //postPeakShift = 1.0;
    //postPitchShift = 1.0;
    //syncPluginParameter();
}

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
    *nPitchShift = pitch;
}
void VoiceChanger_wczAudioProcessor::setPeakShift(float peak)
{
    *nPeakShift = peak;
}


void VoiceChanger_wczAudioProcessor::setFilterFreqShift(float freq, int filterIndex)
{
    //*bandpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(lastSampleRate, freq, *nBandpassQ);
    //*nBandpassFreq = freq;
    //postBandpassFreq = freq;
    *nFilterFreq = freq;
    for (int channel = 0; channel < getTotalNumInputChannels(); channel++)
    {
        filters[filterIndex - 1][channel]->freq = freq;
    }
}
void VoiceChanger_wczAudioProcessor::setFilterQFactorShift(float q, int filterIndex)
{
    //*bandpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(lastSampleRate, *nBandpassFreq, q);
    // *nFilterQFactor = q;
    for (int channel = 0; channel < getTotalNumInputChannels(); channel++)
    {
        filters[filterIndex - 1][channel]->qFactor = q;
    }
    //postBandpassQ = q;
}
void VoiceChanger_wczAudioProcessor::setFilterGainShift(float gain,int filterIndex)
{
    // *nFilterGain = gain;
    for (int channel = 0; channel < getTotalNumInputChannels(); channel++)
    {
        filters[filterIndex - 1][channel]->gain = gain;
    }
}
void VoiceChanger_wczAudioProcessor::setFilterTypeShift(int filterType,int filterIndex)
{
    // *nFilterType = filterType;
    for (int channel = 0; channel < getTotalNumInputChannels(); channel++)
    {
        filters[filterIndex - 1][channel]->filterType = filterType;
    }
}
double VoiceChanger_wczAudioProcessor::getFilterFreqShift(int filterIndex)
{
    return filters[filterIndex - 1][0]->freq;
}
double VoiceChanger_wczAudioProcessor::getFilterQFactorShift(int filterIndex)
{
    return filters[filterIndex - 1][0]->qFactor;
}
double VoiceChanger_wczAudioProcessor::getFilterGainShift(int filterIndex)
{
    return filters[filterIndex - 1][0]->gain;
}
int VoiceChanger_wczAudioProcessor::getFilterTypeShift(int filterIndex)
{
    return filters[filterIndex - 1][0]->filterType;
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
        drawSpectrumGraph(spectrum, level, juce::Colour(0, 255, 0), true);
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
#if _OPEN_FILTERS
    double discreteFrequency = 2.0 * juce::MathConstants<double>::pi * (double)getFilterFreqShift(currentFilterIndex) / getSampleRate();
    double qFactor = (double)getFilterQFactorShift(currentFilterIndex);
    double gain = pow(10.0, (double)getFilterGainShift(currentFilterIndex) * 0.05);
    int type = (int)getFilterTypeShift(currentFilterIndex);
    for (int i = 0; i < getNumInputChannels(); ++i)
    {
        filters[currentFilterIndex - 1][i]->updateCoefficients(discreteFrequency, qFactor, gain, type);
    }
#endif
#if _OPEN_PEAK_PITCH
    float pitchRatio = getPitchShift();
    float peakRatio = getPeakShift();
    // rb->setPitchScale(pitchRatio);
    rbs->setSemitoneShift(pitchRatio);
    for (int i = 0; i < pitchShifters.size(); ++i)
    {
        // pitchShifters[i]->setPitchRatio(pitchRatio);
        // shapeInvariantPitchShifters[i]->setPitchRatio(pitchRatio);
    }
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