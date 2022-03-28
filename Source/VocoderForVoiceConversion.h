#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <functional>
// #include <concepts>
#include"BlockCircularBuffer.h"

#include "vchsm/convert_C.h"
#include"vchsm/train_C.h"
#include"speex/global_speex_resampler.h"
#include"JuceHeader.h"
#include "ppl.h"

using FloatTypeForConversion = float;

class ProcessOneThread :public Thread
{
public:
	ProcessOneThread(int startSample, int numSample, std::vector<double>& inputDblBuffer,
		std::vector<double>& outputDblBuffer,HSMModel& model) :
		Thread("Convert Thread"),
		startSample(startSample),
		numSample(numSample),
		inputDblBuffer(inputDblBuffer),
		outputDblBuffer(outputDblBuffer),
		model(model)
	{
		startThread(Random::getSystemRandom().nextInt(3) + 6);
	}
	~ProcessOneThread()override
	{
		stopThread(10);
	}
	void run() override
	{
		// while (!threadShouldExit())
		{
			wait(interval);
			// const MessageManagerLock mml(Thread::getCurrentThread());
			//if (!mml.lockWasGained())
			//	return;
	
			convertBlock(inputDblBuffer,outputDblBuffer, 1, model);
		}
	}
private:
	int startSample;
	int numSample;
	int interval = Random::getSystemRandom().nextInt(100) + 6;
	std::vector<double>& inputDblBuffer;
	std::vector<double>& outputDblBuffer;
	HSMModel& model;
	bool counter;
	

};

class VocoderForVoiceConversion
{
public:
	enum class Windows {
		hann,
		hamming,
		kaiser
	};

public:
	VocoderForVoiceConversion(
		int sampleRate = 44100,
		int windowLength = 44100*5,
		Windows windowType = Windows::hamming) :
		sampleRate(sampleRate),
		samplesTilNextProcess(windowLength),
		windowSize(windowLength),
		resampleWindowSize((int)(windowLength/3)),
		vcBufferSize(windowLength),
		
		analysisBuffer(windowLength),
		synthesisBuffer(windowLength * 3),
		vcInputBuffer(windowLength,0),
		vcOutputBuffer(windowLength,0),
		vcInputResampleBuffer((int)(windowLength/3),0),
		vcOutputResampleBuffer((int)(windowLength/3),0),
		model(deserializeModel(modelFile))
		

	{
		// model = deserializeModel(modelFile);
		
		downResampler = speex_resampler_init(1, sampleRate, 16000, 5, &err);
		upResampler = speex_resampler_init(1, 16000, sampleRate, 5, &err);

		windowOverlaps = 1;// getOverlapsRequiredForWindowType(windowType);
		analysisHopSize = windowLength;// / windowOverlaps;
		synthesisHopSize = windowLength;// / windowOverlaps;


		// accum /= synthesisHopSize;
		// accum = 2;
		//setRescalingFactor((float));
	}

	juce::SpinLock& getConvertLock() { return convertLock; }

	int getWindowSize() const { return windowSize; }
	int getLatencyInSamples() const { return windowSize; }
	int getWindowOverlapCount() { return windowOverlaps; }


	float getRescalingFactor() const { return rescalingFactor; }

	void setRescalingFactor(float factor)
	{
		rescalingFactor = factor;
	}


