#if USE_SOUNDTOUCH
#pragma once

#include"JuceHeader.h"
#include"SoundTouch.h"


class RingBufferForST
{
public:
    RingBufferForST() {}
    ~RingBufferForST() {}

    void initialise(int numChannels, int numSamples)
    {
        readPos.resize(numChannels);
        writePos.resize(numChannels);

        for (int i = 0; i < readPos.size(); i++)
        {
            readPos[i] = 0.0;
            writePos[i] = 0.0;
        }

        buffer.setSize(numChannels, numSamples);
        pointerBuffer.resize(numChannels * numSamples);
    }

    void pushSample(float sample, int channel)
    {
        buffer.setSample(channel, writePos[channel], sample);

        if (++writePos[channel] >= buffer.getNumSamples())
        {
            writePos[channel] = 0;
        }
    }

    float popSample(int channel)
    {
        auto sample = buffer.getSample(channel, readPos[channel]);

        if (++readPos[channel] >= buffer.getNumSamples())
        {
            readPos[channel] = 0;
        }
        return sample;
    }

    int getAvailableSampleNum(int channel)
    {
        if (readPos[channel] <= writePos[channel])
        {
            return writePos[channel] - readPos[channel];
        }
        else
        {
            return writePos[channel] + buffer.getNumSamples() - readPos[channel];
        }
    }

    const float* readPointerArray(int reqSamples)
    {
        for (int samplePos = 0; samplePos < reqSamples; samplePos++)
        {

            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                pointerBuffer[samplePos * 2 + channel] = popSample(channel);
                // pointerBuffer.setSample(channel, samplePos, popSample(channel));
            }
        }
        return pointerBuffer.data();// pointerBuffer.getArrayOfReadPointers();
    }

    void writePointerArray(float* ptrBegin, int writeNum)
    {
        for (int i = 0; i < writeNum * 2; i++)
        {
            pointerBuffer[i] = *ptrBegin;
            ptrBegin++;
        }
    }

        //return pointerBuffer.data();
    
    
    void copyToBuffer(int numSamples)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < numSamples; sample++)
            {
                pushSample(pointerBuffer[sample * 2 + channel], channel);
                // pushSample(pointerBuffer.getSample(channel, sample), channel);
            }
        }
    }

private:
    juce::AudioBuffer<float> buffer;// , pointerBuffer;
    std::vector<float> pointerBuffer;
    std::vector<int> readPos, writePos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferForST)
};

