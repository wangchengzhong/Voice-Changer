#pragma once
#include "JuceHeader.h"
#include <cmath>
#include "PhaseVocoder.h"
#include <array>
#include <sstream>
#include <iostream>

#include"ManVoiceExtract.h"
// using namespace concurrency;
using namespace std;
typedef juce::MathConstants<float> constant;
#define DEBUG_MSG_MODE false
class ShapeInvariantPitchShifter
{
public:
	ShapeInvariantPitchShifter()
		: synthPhaseIncrements(phaseVocoder.getWindowSize(), 0)
		, previousFramePhases(phaseVocoder.getWindowSize(), 0)

		, crossCorrelation(phaseVocoder.getWindowSize() * 2, 0)
		, analysisWindowCorrelation(phaseVocoder.getWindowSize() * 2, 0)
		, previousFrameSpectrum(phaseVocoder.getWindowSize() * 2, 0)
		, angleMultiplier(phaseVocoder.getWindowSize() * 2, 0)
		, spectrumAmp2(phaseVocoder.getWindowSize(), 0)
		, spectrum2Amp(phaseVocoder.getWindowSize(), 0)

		, peakIndex(phaseVocoder.getWindowSize() / 2, 0)
		, sinComponentIndexes(phaseVocoder.getWindowSize() / 2, 0)
		, previousSinComponentIndexes(phaseVocoder.getWindowSize() / 2, 0)
		, commonSinComponentIndexes(phaseVocoder.getWindowSize() / 2, 0)

