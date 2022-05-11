#pragma once

#include "f0analysisbyboersma.h"
#include "harmonicanalysis.h"
#include "HSManalyze.h"
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

    void clear()
    {
	    for(int i = 0; i < readPos.size(); i++)
	    {
            readPos[i] = 0.0;
            writePos[i] = 0.0;
	    }
    }
private:
    juce::AudioBuffer<double> buffer;
    std::vector<double> pointerBuffer;
public:
    std::vector<int> readPos, writePos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBufferForVC)
};

class VoiceConversionBuffer
{
public:
    VoiceConversionBuffer(int numChannels, double sampleRate, int samplesPerBlock, HSMModel model)
        :samplesPerBlock(samplesPerBlock),sampleRate((int)sampleRate),model(model)
    {
        outputTransitBuffer.resize(45000);
        spxUpSize = (spx_uint32_t)45000;
        spxDownSize = (spx_uint32_t)(45000 / sampleRate * 16000);
        vcOrigBuffer.resize((int)(45000 / sampleRate * 16000));
        vcConvertedBuffer.resize((int)(45000 / sampleRate * 16000));
        downResampler = speex_resampler_init(1, sampleRate, (int)16000, 3, &err);
        upResampler = speex_resampler_init(1, (int)16000, sampleRate, 3, &err);
        //vcOrigBuffer.resize(spxDownSize);
        //vcConvertedBuffer.resize(spxDownSize);
    	initializeBuffer.resize(bufferLength);
		for(auto i:initializeBuffer)
			i = (float)rand() / RAND_MAX - 0.5;

		auto temp = 1 + std::floor((bufferLength - 3.0 * 16000 / 50) / 128);
		pms = 1 + (int)std::ceil(1.5 * 16000 / 50) + 128 * seq<Eigen::RowVectorXi>(0, temp).array();
        DBG("trial size:" << pms.size());
        //f0s = f0analysisbyboersma(initializeBuffer, 16000, pms);
        //picos = harmonicanalysis(initializeBuffer, 16000, pms, f0s, 5000);
        picos = HSManalyze(initializeBuffer, 16000);
        pVcImpl = std::make_unique<VoiceConversionImpl>(model, pms, initializeBuffer, picos);
        // vc = std::make_unique<VoiceConversion>(sampleRate,this->model);
        // vc->setChannels(numChannels);

        input.initialise(numChannels, sampleRate * 20);
        output.initialise(numChannels, sampleRate * 20);

    }

    ~VoiceConversionBuffer()
    {
    }
    void processBuffer(juce::AudioBuffer<float>& buffer)
    {
        //currentEnergy = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
        ////DBG(currentEnergy);
        //for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        //{
        //    input.pushSample(buffer.getSample(0, sample), 0);
        //    buffer.setSample(0, sample, 0);
        //}
        //int numInputSample = input.getAvailableSampleNum(0);
   
    	//if(currentEnergy < energyThreshold)
     //   {
     //       silenceCount++;
     //       if(silenceCount > 100)
     //       {
     //           output.writePointerArray(silenceBuffer.data(), numInputSample);
     //           output.copyToBuffer(numInputSample);
     //       	input.readPos[0] = input.readPos[0] + numInputSample;
     //           prevEnergy = currentEnergy;
     //       }
     //       else if((currentEnergy < prevEnergy) && (numInputSample > 45000) && silenceCount > 1)
     //       {
     //           prevEnergy = currentEnergy;
     //           vc->putSamples(input.readPointerArray(numInputSample), static_cast<uint>(numInputSample));
     //           // DBG(numInputSample);
     //           //auto availableSamples = static_cast<int>(vc->numSamples());

     //           //if (availableSamples > 0)
     //           //{
     //           //    double* readSample = vc->ptrBegin();
     //           //    output.writePointerArray(readSample, availableSamples);
     //           //    uint a = vc->receiveSamples(static_cast<unsigned int>(availableSamples));
     //           //    output.copyToBuffer(availableSamples);
     //           //}
     //       }
     //   }
     //   else
     //   {
     //       silenceCount = 0;
     //       prevEnergy = currentEnergy;
     //   }
        

        //auto reqSamples = 22050;// 46300;// + underThresCount % 10000;//  (int)22050;//46305
        // // const juce::SpinLock::ScopedLockType lock(paramLock);
        //dryWet->pushDrySamples(buffer);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // Loop to push samples to input buffer.
            int channel = 0;
        	//for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            { 
                input.pushSample(buffer.getSample(channel, sample), channel);
                buffer.setSample(channel, sample, 0.0);

                {
                    if (reqSamples == input.getAvailableSampleNum(0))
                    {
                        //vc->putSamples(input.readPointerArray((int)reqSamples), static_cast<unsigned int>(reqSamples));
                        err = speex_resampler_process_float(downResampler, 0, input.readPointerArray(reqSamples), &spxUpSize, vcOrigBuffer.data(), &spxDownSize);
                        
                    	// pVcImpl->processConversion(vcOrigBuffer, vcConvertedBuffer, 1);
                        convertBlock(vcOrigBuffer, vcConvertedBuffer, 1, model);
                        err = speex_resampler_process_float(upResampler, 0, vcConvertedBuffer.data(), &spxDownSize, outputTransitBuffer.data(), &spxUpSize);
                        output.writePointerArray(outputTransitBuffer.data(), reqSamples);
                        output.copyToBuffer(reqSamples);
                    }
                }
            }
        }


        //auto availableSamples = static_cast<int>(vc->numSamples());
        //if (availableSamples > 0)
        //{
        //    double* readSample = vc->ptrBegin();
        //    output.writePointerArray(readSample, availableSamples);
        //    vc->receiveSamples(static_cast<unsigned int>(availableSamples));
        //    output.copyToBuffer(availableSamples);
        //}

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
    HSMModel model;
    std::unique_ptr<VoiceConversionImpl> pVcImpl;
    int sampleRate;
    // std::unique_ptr<VoiceConversion> vc;
    RingBufferForVC input, output;
    std::vector<double> outputTransitBuffer;
    int samplesPerBlock;
    juce::SpinLock paramLock;

    const double energyThreshold{ 0.02 };
    const int silenceCountThreshold{ 90 };
    double currentEnergy{ 0 };
    double prevEnergy{ 10000 };
    int silenceCount{ 0 };
    std::vector<double> silenceBuffer{ std::vector<double>(100000,0.0) };
	int underThresCount{ 0 };
    
	const int reqSamples{ 45000 };
    int bufferLength{ 15000 };
    Eigen::TRowVectorX initializeBuffer;
    Eigen::RowVectorXi pms;
    PicosStructArray picos;

    Eigen::TRowVectorX f0s;
    SpeexResamplerState* upResampler;
    SpeexResamplerState* downResampler;
    int err;
    spx_uint32_t spxUpSize;
    spx_uint32_t spxDownSize;
    std::vector<SAMPLETYPE> vcOrigBuffer;
    std::vector<SAMPLETYPE> vcConvertedBuffer;
    std::vector<SAMPLETYPE> vcBuffer;
};
