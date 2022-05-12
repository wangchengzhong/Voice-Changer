#pragma once


#pragma once

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <functional>
#include <concepts>

#include "BlockCircularBufferForVC.h"
#include<cmath>

#include "global_speex_resampler.h"
// 使用线性插值将信号重新采样到新的大小
// 'originalSize' 是原始信号的最大尺寸
// 'newSignalSize' 是要重新采样到的大小。 'newSignal' 必须至少一样大。

using FloatType = double;

class PhaseVocoderForVC
{

public:
	PhaseVocoderForVC(int sampleRate,int windowLength = 46000) :
		samplesTilNextProcess(windowLength),
		windowSize(windowLength),
		analysisBuffer(windowLength),
		synthesisBuffer(windowLength * 3)
	{

		analysisHopSize = windowLength - 500;
		synthesisHopSize = windowLength - 500;


		outputTransitBuffer.resize(45000);
		spxUpSize = (spx_uint32_t)45000;
		spxDownSize = (spx_uint32_t)(45000 / sampleRate * 16000);
		vcOrigBuffer.resize((int)(45000 / sampleRate * 16000));
		vcConvertedBuffer.resize((int)(45000 / sampleRate * 16000));
		downResampler = speex_resampler_init(1, sampleRate, (int)16000, 5, &err);
		upResampler = speex_resampler_init(1, (int)16000, sampleRate, 5, &err);

	}

	juce::SpinLock& getParamLock() { return paramLock; }

	int getWindowSize() const { return windowSize; }
	int getLatencyInSamples() const { return windowSize; }
	int getWindowOverlapCount() { return windowOverlaps; }


	float getTimeStretchRatio() const { return timeStretchRatio; }

	int getSynthesisHopSize() const { return synthesisHopSize; }

	void setSynthesisHopSize(int hopSize)
	{
		synthesisHopSize = hopSize;
	}

	int getAnalysisHopSize() const { return analysisHopSize; }

	void setAnalysisHopSize(int hopSize)
	{
		analysisHopSize = hopSize;
	}

	void process(float* const audioBuffer, const int audioBufferSize)
	{
		juce::ScopedNoDenormals noDenormals;
		const juce::SpinLock::ScopedLockType lock(paramLock);

		static int callbackCount = 0;

		// Only write enough samples into the analysis buffer to complete a processing
		// frame. Likewise, only write enough into the synthesis buffer to generate the 
		// next output audio frame. 
		for (auto internalOffset = 0, internalBufferSize = 0;
			internalOffset < audioBufferSize;
			internalOffset += internalBufferSize)
		{
			const auto remainingIncomingSamples = (audioBufferSize - internalOffset);
			internalBufferSize = incomingSampleCount + remainingIncomingSamples >= samplesTilNextProcess ?
				samplesTilNextProcess - incomingSampleCount : remainingIncomingSamples;

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

				// After first processing, do another process every analysisHopSize samples
				samplesTilNextProcess = analysisHopSize;

				jassert(spectralBufferSize > windowSize);
				analysisBuffer.setReadHopSize(analysisHopSize);
				analysisBuffer.read(vcBuffer.data(), windowSize);

				synthesisBuffer.setWriteHopSize(synthesisHopSize);
				//synthesisBuffer.overlapWrite(resampleBuffer.data(), resampleBufferSize);
			}

			// Emit silence until we start producing output
			if (!isProcessing)
			{
				std::fill(audioBuffer + internalOffset, audioBuffer + internalOffset +
					internalBufferSize, 0.f);
				continue;
			}

			const auto previousSynthesisReadIndex = synthesisBuffer.getReadIndex();
			synthesisBuffer.read(audioBuffer + internalOffset, internalBufferSize);
		}
	}

private:

	// Buffers
	BlockCircularBufferForVC<FloatType> analysisBuffer;
	BlockCircularBufferForVC<FloatType> synthesisBuffer;
	// Misc state
	long incomingSampleCount = 0;
	int spectralBufferSize = 0;
	int samplesTilNextProcess = 0;
	bool isProcessing = false;

	juce::SpinLock paramLock;
	std::vector<FloatType> windowFunction;
	float rescalingFactor = 1.f;
	int analysisHopSize = 0;
	int synthesisHopSize = 0;
	int windowSize = 0;
	int resampleBufferSize = 0;
	int windowOverlaps = 0;

	float pitchRatio = 0.f;
	float timeStretchRatio = 1.f;


	SpeexResamplerState* upResampler;
	SpeexResamplerState* downResampler;
	int err;
	spx_uint32_t spxUpSize;
	spx_uint32_t spxDownSize;
	std::vector<double> vcOrigBuffer;
	std::vector<double> vcConvertedBuffer;
	std::vector<double> vcBuffer;
	std::vector<double> outputTransitBuffer;
};