	{
		setPitchRatio(1.f);
		initializeWindowCorrelation(analysisWindowCorrelation);
		initializeAngleMultiplier(angleMultiplier);
	}
	int getLatencyInSamples() const 
	{
		return phaseVocoder.getLatencyInSamples();
	}
	void setPitchRatio(float newPitchRatio)
	{
		if (phaseVocoder.getPitchRatio() == newPitchRatio)
		{
			return;
		}
		const juce::SpinLock::ScopedLockType lock(phaseVocoder.getParamLock());
		phaseVocoder.setPitchRatio(newPitchRatio);
		phaseVocoder.setSynthesisHopSize((int)phaseVocoder.getWindowSize() / (float)phaseVocoder.getWindowOverlapCount());
		phaseVocoder.setAnalysisHopSize((int)round(phaseVocoder.getSynthesisHopSize() / phaseVocoder.getPitchRatio()));
		double accum = 0.0;
		auto windowFunction = phaseVocoder.getWindowFunction();
		for (int i = 0; i < phaseVocoder.getWindowSize(); ++i)
		{
			accum += windowFunction[i] * (double)windowFunction[i];
		}
		accum /= phaseVocoder.getSynthesisHopSize();
		phaseVocoder.setRescalingFactor((float)accum);
		phaseVocoder.updateResampleBufferSize();
		phaseVocoder.setSynthesisHopSize(phaseVocoder.getAnalysisHopSize());

		memset(previousFramePhases.data(), 0, sizeof(FloatType) * phaseVocoder.getWindowSize());
		memset(synthPhaseIncrements.data(), 0, sizeof(FloatType) * phaseVocoder.getWindowSize());
		// memset(crossCorrelation.data(), 0, sizeof(FloatType) * phaseVocoder.getWindowSize() * 2);

	}
	void process(FloatType* const buffer, const int bufferSize)
	{
		phaseVocoder.process(buffer, bufferSize, [&](FloatType* const buffer, const int bufferSize)
		{
			
			for (int index = 0, k = 0; index < bufferSize; index += 2, k++)
			{
				const auto real = buffer[index];
				const auto imag = buffer[index + 1];
				const float a = real * real;
				const float b = imag * imag;
				const float c = 2 * real * imag;
				/*if (abs(real)<1e-10 && abs(imag) < 1e-10)
				{
					setProcessFlag(true);
					return;
				}*/
				spectrumAmp2[k] = a + b;
				spectrum2Amp[k] = sqrtf((a - b) * (a - b) + c * c);
				// DBG("spectrumAmpSquare[" << k << "]:" << spectrumAmp2[k]);
			}


			for (int k = 5; k < N - 5; k++)
			{
				if ((spectrumAmp2[k] > spectrumAmp2[k - 1]) && (spectrumAmp2[k] > spectrumAmp2[k + 1]) && (spectrumAmp2[k] > spectrumAmp2[k - 2]) && (spectrumAmp2[k] > spectrumAmp2[k + 2]) &&
					(spectrumAmp2[k] > spectrumAmp2[k - 3]) && (spectrumAmp2[k] > spectrumAmp2[k + 3]) && (spectrumAmp2[k] > spectrumAmp2[k - 4]) && (spectrumAmp2[k] > spectrumAmp2[k + 4]))
				{
					peakIndex[peakNum] = k;
					peakNum++;
					// peakIndex.push_back(k);
				} 
			}
		
			// DBG("\nselecting peak indexes. Peak Index size: " << peakNum);
			// if(DEBUG_MSG_MODE) DBG("spectrumAmp2.size: " << spectrumAmp2.size() << "  ,spectrum2Amp.size: " << spectrum2Amp.size());
			if (peakNum > 2)
			{
				int peakSize = peakNum;// peakIndex.size();
				for (int k = 1; k < peakSize - 2; k++)
				{
					const int currIndex = peakIndex[k];
					// const auto sum_KMinusOmega_2
					int inflectionIndexLeft = 1, inflectionIndexRight = 1;
					while (((currIndex + inflectionIndexRight + 1) < peakIndex[k + 1]) &&
						(spectrumAmp2[currIndex] > spectrumAmp2[currIndex + inflectionIndexRight]))
					{
						inflectionIndexRight++;
					}
					// DBG("handling peakIndex: " << currIndex);
					while(((currIndex - inflectionIndexLeft - 1) > 0) &&
						((currIndex - inflectionIndexLeft - 1) > peakIndex[k - 1]) &&
						(spectrumAmp2[currIndex] > spectrumAmp2[currIndex - inflectionIndexLeft]))
					{
						inflectionIndexLeft++;
					}
					// DBG("\nhandling nb: " << k << " left size:" << inflectionIndexRight << " right size: " << inflectionIndexLeft);

					omegaAverage = calculateOmegaAverage(currIndex - inflectionIndexLeft, currIndex + inflectionIndexRight, spectrumAmp2.data());
					// DBG("calculate:" << calculateNBD(currIndex - inflectionIndexLeft, currIndex + inflectionIndexRight, omegaAverage, spectrum2Amp.data()) << " omegaAverage: " << omegaAverage);
					if (calculateNBD(currIndex - inflectionIndexLeft, currIndex + inflectionIndexRight, omegaAverage, spectrum2Amp.data()) < 0.38)
					{
						sinComponentIndexes[sinComponentNum] = currIndex;// .push_back(currIndex);
						sinComponentNum++;
						// if(DEBUG_MSG_MODE) DBG("have run after calculate NBD. add Index:" << currIndex);
					}
				}
			}
			peakNum = 0;
			// peakIndex.clear();
			// DBG("sinCompoentIndexSize: " << sinComponentNum);

			int tmpJ = 0;
			if (sinComponentNum < 2)
			{
				// sinComponentIndexes.clear();
				// peakIndex.clear();
				// commonSinComponentIndexes.clear();
				sinComponentNum = 0;
				previousSinComponentNum = 0;
				commonSinComponentNum = 0;
				return;
			}

			if (previousSinComponentNum < 1)
			{
				if(DEBUG_MSG_MODE) DBG("===============================NOT GET PROCESS FLAG===================");
				// previousSinComponentIndexes.clear();
				// for (int i = 0; i < sinComponentIndexes.size(); i++)
				// {
					// previousSinComponentIndexes.push_back(sinComponentIndexes[i]);
				// }
				previousSinComponentNum = sinComponentNum;
				memcpy(previousSinComponentIndexes.data(), sinComponentIndexes.data(), sizeof(FloatType) * sinComponentNum);
			}
			// DBG("=====================================GET PROCESS FLAG=================================");
			for (int i = 0; i < sinComponentNum - 1; i++)
			{
				while ((tmpJ < previousSinComponentNum - 2) &&
					(sinComponentIndexes[i] > previousSinComponentIndexes[tmpJ + 1]))

				{
					tmpJ++;
				}
				if ((abs(sinComponentIndexes[i] - previousSinComponentIndexes[tmpJ]) < 3.1) || 
					(abs(sinComponentIndexes[i] - previousSinComponentIndexes[tmpJ + 1]) < 3.1))
				{

					commonSinComponentIndexes[commonSinComponentNum] = sinComponentIndexes[i];// .push_back(i);
					commonSinComponentNum++;
					// DBG("commonSinComponentIndexes: " << commonSinComponentNum << ", at  " << sinComponentIndexes[i]);
				}
				// tmpJ = 0;
			}
			tmpJ = 0;

			//for (int n = 0; n < bufferSize - 1; n += 2)
			// int commonSize = commonSinComponentNum; // commonSinComponentIndexes.size();
			if (commonSinComponentNum > 3)
			{
				for (int n = 0, oIndex = 0; n < bufferSize - 2; n += 2, oIndex += 1)
				{
					for (int common = 0; common < commonSinComponentNum; common++)
					{
						int k = 2 * commonSinComponentIndexes[common];
						int index = commonSinComponentIndexes[common];

						a = buffer[k];
						b = -buffer[k + 1];
						c = previousFrameSpectrum[k];
						d = previousFrameSpectrum[k + 1];
						const float acbd = a * c - b * d;
						const float bcad = b * c + a * d;
						// DBG("acbd: " << acbd);
						const int angleMultiplierIndex = 2 * ((oIndex * index) % N);
						e = angleMultiplier[angleMultiplierIndex];
						f = angleMultiplier[angleMultiplierIndex + 1];
						corrRealPart = acbd * e - bcad * f;
						corrImagPart = acbd * f + bcad * e;

						sumCorrRealPart += corrRealPart;
						sumCorrImagPart += corrImagPart;
					}
					crossCorrelation[n] = sumCorrRealPart;

					crossCorrelation[n + 1] = sumCorrImagPart;
					sumCorrImagPart = 0;
					sumCorrRealPart = 0;
					// DBG("crossCorrelation[n] " << crossCorrelation[n]);
					// DBG("crossCorrelation[n+1]: " << crossCorrelation[n + 1]);
				}
				// DBG("commonSinComponentNum: " << commonSinComponentNum);
				commonSinComponentNum = 0;

				// sumCorrRealPart = 0;
				// commonSinComponentIndexes.clear();

				for (int n = 0, index = 0; n < bufferSize - 1; n += 2, index += 1)
				{
					crossCorrelation[n] /= pow(analysisWindowCorrelation[index], 2);
					crossCorrelation[n + 1] /= pow(analysisWindowCorrelation[index], 2);
					// DBG("crossCorrelation: " << crossCorrelation[n]);
				}
				int offset = phaseVocoder.getWindowSize();//2 * phaseVocoder.getSynthesisHopSize();
				float partialMax = 0, bias = 0;
				float olpLeft = 0, olpRight = 0;
				for (int i = 0,index = 0; i < 256; index += 2, i++)
				{
					olpLeft = sqrtf(crossCorrelation[offset - index] * crossCorrelation[offset - index] + crossCorrelation[offset - index + 1] * crossCorrelation[offset - index + 1]) * pow(analysisWindowCorrelation[1023 + i], 2);
					olpRight = sqrtf(crossCorrelation[offset + index] * crossCorrelation[offset + index] + crossCorrelation[offset + index + 1] * crossCorrelation[offset + index + 1]) * pow(analysisWindowCorrelation[1023 + i], 2);
					if (olpLeft > partialMax)
					{
						partialMax = olpLeft;
						bias = -i;
					}
					if (olpRight > partialMax)
					{
						partialMax = olpRight;
						bias = i;
					}
				}
				deltaN = (offset + bias) / offset;
				// DBG("deltaN: " << deltaN);
			}
			else
			{
				commonSinComponentNum = 0;
				deltaN = 1;
			}

			for (int i = 0, x = 0; i < bufferSize - 1; i += 2, ++x)
			{
				const auto real = buffer[i];
				const auto imag = buffer[i + 1];                          
				const auto mag = sqrtf(real * real + imag * imag);
				const auto phase = atan2(imag, real);
				const auto omega = constant::twoPi * phaseVocoder.getAnalysisHopSize() * x / (float)phaseVocoder.getWindowSize();
				
				const auto deltaPhase = omega + PhaseVocoder::principalArgument(phase - previousFramePhases[x] - omega);
				previousFramePhases[x] = phase;
				synthPhaseIncrements[x] = PhaseVocoder::principalArgument(synthPhaseIncrements[x] + (deltaN * deltaPhase * phaseVocoder.getTimeStretchRatio()));

				buffer[i] = mag * std::cos(synthPhaseIncrements[x]);
				buffer[i + 1] = mag * std::sin(synthPhaseIncrements[x]);
			}

			// const int sinSize = sinComponentNum;//sinComponentIndexes.size();
			// previousSinComponentIndexes.resize(sinSize);
			memcpy(previousFrameSpectrum.data(), buffer, sizeof(FloatType) * bufferSize);
			memcpy(previousSinComponentIndexes.data(), sinComponentIndexes.data(), sizeof(FloatType) * sinComponentNum);
			previousSinComponentNum = sinComponentNum;
			sinComponentNum = 0;
			deltaN = 0;
			// sinComponentIndexes.clear();
			// DBG("sinComponentIndexAfterClear: " << sinComponentIndexes.size());
			setProcessFlag(true);
		});
	}
	void initializeWindowCorrelation(std::vector<FloatType>& analysisWindowCorrelation)
	{
		TemplateParameters t;
		const float* window = phaseVocoder.getWindowFunction();
		const int windowSize = phaseVocoder.getWindowSize();
		for (int i = 0; i < windowSize; i++)
		{
			int numToMultiply = i + 1;
			for (int j = 1; j < i + 1; j++)
			{
				analysisWindowCorrelation[j - 1] += window[j - 1] * window[(windowSize - j - 1) / 2];
			}
		}
		std::vector<float> expandedCorr(windowSize * 4, 0);
		// auto hopSize = phaseVocoder.getAnalysisHopSize();
		// analysisWindowCorrelation.resize(windowSize * 4);
		memcpy(expandedCorr.data() + sizeof(float) * windowSize / 2, window, sizeof(float) * windowSize);

		fft->performRealOnlyForwardTransform(expandedCorr.data());
		for (int i = 0, index = 0; i < expandedCorr.size() - 2; i += 2, index++)
		{
			float orgReal = expandedCorr[i];
			expandedCorr[i] = expandedCorr[i] * expandedCorr[i] - expandedCorr[i + 1] * expandedCorr[i + 1];
			expandedCorr[i + 1] = 2 * orgReal * expandedCorr[i + 1];
		}
		fft->performRealOnlyInverseTransform(expandedCorr.data());
		memcpy(analysisWindowCorrelation.data(), t.autoCorr.data(), sizeof(float) * t.autoCorr.size());
		
		for (int i = 0; i < windowSize; i++)
		{
			analysisWindowCorrelation[i] = std::max(window[i], window[300]);
		}

		
	}
	void initializeAngleMultiplier(std::vector<FloatType>& angleMultiplier)
	{
		const int N = phaseVocoder.getWindowSize();
		for (int i = 0, index = 0; i < 2 * N - 1; i += 2, index++)
		{
			angleMultiplier[i] = std::cos(2 * constant::pi * index / N);
			angleMultiplier[i + 1] = std::sin(2 * constant::pi * index / N);
			// DBG("angleMultiplier: " << angleMultiplier[i] << "     " << angleMultiplier[i + 1]);
		}
	}
	bool getProcessFlag()
	{
		return phaseVocoder.getProcessFlag();
	}
	void setProcessFlag(bool flag)
	{
		phaseVocoder.setProcessFlag(flag);
	}

