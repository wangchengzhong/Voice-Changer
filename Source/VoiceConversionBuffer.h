#pragma once

#include"JuceHeader.h"
#include"VoiceConversion.h"
#include"RingBuffer.h"

class RingBufferForVC
{
public:
    RingBufferForVC() {}
    ~RingBufferForVC() {}

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
                pointerBuffer[samplePos] = popSample(channel);
                // pointerBuffer.setSample(channel, samplePos, popSample(channel));
            }
        }
        return pointerBuffer.data();// pointerBuffer.getArrayOfReadPointers();
    }

    void writePointerArray(float* ptrBegin, int writeNum)
    {
        for (int i = 0; i < writeNum; i++)
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
                pushSample(pointerBuffer[sample], channel);
                // pushSample(pointerBuffer.getSample(channel, sample), channel);
            }
        }
    }

private:
    juce::AudioBuffer<float> buffer;// , pointerBuffer;
    std::vector<float> pointerBuffer;
    std::vector<int> readPos, writePos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferForVC)
};

class VoiceConversionBuffer
{
public:
    /**
    Setup the pitch shifter. By default the shifter will be setup so that
    the dry signal isn't delayed to be given a somewhat similar latency to the wet signal -
    this is not accurate when enabled! By enabling minLatency some latency can be reduced with the
    expense of potential tearing during modulation with a change of the pitch parameter.
     */
    VoiceConversionBuffer(int numChannels, double sampleRate, int samplesPerBlock, bool dryCompensationDelay = false, bool minLatency = false)
        :samplesPerBlock(samplesPerBlock)
    {

        vc = std::make_unique<VoiceConversion>();
        vc->setSampleRate(sampleRate);
        vc->setChannels(numChannels);
        // vc->setSetting(SOUNDTOUCH_ALLOW_MMX, 1);

        // vc->setSetting(SETTING_USE_QUICKSEEK, 0);
        vc->setSettings(SETTING_SEQUENCE_MS, 60);
        vc->setSettings(SETTING_SEEKWINDOW_MS, 25);
        vc->setSettings(SETTING_OVERLAP_MS, 8);

        maxSamples = 256;

        input.initialise(numChannels, sampleRate * 20);
        output.initialise(numChannels, sampleRate * 20);

        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = numChannels;
        spec.sampleRate = sampleRate;

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

    ~VoiceConversionBuffer()
    {
    }
    void processBuffer(juce::AudioBuffer<float>& buffer)
    {
        //dryWet->pushDrySamples(buffer);


        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // Loop to push samples to input buffer.
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                input.pushSample(buffer.getSample(channel, sample), channel);
                buffer.setSample(channel, sample, 0.0);

                if (channel == buffer.getNumChannels() - 1)
                {
                    auto reqSamples = 44100 * 5;
                    
                    if (reqSamples <= input.getAvailableSampleNum(0))
                    {
                        vc->putSamples(input.readPointerArray((int)reqSamples), static_cast<unsigned int>(reqSamples));
                    }
                }
            }
        }

        auto availableSamples = static_cast<int>(vc->numSamples());

        if (availableSamples > 0)
        {
            float* readSample = vc->ptrBegin();
            output.writePointerArray(readSample, availableSamples);
            vc->receiveSamples(static_cast<unsigned int>(availableSamples));
            output.copyToBuffer(availableSamples);
        }

        auto availableOutputSamples = output.getAvailableSampleNum(0);

        // Copy samples from output ring buffer to output buffer where available.
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                if (output.getAvailableSampleNum(channel) > 0)
                {
                    buffer.setSample(channel, ((availableOutputSamples >= buffer.getNumSamples()) ?
                        sample : sample + buffer.getNumSamples() - availableOutputSamples), output.popSample(channel));
                }
            }
        }


    }

    /** Set the wet/dry mix as a % value.


    /** Get the estimated latency. This is an average guess of latency with no pitch shifting
    but can vary by a few buffers. Changing the pitch shift can cause less or more latency.
     */
    int getLatencyEstimationInSamples()
    {
        return maxSamples * 3.0 + initLatency;
    }

private:
    std::unique_ptr<VoiceConversion> vc;
    RingBufferForVC input, output;
    juce::AudioBuffer<float> inputBuffer, outputBuffer;
    int maxSamples, initLatency, bufferFail, smallestAcceptableSize, largestAcceptableSize, samplesPerBlock;
};
