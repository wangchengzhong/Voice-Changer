#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <functional>
// #include <concepts>
#include"BlockCircularBuffer.h"

#include "vchsm/convert_C.h"
#include"vchsm/train_C.h"
//#include"D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/CppAlgo/include/vchsm/convert_C.h"
#include"speex/global_speex_resampler.h"


using FloatTypeForConversion = float;

class VocoderForVoiceConversion
{
public:
	enum class Windows {
		hann,
		hamming,
		kaiser
	};

public:
	VocoderForVoiceConversion(int windowLength = 18000, 
		Windows windowType = Windows::hann) :
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
		windowFunction(windowLength)


	{
		
		downResampler = speex_resampler_init(1, 48000, 16000, 3, &err);
		upResampler = speex_resampler_init(1, 16000, 48000, 3, &err);

		windowOverlaps = getOverlapsRequiredForWindowType(windowType);
		analysisHopSize = windowLength / windowOverlaps;
		synthesisHopSize = windowLength / windowOverlaps;
		// DBG("cur1: " << synthesisHopSize);

		initialiseWindow(getWindowForEnum(windowType));

		double accum = 0.0;
		auto windowFunction = getWindowFunction();
		for(int i = 0; i < getWindowSize(); ++i)
		{
			accum += windowFunction[i] * (double)windowFunction[i];
		}
		accum /= synthesisHopSize;
		setRescalingFactor((float)accum);
	}

	juce::SpinLock& getParamLock() { return paramLock; }

	int getWindowSize() const { return windowSize; }
	int getLatencyInSamples() const { return windowSize; }
	int getWindowOverlapCount() { return windowOverlaps; }

	const FloatTypeForConversion* const getWindowFunction()
	{
		return windowFunction.data();
	}

	float getRescalingFactor() const { return rescalingFactor; }

	void setRescalingFactor(float factor)
	{
		rescalingFactor = factor;
	}


	void process(FloatTypeForConversion* const audioBuffer, const int audioBufferSize)
	{
		juce::ScopedNoDenormals noDenormals;
		const juce::SpinLock::ScopedLockType lock(paramLock);

		static int callbackCount = 0;

		for (auto internalOffset = 0, internalBufferSize = 0;
			internalOffset < audioBufferSize;
			internalOffset += internalBufferSize)
		{
			const auto remainingIncomingSamples = audioBufferSize - internalOffset;
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
				convertBlock(modelFile, inputDblBuffer, outputDblBuffer, 1);


				// std::vector<double> outputDblBuffer(vcInputResampleBuffer.begin(), vcInputResampleBuffer.end());
			
				std::vector<float> outputFloatBuffer(outputDblBuffer.begin(), outputDblBuffer.end());
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
		juce::FloatVectorOperations::multiply(audioBuffer, 1.f / rescalingFactor, audioBufferSize);
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

private:
	using JuceWindow = typename juce::dsp::WindowingFunction<FloatTypeForConversion>;
	using JuceWindowTypes = typename juce::dsp::WindowingFunction<FloatTypeForConversion>::WindowingMethod;

	int getOverlapsRequiredForWindowType(Windows windowType) const
	{
		switch (windowType)
		{
		case Windows::hann:
		case Windows::hamming:
			return 4;

		case Windows::kaiser:
			return 8;

		default:
			return -1;
		}
	}

	JuceWindowTypes getWindowForEnum(Windows windowType)
	{
		switch (windowType)
		{
		case Windows::kaiser:
			return JuceWindow::kaiser;

		case Windows::hamming:
			return JuceWindow::hamming;

		case Windows::hann:
		default:
			return JuceWindow::hann;
		}
	}

	void initialiseWindow(JuceWindowTypes window)
	{
		JuceWindow::fillWindowingTables(windowFunction.data(), windowSize, window, false);
	}

protected:
	std::shared_ptr<float> fftBufferIn, fftBufferOut;
private:
	std::unique_ptr<juce::dsp::FFT> fft;

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

	juce::SpinLock paramLock;

	std::mutex flagLock;
	bool processDone{ true };

	std::vector<FloatTypeForConversion> windowFunction;

	float rescalingFactor = 1.f;
	int analysisHopSize = 0;
	int synthesisHopSize = 0;
	int windowSize = 0;
	int resampleWindowSize = 0;
	int windowOverlaps = 0;

	const char* modelFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Model.dat";

	SpeexResamplerState* upResampler;
	SpeexResamplerState* downResampler;
	int err;
	spx_uint32_t spxUpSize;
	spx_uint32_t spxDownSize;

};