	float calculateOmegaAverage(int kMin, int kMax, FloatType* pSpectrum)
	{
		float sumK_Xk_2 = 0;
		float sum_Xk_2 = 0;
		for (int k = kMin; k < kMax; k++)
		{
			sumK_Xk_2 += pSpectrum[k] * k;// *pSpectrum[k] * k;
			sum_Xk_2 += pSpectrum[k];
		}
		return sumK_Xk_2 / sum_Xk_2;
	}

	float calculateNBD(int kMin, int kMax, float omegaAverage, FloatType* pSpectrum)
	{
		float sum_KMinusOmega_2_Xk2_ = 0;
		float sum_Xk2_ = 0;
		if ((kMax - kMin) < 2)
		{
			return 2.0f;
		}
		for (int k = kMin; k < kMax; k++)
		{
			sum_KMinusOmega_2_Xk2_ += (k - omegaAverage) * (k - omegaAverage) * pSpectrum[k];
			sum_Xk2_ += pSpectrum[k];
		}
		return sum_KMinusOmega_2_Xk2_ / (kMax - kMin) / sum_Xk2_;
	}
private:
	PhaseVocoder phaseVocoder;
	std::vector<FloatType> synthPhaseIncrements;
	std::vector<FloatType> previousFramePhases;
	
