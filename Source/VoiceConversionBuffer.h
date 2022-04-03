#pragma once

#include"JuceHeader.h"
#include"VoiceConversion.h"

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

    double popSample(int channel)
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

    const double* readPointerArray(int reqSamples)
    {
        for (int samplePos = 0; samplePos < reqSamples; samplePos++)
        {
        	pointerBuffer[samplePos] = popSample(0);
        }
        return pointerBuffer.data();// pointerBuffer.getArrayOfReadPointers();
    }

    void writePointerArray(double* ptrBegin, int writeNum)
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
        for (int sample = 0; sample < numSamples; sample++)
        {
            pushSample(pointerBuffer[sample], 0);
            // pushSample(pointerBuffer.getSample(channel, sample), channel);
        }
    }

private:
    juce::AudioBuffer<double> buffer;// , pointerBuffer;
    std::vector<double> pointerBuffer;
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
        :samplesPerBlock(samplesPerBlock),sampleRate((int)sampleRate)
    {

        vc = std::make_unique<VoiceConversion>(sampleRate);
        vc->setChannels(numChannels);

        input.initialise(numChannels, sampleRate * 20);
        output.initialise(numChannels, sampleRate * 20);

    }

    ~VoiceConversionBuffer()
    {
    }
    void processBuffer(juce::AudioBuffer<float>& buffer)
    {
        auto reqSamples = 22050;//  (int)22050;
        const juce::SpinLock::ScopedLockType lock(paramLock);
        //dryWet->pushDrySamples(buffer);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // Loop to push samples to input buffer.
            int channel = 0;
        	//for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                input.pushSample(buffer.getSample(channel, sample), channel);
                buffer.setSample(channel, sample, 0.0);

                //if (channel == buffer.getNumChannels() - 1)
                {
                   
                    if (reqSamples < input.getAvailableSampleNum(0))
                    {
                        vc->putSamples(input.readPointerArray((int)reqSamples), static_cast<unsigned int>(reqSamples));
                    }
                }
            }
        }

        auto availableSamples = static_cast<int>(vc->numSamples());

        if (availableSamples > 0)
        {
            double* readSample = vc->ptrBegin();
            output.writePointerArray(readSample, availableSamples);
            vc->receiveSamples(static_cast<unsigned int>(availableSamples));
            output.copyToBuffer(availableSamples);
        }

        auto availableOutputSamples = output.getAvailableSampleNum(0);

        // Copy samples from output ring buffer to output buffer where available.
        //for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        int channel = 0;
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

private:
    int sampleRate;
    std::unique_ptr<VoiceConversion> vc;
    RingBufferForVC input, output;
    juce::AudioBuffer<double> inputBuffer, outputBuffer;
    int samplesPerBlock;
    juce::SpinLock paramLock;
};
