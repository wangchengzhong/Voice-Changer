#pragma once
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <functional>
#include <concepts>

#include "BlockCircularBufferForVC.h"
#include<cmath>

#include "global_speex_resampler.h"
#include "vchsm/convert_C.h"
#define _MM_LOAD    _mm_loadu_ps
// 使用线性插值将信号重新采样到新的大小
// 'originalSize' 是原始信号的最大尺寸
// 'newSignalSize' 是要重新采样到的大小。 'newSignal' 必须至少一样大。
#define ALIGN_POINTER_16(x)      ( ( (ulongptr)(x) + 15 ) & ~(ulongptr)15 )
using FloatTypeForVC = double;

class PhaseVocoderForVC
{

public:
	PhaseVocoderForVC(int sampleRate, HSMModel& model, int windowLength = 45000) :
		samplesTilNextProcess(windowLength),
		windowSize(windowLength),
		analysisBuffer(windowLength * 2),
		synthesisBuffer(windowLength * 3),
		pMidBufferUnaligned(new double[overlapLength + 16 / sizeof(double)]),
		model(model)
	{
		pMidBuffer = (double*)ALIGN_POINTER_16(pMidBufferUnaligned);
		memset(pMidBuffer, 0,  sizeof(double) * overlapLength);
		midEndPos = bufferLength - 1024;
		analysisHopSize = windowLength - 3000;
		synthesisHopSize = windowLength - 3000;

		outputTransitBuffer.resize(bufferLength);
		spxUpSize = (spx_uint32_t)bufferLength;
		spxDownSize = (spx_uint32_t)(bufferLength / sampleRate * 16000);
		vcBuffer.resize(bufferLength);
		vcOrigBuffer.resize((int)(bufferLength / sampleRate * 16000));
		vcConvertedBuffer.resize((int)(bufferLength / sampleRate * 16000));
		downResampler = speex_resampler_init(1, sampleRate, (int)16000, 3, &err);
		upResampler = speex_resampler_init(1, (int)16000, sampleRate, 3, &err);

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
		// const juce::SpinLock::ScopedLockType lock(paramLock);

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
				if (isBeginning)
				{
					analysisBuffer.setReadHopSize(analysisHopSize);
				}
				analysisBuffer.read(vcBuffer.data(), windowSize);
				// memcpy(vcBuffer.data(), outputTransitBuffer.data(), sizeof(double) * bufferLength);
				err = speex_resampler_process_float(downResampler, 0, vcBuffer.data(), &spxUpSize, vcOrigBuffer.data(), &spxDownSize);
				convertBlock(vcOrigBuffer, vcConvertedBuffer, 1, model);
				err = speex_resampler_process_float(upResampler, 0, vcConvertedBuffer.data(), &spxDownSize, outputTransitBuffer.data(), &spxUpSize);

				//do something here
				if (isBeginning)
				{
					//memcpy(pMidBuffer, outputTransitBuffer.data() + sizeof(double)*(midEndPos - 1), sizeof(double) * 1024);
					synthesisBuffer.setWriteHopSize(getAnalysisHopSize());
					isBeginning = false;
				}
				else
				{
					
					//bestOffset = seekBestOverlapPositionQuick(outputTransitBuffer.data());
					//DBG(bestOffset);
					// synthesisBuffer.setWriteHopSize(getAnalysisHopSize());
					//memcpy(pMidBuffer, outputTransitBuffer.data() + sizeof(double)*(midEndPos - 1), sizeof(double) * 1024);
				}

				synthesisBuffer.overlapWrite(outputTransitBuffer.data(), bufferLength);
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
	int seekBestOverlapPositionQuick(const double* refPos)
	{
#define _MIN(a, b)   (((a) < (b)) ? (a) : (b))
#define SCANSTEP    16
#define SCANWIND    8

		int bestOffs;
		int i;
		int bestOffs2;
		float bestCorr, corr;
		float bestCorr2;
		double norm;

		// note: 'float' types used in this function in case that the platform would need to use software-fp

		bestCorr =
			bestCorr2 = -FLT_MAX;
		bestOffs =
			bestOffs2 = SCANWIND;
		for (i = SCANSTEP; i < seekLength - SCANWIND - 1; i += SCANSTEP)
		{
			// Calculates correlation value for the mixing position corresponding
			// to 'i'
			corr = (float)calcCrossCorr(refPos + i, pMidBuffer, norm);
			// heuristic rule to slightly favour values close to mid of the seek range
			float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
			corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

			// Checks for the highest correlation value
			if (corr > bestCorr)
			{
				// found new best match. keep the previous best as 2nd best match
				bestCorr2 = bestCorr;
				bestOffs2 = bestOffs;
				bestCorr = corr;
				bestOffs = i;
			}
			else if (corr > bestCorr2)
			{
				// not new best, but still new 2nd best match
				bestCorr2 = corr;
				bestOffs2 = i;
			}
		}

		// Scans surroundings of the found best match with small stepping
		int end = _MIN(bestOffs + SCANWIND + 1, seekLength);
		for (i = bestOffs - SCANWIND; i < end; i++)
		{
			if (i == bestOffs) continue;    // this offset already calculated, thus skip

			// Calculates correlation value for the mixing position corresponding
			// to 'i'
			corr = (float)calcCrossCorr(refPos + i, pMidBuffer, norm);
			// heuristic rule to slightly favour values close to mid of the range
			float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
			corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

			// Checks for the highest correlation value
			if (corr > bestCorr)
			{
				bestCorr = corr;
				bestOffs = i;
			}
		}

		// Scans surroundings of the 2nd best match with small stepping
		end = _MIN(bestOffs2 + SCANWIND + 1, seekLength);
		for (i = bestOffs2 - SCANWIND; i < end; i++)
		{
			if (i == bestOffs2) continue;    // this offset already calculated, thus skip

			// Calculates correlation value for the mixing position corresponding
			// to 'i'
			corr = (float)calcCrossCorr(refPos + i, pMidBuffer, norm);
			// heuristic rule to slightly favour values close to mid of the range
			float tmp = (float)(2 * i - seekLength - 1) / (float)seekLength;
			corr = ((corr + 0.1f) * (1.0f - 0.25f * tmp * tmp));

			// Checks for the highest correlation value
			if (corr > bestCorr)
			{
				bestCorr = corr;
				bestOffs = i;
			}
		}
		return bestOffs;
	}
	double calcCrossCorr(const double* pV1, const double* pV2, double& anorm)
	{

		int i;
		const float* pVec1;
		const __m128* pVec2;
		__m128 vSum, vNorm;

		// ensure overlapLength is divisible by 8
		assert((overlapLength % 8) == 0);

		// Calculates the cross-correlation value between 'pV1' and 'pV2' vectors
		// Note: pV2 _must_ be aligned to 16-bit boundary, pV1 need not.
		pVec1 = (const float*)pV1;
		pVec2 = (const __m128*)pV2;
		vSum = vNorm = _mm_setzero_ps();

		// Unroll the loop by factor of 4 * 4 operations. Use same routine for
		// stereo & mono, for mono it just means twice the amount of unrolling.
		for (i = 0; i < overlapLength / 16; i++)
		{
			__m128 vTemp;
			// vSum += pV1[0..3] * pV2[0..3]
			vTemp = _MM_LOAD(pVec1);
			vSum = _mm_add_ps(vSum, _mm_mul_ps(vTemp, pVec2[0]));
			vNorm = _mm_add_ps(vNorm, _mm_mul_ps(vTemp, vTemp));

			// vSum += pV1[4..7] * pV2[4..7]
			vTemp = _MM_LOAD(pVec1 + 4);
			vSum = _mm_add_ps(vSum, _mm_mul_ps(vTemp, pVec2[1]));
			vNorm = _mm_add_ps(vNorm, _mm_mul_ps(vTemp, vTemp));

			// vSum += pV1[8..11] * pV2[8..11]
			vTemp = _MM_LOAD(pVec1 + 8);
			vSum = _mm_add_ps(vSum, _mm_mul_ps(vTemp, pVec2[2]));
			vNorm = _mm_add_ps(vNorm, _mm_mul_ps(vTemp, vTemp));

			// vSum += pV1[12..15] * pV2[12..15]
			vTemp = _MM_LOAD(pVec1 + 12);
			vSum = _mm_add_ps(vSum, _mm_mul_ps(vTemp, pVec2[3]));
			vNorm = _mm_add_ps(vNorm, _mm_mul_ps(vTemp, vTemp));

			pVec1 += 16;
			pVec2 += 4;
		}

		// return value = vSum[0] + vSum[1] + vSum[2] + vSum[3]
		float* pvNorm = (float*)&vNorm;
		float norm = (pvNorm[0] + pvNorm[1] + pvNorm[2] + pvNorm[3]);
		anorm = norm;

		float* pvSum = (float*)&vSum;
		return (double)(pvSum[0] + pvSum[1] + pvSum[2] + pvSum[3]) / sqrt(norm < 1e-9 ? 1.0 : norm);
	}
private:

	// Buffers
	int bufferLength{ 45000 };
	BlockCircularBufferForVC<FloatTypeForVC> analysisBuffer;
	BlockCircularBufferForVC<FloatTypeForVC> synthesisBuffer;
	// Misc state
	long incomingSampleCount = 0;

	int samplesTilNextProcess = 0;
	bool isProcessing = false;

	juce::SpinLock paramLock;
	std::vector<FloatTypeForVC> windowFunction;
	float rescalingFactor = 1.f;
	int analysisHopSize = 0;
	int synthesisHopSize = 0;
	int windowSize = 0;
	int resampleBufferSize = 0;
	int windowOverlaps = 0;

	float pitchRatio = 0.f;
	float timeStretchRatio = 1.f;

	HSMModel& model;

	SpeexResamplerState* upResampler;
	SpeexResamplerState* downResampler;
	int err;
	spx_uint32_t spxUpSize;
	spx_uint32_t spxDownSize;
	std::vector<double> vcOrigBuffer;
	std::vector<double> vcConvertedBuffer;
	std::vector<double> vcBuffer;
	std::vector<double> outputTransitBuffer;

	int overlapLength{ 1024 };
	int seekLength{ 1024 };
	double* pMidBuffer;
	double* pMidBufferUnaligned;
	bool isBeginning{ true };
	int bestOffset{ 0 };
	int midEndPos;
};
