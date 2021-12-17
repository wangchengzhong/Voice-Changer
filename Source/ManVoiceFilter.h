#pragma once
#include"ManVoiceExtract.h"
#include"JuceHeader.h"
#include"PhaseVocoder.h"

class SpectrumFilter:public TemplateParameters
{
public:
	SpectrumFilter()//std::vector<float>& spectrumData, const int spectrumSize, double mixRatio)
		//:spectrumSize(spectrumSize)
			
	{
	}
	
	int getLatencyInSamples() const { return phaseVocoder.getLatencyInSamples(); }

	void setPitchRatio(float ratio)
	{
		if (pitchRatio == ratio)
			return;
		const juce::SpinLock::ScopedLockType lock(phaseVocoder.getParamLock());

		pitchRatio = std::clamp(ratio, PhaseVocoder::MinPitchRatio, PhaseVocoder::MaxPitchRatio);
		phaseVocoder.setPitchRatio(ratio);
		phaseVocoder.setSynthesisHopSize((int)(phaseVocoder.getWindowSize() / (float)phaseVocoder.getWindowOverlapCount()));
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

		timeStretchRatio = phaseVocoder.getSynthesisHopSize() / (float)phaseVocoder.getAnalysisHopSize();
	}
	float getBaseFrequency(FloatType* const buffer, const int spectrumSize)
	{
		auto peakCount = 0;
		std::vector<FloatType>phases(spectrumSize / 2, 0);
		std::vector<FloatType>mags(spectrumSize / 2, 0);
		for (int i = 0, x = 0; i < spectrumSize; i += 2, ++x)
		{
			const auto real = buffer[i];
			const auto imag = buffer[i + 1];
			mags[x] = sqrtf(real * real + imag * imag);
			phases[x] = atan2(imag, real);
		}
		peaks.clear();
		for (int i = 3; i < (spectrumSize / 4) - 4; i += 3)
		{
			if ((mags[i] > mags[i - 1]) && (mags[i] > mags[i - 2]) &&
				(mags[i] > mags[i + 1]) && (mags[i] > mags[i + 2]) ) //&&
				//(mags[i] > mags[i - 3]) && (mags[i] > mags[i + 3]))
			{
				peakCount + 1;
				peaks.push_back(i);
			}
		}
		if (peakCount > 0 && previousPeakCount > 0)
		{
			auto previousPeakIndex = 0;
			for (int peakIndex = 0; peakIndex < peakCount - 1; ++peakIndex)
			{
				const auto peak = peaks[peakIndex];
				const auto prevPeak = previousPeaks[previousPeakIndex];

				while(previousPeakIndex<previousPeakCount&&
					std::abs(peak - previousPeaks[previousPeakIndex + 1]))
				{
					previousPeakIndex += 1;
				}
				if ((std::abs(previousPeakIndex - peakIndex) <= 1) &&
					peakIndex >= 4 && peakIndex <= 45)
				{
					return peakIndex;
				}
				else if (peakIndex > 90)
				{
					return -1;
				}
			}
			return -1;
		}
	}
	
	void process(FloatType* const buffer, const int bufferSize)
	{
		phaseVocoder.process(buffer, bufferSize, [&](FloatType* const buffer, const int bufferSize)
		{
			if (bufferSize >= templateSize * 2)
			{
				for (int i = 0, index = 0; i < templateSize * 2; i += 2, index += 1)
				{
					denominator = pow(originRatio + spectrumTemplate[i], 2) + pow(spectrumTemplate[i + 1], 2);
					numeratorReal = buffer[i] * spectrumTemplate[i] + buffer[i + 1] * spectrumTemplate[i + 1];
					numeratorImag = buffer[i + 1] * spectrumTemplate[i] - buffer[i] * spectrumTemplate[i + 1];
					
					buffer[i] = numeratorReal / denominator;
					buffer[i + 1] = numeratorImag / denominator;
				}
			}
		}
		);
	}
private:
	PhaseVocoder phaseVocoder;
	std::vector<int>peaks;
	std::vector<int> previousPeaks;
	std::vector<float> spectrumBuffer;
	int originRatio{ 3 };
	int previousPeakCount{ 0 };
	int templateSize{ 2048 };
	double denominator{ 1.0f };
	float numeratorReal{ 1.0f };
	float numeratorImag{ 0.0f };
	float pitchRatio{ 1.f };
	float timeStretchRatio{ 1.f };
	//const int spectrumSize{ 4096 };
	
};