class PitchShifterSoundTouch
{
public:
    /**
    Setup the pitch shifter. By default the shifter will be setup so that
    the dry signal isn't delayed to be given a somewhat similar latency to the wet signal -
    this is not accurate when enabled! By enabling minLatency some latency can be reduced with the
    expense of potential tearing during modulation with a change of the pitch parameter.
     */
    PitchShifterSoundTouch(int numChannels, double sampleRate, int samplesPerBlock, bool dryCompensationDelay = false, bool minLatency = false)
        :samplesPerBlock(samplesPerBlock)
    {

        st = std::make_unique<soundtouch::SoundTouch>();
        st->setSampleRate(sampleRate);
        st->setChannels(numChannels);
        // st->setSetting(SOUNDTOUCH_ALLOW_MMX, 1);
        st->setSetting(SETTING_USE_AA_FILTER, 1);
        st->setSetting(SETTING_AA_FILTER_LENGTH, 64);
        st->setSetting(SETTING_USE_QUICKSEEK, 0);
        st->setSetting(SETTING_SEQUENCE_MS, 60);
        st->setSetting(SETTING_SEEKWINDOW_MS, 25);
        //soundtouch::SAMPLETYPE
        //rubberband->setMaxProcessSize(samplesPerBlock);
        // initLatency = (int)rubberband->getLatency();
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

    ~PitchShifterSoundTouch()
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
            st->setPitch(newPitch);
            // st->setPitch(newPitch);
            //oldPitch = newPitch;
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
                    auto reqSamples = samplesPerBlock;// rubberband->getSamplesRequired();
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
                        // st->setPitch(timeSmoothing.skip((int)reqSamples));
                        st->putSamples(input.readPointerArray((int)reqSamples),static_cast<unsigned int>(reqSamples));
                        // rubberband->process(input.readPointerArray((int)reqSamples), reqSamples, false);   // Process stored input samples.
                    }
                }
            }
        }

        auto availableSamples = static_cast<int>(st->numSamples());
        //DBG(availableSamples);
        if (availableSamples > 0)
        {
            // If rubberband samples are available then copy to the output ring buffer.
            // rubberband->retrieve(output.writePointerArray(), availableSamples);
            float* readSample = st->ptrBegin();
            output.writePointerArray(readSample, availableSamples);
            st->receiveSamples(static_cast<unsigned int>(availableSamples));
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
                    buffer.setSample(channel, ((availableOutputSamples >= buffer.getNumSamples()) ?
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
    // std::unique_ptr<RubberBand::RubberBandStretcher> rubberband;
    std::unique_ptr<soundtouch::SoundTouch> st;
    RingBufferForST input, output;
    juce::AudioBuffer<float> inputBuffer, outputBuffer;
    int maxSamples, initLatency, bufferFail, smallestAcceptableSize, largestAcceptableSize, samplesPerBlock;
    float oldPitch, pitchParam, mixParam{ 100.0f };
    std::unique_ptr<juce::dsp::DryWetMixer<float>> dryWet;
    juce::SmoothedValue<double> timeSmoothing, mixSmoothing, pitchSmoothing;
};





/*
struct Stretcher
{
	virtual ~Stretcher() {}
    virtual bool isOk() const = 0;
	virtual void reset() = 0;
	virtual bool setSpeedAndPitch(float speedRatio, float semitonesUp) = 0;
	virtual int getFramesNeeded()const = 0;
	virtual int getMaxFramesNeeded()const = 0;
	virtual int processData(const float* const* inChannels, int numSamples, float* const* outChannel) = 0;
	virtual int flush(float* const* outChannels) = 0;

};

struct RubberBandStretcher : public Stretcher
{
    static int getOptionFlags(bool percussive)
    {
        if (percussive)
            return RubberBand::RubberBandStretcher::OptionProcessRealTime
            | RubberBand::RubberBandStretcher::OptionPitchHighConsistency
            | RubberBand::RubberBandStretcher::PercussiveOptions;

        return RubberBand::RubberBandStretcher::OptionProcessRealTime
            | RubberBand::RubberBandStretcher::OptionPitchHighConsistency
            | RubberBand::RubberBandStretcher::OptionWindowShort;
    }

    RubberBandStretcher(double sourceSampleRate, int samplesPerBlock, int numChannels, bool percussive)
        : rubberBandStretcher((size_t)sourceSampleRate, (size_t)numChannels,
            getOptionFlags(percussive)),
        samplesPerOutputBuffer(samplesPerBlock)
    {
    }

    bool isOk() const override
    {
        return true;
    }

    void reset() override
    {
        rubberBandStretcher.reset();
    }

    bool setSpeedAndPitch(float speedRatio, float semitonesUp) override
    {
        const float pitch = juce::jlimit(0.25f, 4.0f, powf(2.0, semitonesUp / 12));

        rubberBandStretcher.setPitchScale(pitch);
        rubberBandStretcher.setTimeRatio(speedRatio);

        if (numSamplesToDrop == -1)
        {
            // This is the first speed and pitch change so set up the padding and dropping
            numSamplesToDrop = int(rubberBandStretcher.getLatency());
            int numSamplesToPad = juce::roundToInt(numSamplesToDrop * pitch);

            if (numSamplesToPad > 0)
            {
                AudioScratchBuffer scratch(int(rubberBandStretcher.getChannelCount()), samplesPerOutputBuffer);
                scratch.buffer.clear();

                while (numSamplesToPad > 0)
                {
                    const int numThisTime = std::min(numSamplesToPad, samplesPerOutputBuffer);
                    rubberBandStretcher.process(scratch.buffer.getArrayOfReadPointers(), numThisTime, false);
                    numSamplesToPad -= numThisTime;
                }
            }

            jassert(numSamplesToPad == 0);
        }

        const bool r = rubberBandStretcher.getPitchScale() == pitch
            && rubberBandStretcher.getTimeRatio() == speedRatio;
        jassert(r);
        juce::ignoreUnused(r);

        return r;
    }

    int getFramesNeeded() const override
    {
        return (int)rubberBandStretcher.getSamplesRequired();
    }

    int getMaxFramesNeeded() const override
    {
        return maxFramesNeeded;
    }

    int processData(const float* const* inChannels, int numSamples, float* const* outChannels) override
    {
        jassert(numSamples <= getFramesNeeded());
        rubberBandStretcher.process(inChannels, (size_t)numSamples, false);

        // Once there is output, read this in to the output buffer
        int numAvailable = rubberBandStretcher.available();
        jassert(numAvailable >= 0);

        if (numSamplesToDrop > 0)
        {
            const int numToDropThisTime = juce::jmin(numSamplesToDrop, numAvailable, samplesPerOutputBuffer);
            rubberBandStretcher.retrieve(outChannels, (size_t)numToDropThisTime);
            numSamplesToDrop -= numToDropThisTime;
            jassert(numSamplesToDrop >= 0);

            numAvailable -= numToDropThisTime;
        }

        if (numAvailable > 0)
            return (int)rubberBandStretcher.retrieve(outChannels, (size_t)numAvailable);

        return 0;
    }

    int flush(float* const* outChannels) override
    {
        // Empty the FIFO in to the stretcher and mark the last block as final
        if (!hasDoneFinalBlock)
        {
            float* inChannels[32] = { nullptr };
            rubberBandStretcher.process(inChannels, 0, true);
            hasDoneFinalBlock = true;
        }

        // Then get the rest of the data out of the stretcher
        const int numAvailable = rubberBandStretcher.available();
        const int numThisBlock = std::min(numAvailable, samplesPerOutputBuffer);

        if (numThisBlock > 0)
            return (int)rubberBandStretcher.retrieve(outChannels, (size_t)numThisBlock);

        return 0;
    }

private:
    RubberBand::RubberBandStretcher rubberBandStretcher;
    const int maxFramesNeeded = 8192;
    const int samplesPerOutputBuffer = 0;
    int numSamplesToDrop = -1;
    bool hasDoneFinalBlock = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RubberBandStretcher)
};


struct SoundTouchStretcher : public Stretcher,
    private soundtouch::SoundTouch
{
    SoundTouchStretcher(double sourceSampleRate, int samplesPerBlock,
        int chans, bool betterQuality)
        : numChannels(chans), samplesPerOutputBuffer(samplesPerBlock)
    {
 
        setChannels(static_cast<unsigned int> (numChannels));
        setSampleRate(static_cast<unsigned int> (sourceSampleRate));

        if (betterQuality)
        {
            setSetting(SETTING_USE_AA_FILTER, 1);
            setSetting(SETTING_AA_FILTER_LENGTH, 64);
            setSetting(SETTING_USE_QUICKSEEK, 0);
            setSetting(SETTING_SEQUENCE_MS, 60);
            setSetting(SETTING_SEEKWINDOW_MS, 25);
        }
    }

    bool isOk() const override { return true; }
    void reset() override { clear(); }

    bool setSpeedAndPitch(float speedRatio, float semitonesUp) override
    {
        setTempo(1.0f / speedRatio);
        setPitchSemiTones(semitonesUp);
        inputOutputSampleRatio = getInputOutputSampleRatio();

        return true;
    }

    int getFramesNeeded() const override
    {
        const int numAvailable = (int)numSamples();
        const int numRequiredForOneBlock = juce::roundToInt(samplesPerOutputBuffer * inputOutputSampleRatio);

        return std::max(0, numRequiredForOneBlock - numAvailable);
    }

    int getMaxFramesNeeded() const override
    {
        // This was derived by experimentation
        return 8192;
    }

    int processData(const float* const* inChannels, int numSamples, float* const* outChannels) override
    {

        jassert(numSamples <= getFramesNeeded());
        writeInput(inChannels, numSamples);

        const int numAvailable = (int)soundtouch::SoundTouch::numSamples();
        jassert(numAvailable >= 0);

        const int numToRead = std::min(numAvailable, samplesPerOutputBuffer);

        if (numToRead > 0)
            return readOutput(outChannels, 0, numToRead);

        return 0;
    }

    int flush(float* const* outChannels) override
    {

            if (!hasDoneFinalBlock)
            {
                soundtouch::SoundTouch::flush();
                hasDoneFinalBlock = true;
            }

        const int numAvailable = (int)numSamples();
        const int numToRead = std::min(numAvailable, samplesPerOutputBuffer);

        return readOutput(outChannels, 0, numToRead);
    }

private:
    int numChannels = 0, samplesPerOutputBuffer = 0;
    bool hasDoneFinalBlock = false;
    double inputOutputSampleRatio = 1.0;

    int readOutput(float* const* outChannels, int offset, int numNeeded)
    {
        float* interleaved = ptrBegin();
        const int num = juce::jmin(numNeeded, (int)numSamples());

        for (int chan = 0; chan < numChannels; ++chan)
        {
            const float* src = interleaved + chan;
            float* dest = outChannels[chan] + offset;

            for (int i = 0; i < num; ++i)
            {
                dest[i] = *src;
                src += numChannels;
            }
        }

        receiveSamples(static_cast<unsigned int> (num));
        return num;
    }

    void writeInput(const float* const* inChannels, int numSamples)
    {
        if (numChannels > 1)
        {
            AudioScratchBuffer scratch(1, numSamples * numChannels);
            float* interleaved = scratch.buffer.getWritePointer(0);

            for (int chan = 0; chan < numChannels; ++chan)
            {
                float* dest = interleaved + chan;
                const float* src = inChannels[chan];

                for (int i = 0; i < numSamples; ++i)
                {
                    *dest = src[i];
                    dest += numChannels;
                }
            }

            putSamples(interleaved, static_cast<unsigned int> (numSamples));
        }
        else
        {
            putSamples(inChannels[0], static_cast<unsigned int> (numSamples));
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundTouchStretcher)
};










*/
#endif