	void process(FloatTypeForConversion* const audioBuffer, const int audioBufferSize)
	{
		juce::ScopedNoDenormals noDenormals;
		const juce::SpinLock::ScopedLockType lock(convertLock);

		static int callbackCount = 0;

		for (auto internalOffset = 0, internalBufferSize = 0;
			internalOffset < audioBufferSize;
			internalOffset += internalBufferSize)
		{
			const auto remainingIncomingSamples = audioBufferSize - internalOffset;//
			internalBufferSize = incomingSampleCount + remainingIncomingSamples >= samplesTilNextProcess ?
				samplesTilNextProcess - incomingSampleCount : remainingIncomingSamples;

			jassert(internalBufferSize <= audioBufferSize);

			// Write the incoming samples into the internal buffer
			// Once there are enough samples, perform spectral processing
			const auto previousAnalysisWriteIndex = analysisBuffer.getReadIndex();
			analysisBuffer.write(audioBuffer + internalOffset, internalBufferSize);

			incomingSampleCount += internalBufferSize;

			// Collected enough samples, do processing
			if (incomingSampleCount >= samplesTilNextProcess)
			{
				isProcessing = true;

				incomingSampleCount -= samplesTilNextProcess;

				samplesTilNextProcess = analysisHopSize;

				auto vcInputBufferData = vcInputBuffer.data();

				analysisBuffer.setReadHopSize(analysisHopSize);
				analysisBuffer.read(vcInputBufferData, windowSize);

				spxUpSize = (spx_uint32_t)windowSize;
				spxDownSize = (spx_uint32_t)resampleWindowSize;
				err = speex_resampler_process_float(downResampler, 0, vcInputBuffer.data(), &spxUpSize, vcInputResampleBuffer.data(), &spxDownSize);
				std::vector<double> inputDblBuffer(vcInputResampleBuffer.begin(), vcInputResampleBuffer.end());
				std::vector<double> outputDblBuffer(vcOutputResampleBuffer.begin(), vcOutputResampleBuffer.end());
				//convertBlock(inputDblBuffer, outputDblBuffer, 1,model);

				 int aa = 5;

				 int t = (int)(resampleWindowSize / aa);
			
				 for (int k = 0; k < aa; k++)
				 {
					 inputDblArray.add(new std::vector<double>(inputDblBuffer.begin() + k * t, inputDblBuffer.begin() + k * t + t - 1));
					 outputDblArray.add(new std::vector<double>(outputDblBuffer.begin() + k * t, outputDblBuffer.begin() + k * t + t - 1));
				 }

				//for(int k = 0; k < aa; k++)
				//{
				//	processThreads.add(new ProcessOneThread(k * t, t, inputDblArray.data()[k][0], outputDblArray.data()[k][0], model));
				//}

				//concurrency::parallel_for(size_t(0), (size_t)aa, [&](size_t k)
				//	{
				//		//convertBlock(inputDblArray.data()[k][0], outputDblArray.data()[k][0], 1, model);
				//		memcpy(outputDblArray.data()[k][0].data(), inputDblArray.data()[k][0].data(), sizeof(double) * inputDblArray.data()[k][0].size());
				//	}
				//);
				for(int k = 0; k < aa; k++)
				{
					memcpy(outputDblArray.data()[k][0].data(), inputDblArray.data()[k][0].data(), sizeof(double) * inputDblArray.data()[k][0].size());
				}

				for(int k = 0; k < aa; k++)
				{
					memcpy(outputDblBuffer.data() + k * t, outputDblArray.data()[k][0].data(), t * sizeof(double));
				}

				std::vector<float> outputFloatBuffer(outputDblBuffer.begin(), outputDblBuffer.end());
				//outputDblBuffer.clear();
				inputDblArray.clear();
				outputDblArray.clear();
				//processThreads.clear();
				
				err = speex_resampler_process_float(upResampler, 0, outputFloatBuffer.data(), &spxDownSize, vcOutputBuffer.data(), &spxUpSize);
				
				// memcpy(vcOutputBuffer.data(), vcInputBuffer.data(), sizeof(float) * windowSize);
				

				synthesisBuffer.setWriteHopSize(synthesisHopSize);
				synthesisBuffer.overlapWrite(vcOutputBuffer.data(), vcBufferSize);

				setProcessFlag(true);
			}

			// Emit silence until we start producing output
			if (!isProcessing)
			{
				std::fill(audioBuffer + internalOffset, audioBuffer + internalOffset +
					internalBufferSize, (double)0.f);

				// DBG("Zeroed output: " << internalOffset << " -> " << internalBufferSize);
				continue;
			}

			const auto previousSynthesisReadIndex = synthesisBuffer.getReadIndex();
			synthesisBuffer.read(audioBuffer + internalOffset, internalBufferSize);
		}


		// Rescale output
		// juce::FloatVectorOperations::multiply(audioBuffer, 1.f / rescalingFactor, audioBufferSize);
	}

	void setProcessFlag(bool flag)
	{
		std::lock_guard<std::mutex> lock(flagLock);
		processDone = flag;
	}
	bool getProcessFlag()
	{
		std::lock_guard<std::mutex> lock(flagLock);
		return processDone;
	}

public:

	// Buffers
	BlockCircularBuffer<FloatTypeForConversion> analysisBuffer;
	int vcBufferSize = 0;
	std::vector<FloatTypeForConversion> vcInputBuffer;
	std::vector<FloatTypeForConversion> vcOutputBuffer;
	std::vector<FloatTypeForConversion> vcInputResampleBuffer;
	std::vector<FloatTypeForConversion> vcOutputResampleBuffer;

	BlockCircularBuffer<FloatTypeForConversion> synthesisBuffer;

	long incomingSampleCount = 0;
	
	int samplesTilNextProcess = 0;
	bool isProcessing = false;

	juce::SpinLock convertLock;

	std::mutex flagLock;
	bool processDone{ true };

	float rescalingFactor = 1.f;
	int analysisHopSize = 0;
	int synthesisHopSize = 0;
	int windowSize = 0;
	int resampleWindowSize = 0;
	int windowOverlaps = 0;

	const char* modelFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Models/Model.dat";

	SpeexResamplerState* upResampler;
	SpeexResamplerState* downResampler;
	int err;
	spx_uint32_t spxUpSize;
	spx_uint32_t spxDownSize;
public:
	HSMModel model;
	int sampleRate;

	juce::OwnedArray<ProcessOneThread> processThreads;
	juce::OwnedArray<std::vector<double>> inputDblArray;
	juce::OwnedArray<std::vector<double>> outputDblArray;
};