	std::vector<FloatType> previousFrameSpectrum;
	std::vector<FloatType> crossCorrelation;
	std::vector<FloatType> analysisWindowCorrelation;
	std::vector<FloatType> angleMultiplier;

	std::vector<int> sinComponentIndexes;
	std::vector<int> previousSinComponentIndexes;
	std::vector<int> commonSinComponentIndexes;
	std::vector<int> peakIndex;

	int peakNum{ 0 };
	int sinComponentNum{ 0 };
	int commonSinComponentNum{ 0 };
	int previousSinComponentNum{ 0 };

	std::vector<FloatType> spectrumAmp2;
	std::vector<FloatType> spectrum2Amp;

	std::unique_ptr<juce::dsp::FFT> fft{ std::make_unique<juce::dsp::FFT>((int)log2(juce::nextPowerOfTwo(4096))) };

	int N{ phaseVocoder.getWindowSize() };

	float corrRealPart{ 0 }, corrImagPart{ 0 };
	float a, b, c, d, e, f;
	float sumCorrRealPart{ 0 }, sumCorrImagPart{ 0 };
	//std::vector<FloatType> deltaN;
	float deltaN{ 0 };

	float omegaAverage{ 0 };
	float sumXk2{ 0 };
	float sum2Xk{ 0 };
	float sumKMinusOmegaSquare{ 0 };
	float nbdThreshold{ 0.17 };
};