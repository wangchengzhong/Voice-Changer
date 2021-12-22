#include <JuceHeader.h>
#include "rubberband/RubberBandStretcher.h"
#include "RingBuffer.h"
#if USE_RUBBERBAND
class PitchShifterRubberband
{
public:
    /** 
    Setup the pitch shifter. By default the shifter will be setup so that 
    the dry signal isn't delayed to be given a somewhat similar latency to the wet signal -
    this is not accurate when enabled! By enabling minLatency some latency can be reduced with the
    expense of potential tearing during modulation with a change of the pitch parameter.
     */
    PitchShifterRubberband(int numChannels, double sampleRate, int samplesPerBlock, bool dryCompensationDelay = false, bool minLatency = false)
    {
        rubberband = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, numChannels, 
            RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + 
            RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency, 1.0, 1.0);

        
        //soundtouch::SAMPLETYPE
        //rubberband->setMaxProcessSize(samplesPerBlock);
        initLatency = (int)rubberband->getLatency();
        maxSamples = 256;

        input.initialise(numChannels, sampleRate);
        output.initialise(numChannels, sampleRate);

        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = numChannels;
        spec.sampleRate = sampleRate;
        if (dryCompensationDelay)
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>>(samplesPerBlock * 3.0 + initLatency);
            dryWet->prepare(spec);
            dryWet->setWetLatency(samplesPerBlock * ((minLatency) ? 2.0 : 3.0) + initLatency);
        }
        else
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>>();
            dryWet->prepare(spec);
        }

        timeSmoothing.reset(sampleRate, 0.05);
        mixSmoothing.reset(sampleRate, 0.3);
        pitchSmoothing.reset(sampleRate, 0.1);

        if (minLatency)
        {
            smallestAcceptableSize = maxSamples * 1.0;
            largestAcceptableSize = maxSamples * 3.0;
        }
        else
        {
            smallestAcceptableSize = maxSamples * 2.0;
            largestAcceptableSize = maxSamples * 4.0;
        }
    }

    ~PitchShifterRubberband()
    {
    }

    /** Pitch shift a juce::AudioBuffer<float>
     */
    void processBuffer(juce::AudioBuffer<float>& buffer)
    {
        dryWet->pushDrySamples(buffer);

        pitchSmoothing.setTargetValue(powf(2.0, pitchParam / 12));          // Convert semitone value into pitch scale value.
        auto newPitch = pitchSmoothing.skip(buffer.getNumSamples());
        if (oldPitch != newPitch)
        {
            // st->setPitch(newPitch);
            rubberband->setPitchScale(newPitch);
            // st->setPitch(newPitch);
            oldPitch = newPitch;
        }

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) 
        {   
            // Loop to push samples to input buffer.
            for (int channel = 0; channel < buffer.getNumChannels(); channel++) 
            {
                input.pushSample(buffer.getSample(channel, sample), channel);
                buffer.setSample(channel, sample, 0.0);

                if (channel == buffer.getNumChannels() - 1) 
                {
                    // st->putSamples(input.readPointerArray(buffer.getNumSamples()), buffer.getNumSamples);
                    auto reqSamples = rubberband->getSamplesRequired();
                    // DBG(reqSamples);
                    // auto reqStSamples = st->putSamples();
                    if (reqSamples <= input.getAvailableSampleNum(0))
                    {
                        // Check to trigger rubberband to process when full enough.
                        auto readSpace = output.getAvailableSampleNum(0);

                        if (readSpace < smallestAcceptableSize)
                        {           
                            // Compress or stretch time when output ring buffer is too full or empty.
                            timeSmoothing.setTargetValue(1.0);
                        }
                        else if (readSpace > largestAcceptableSize) 
                        {
                            // DBG("readSpace:" << readSpace);
                            timeSmoothing.setTargetValue(1.0);
                        }
                        else 
                        {
                            timeSmoothing.setTargetValue(1.0);
                        }
                        rubberband->setTimeRatio(timeSmoothing.skip((int)reqSamples));
                        rubberband->process(input.readPointerArray((int)reqSamples), reqSamples, false);   // Process stored input samples.
                    }
                }
            }
        }

        auto availableSamples = rubberband->available();

        if (availableSamples > 0) 
        {                                         
            // If rubberband samples are available then copy to the output ring buffer.
            rubberband->retrieve(output.writePointerArray(), availableSamples);
            output.copyToBuffer(availableSamples);
        }

        auto availableOutputSamples = output.getAvailableSampleNum(0);      
        // DBG(availableOutputSamples);
        // Copy samples from output ring buffer to output buffer where available.
        for (int channel = 0; channel < buffer.getNumChannels(); channel++) 
        {
            for (int sample = 0; sample < buffer.getNumSamples(); sample++) 
            {
                if (output.getAvailableSampleNum(channel) > 0) 
                {
                    // DBG(availableOutputSamples);
                    //if (availableOutputSamples < buffer.getNumSamples())
                    //    DBG("available<numSamples!");// only begin at the beginning several
                    buffer.setSample(channel,  ((availableOutputSamples >= buffer.getNumSamples()) ?
                        sample : sample + buffer.getNumSamples() - availableOutputSamples), output.popSample(channel));
                }
            }
        }

        if (pitchParam == 0 && mixParam != 100.0) 
        {                         
            // Ensure no phasing with mix occurs when pitch is set to +/-0 semitones.
            mixSmoothing.setTargetValue(0.0);
        }
        else
        {
            mixSmoothing.setTargetValue(mixParam / 100.0);
        }
        dryWet->setWetMixProportion(mixSmoothing.skip(buffer.getNumSamples()));
        dryWet->mixWetSamples(buffer);   
        //                                    
        // Mix in the dry signal.
    }

    /** Set the wet/dry mix as a % value.
     */
    void setMixPercentage(float newPercentage)
    {
        mixParam = newPercentage;
    }

    /** Set the pitch shift in semitones.
     */
    void setSemitoneShift(float newShift)
    {
        pitchParam = newShift;
    }

    /** Get the % value of the wet/dry mix.
     */
    float getMixPercentage()
    {
        return mixParam;
    }

    /** Get the pitch shift in semitones.
     */
    float getSemitoneShift()
    {
        return pitchParam;
    }

    /** Get the estimated latency. This is an average guess of latency with no pitch shifting
    but can vary by a few buffers. Changing the pitch shift can cause less or more latency.
     */
    int getLatencyEstimationInSamples()
    {
        return maxSamples * 3.0 + initLatency;
    }
    
private:
    std::unique_ptr<RubberBand::RubberBandStretcher> rubberband;
    RingBuffer input, output;
    juce::AudioBuffer<float> inputBuffer, outputBuffer;
    int maxSamples, initLatency, bufferFail, smallestAcceptableSize, largestAcceptableSize;
    float oldPitch, pitchParam, mixParam{ 100.0f };
    std::unique_ptr<juce::dsp::DryWetMixer<float>> dryWet;
    juce::SmoothedValue<float> timeSmoothing, mixSmoothing, pitchSmoothing;
};
#endif