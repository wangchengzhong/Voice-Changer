#pragma once

#include "PhaseVocoder.h"

class PeakShifter
{
public:
	PeakShifter() //:
		//phi0(phaseVocoder.getWindowSize() /2, 0),
		//psi(phaseVocoder.getWindowSize() /2, 0),
		//psi2(phaseVocoder.getWindowSize() /2, 0),
		//peaks(phaseVocoder.getWindowSize() /2, 0)
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

	void process(FloatType* const buffer, const int bufferSize)

	{
		phaseVocoder.process(buffer, bufferSize, [&](FloatType* const buffer, const int bufferSize)
		{
			auto peakCount = 0;
			//DBG("running peakVocoder process"<<" "<<bufferSize);
			std::vector<FloatType> mags(bufferSize/2, 0);
			std::vector<FloatType> psi(bufferSize/2, 0);
			std::vector<FloatType> phases(bufferSize/2, 0);
			std::vector<FloatType> psi2(bufferSize/2, 0);
			std::vector<FloatType> phi0(bufferSize/2, 0);
			std::vector<FloatType> peaks(bufferSize/2, 0);
			std::vector<FloatType> previousPeaks(bufferSize/2, 0);
			//DBG("SIZE mags size: " << mags.size() << " phases size: " << phases.size() << "SIZE");

			// Get magnitudes and phases from spectral coefficients
			for (int i = 0, x = 0; i < bufferSize - 1; i += 2, ++x)
			{
				const auto real = buffer[i];
				const auto imag = buffer[i + 1];
				mags[x] = sqrtf(real * real + imag * imag);
				phases[x] = atan2(imag, real);
			}

			peaks.clear();

			// Find spectral peaks (local maxima)
			//for (size_t i = 3; i < (bufferSize/2)-2; i += 3)
			//{

			//	if (mags[i] > mags[i-1] && (mags[i] > mags[i-2]) &&
			//		(mags[i] > mags[i+1]) && (mags[i] > mags[i+2]))
			//	{
			//		peakCount += 1;
			//		peaks.push_back(i);
			//	}
			//}
			
			for (size_t i = 4; i < (bufferSize / 2) - 4; i += 4)
			{

				if (mags[i] > mags[i - 1] && (mags[i] > mags[i - 2]) &&
					(mags[i] > mags[i + 1]) && (mags[i] > mags[i + 2])&&(mags[i]>mags[i-3])&&(mags[i]>mags[i+3]))
				{
					peakCount += 1;
					peaks.push_back(i);
				}
			}
			//DBG("peakCount " << peakCount);

			// Propagate peak phases and compute spectral bin phases
			//if (psi.size() == 0)
			//{
			//	psi = phases;
			//}
			//else if (peakCount > 0 && previousPeakCount > 0)
			if (peakCount > 0 && previousPeakCount > 0)
			{
				//DBG("run inside peakCount>0 and previousPeakCount>0");
				auto previousPeakIndex = 1;

				for (int peakIndex = 0; peakIndex < peakCount-1; ++peakIndex)
				{
					//DBG("Peak Count: " << peakCount << " Peak index: " << peakIndex);
					const auto peak = peaks[peakIndex];
					const auto prevPeak = previousPeaks[previousPeakIndex];
					//DBG("\n");
					// Connect current peak to the previous closest peak
					
					while (previousPeakIndex < previousPeakCount &&
						std::abs(peak - previousPeaks[previousPeakIndex + 1]) < std::abs(peak - prevPeak))
					{
						previousPeakIndex += 1;
					}

					// Propagate peak's phase assuming linear frequency
					// Variation between connected peaks p1 and p2
					const auto averagePeak = (peak + prevPeak) * FloatType(0.5);
					const auto omega = juce::MathConstants<float>::twoPi * phaseVocoder.getAnalysisHopSize() *
						averagePeak / (float)phaseVocoder.getWindowSize();
					
					const auto peakDeltaPhi = omega + PhaseVocoder::principalArgument(phases[peak] + phi0[prevPeak] - omega);
					const auto peakTargetPhase = PhaseVocoder::principalArgument(psi[prevPeak] + peakDeltaPhi * timeStretchRatio);
					const auto peakPhaseRotation = PhaseVocoder::principalArgument(peakTargetPhase - phases[peak]);
				
					auto bin1 = 1;
					auto bin2 = phaseVocoder.getWindowSize() / 2;
					
					// Rotate phases of all bins around the current peak
					if (peakIndex == peakCount)
					{
						//DBG("have run at peakIndex==peakCount");

						bin1 = (int)std::round((peaks[peakIndex-1] + peak) * FloatType(0.5));
					}
					else if (peakCount != 1 && peakIndex > 1 )//&& peakIndex != 0)
					{
						
						
						bin1 = (int)std::round((peaks[peakIndex - 1] + peak) * FloatType(0.5));
						//DBG("in bin2,bin1 peakCount: " << peakCount << " peakIndex " << peakIndex<<" in bin2,bin1");
						bin2 = (int)std::round((peaks[peakIndex+1] + peak) * FloatType(0.5));
						
					}
					//DBG("run before bin2-bin1: " << bin2<<" bin1:  "<< bin1 <<"before bin2-bin1");

					for (auto i = 0; i < bin2 - bin1; ++i)//-1
					{
						
						psi2[bin1 + i] = PhaseVocoder::principalArgument(phases[bin1 + i] + peakPhaseRotation);
						
					}
				}

				psi = psi2;
			}
			else
			{
				for (auto i = 0; i < bufferSize/2; ++i)//phaseVocoder.getWindowSize() / 2; ++i)
				{
					const auto omega = juce::MathConstants<float>::twoPi * phaseVocoder.getAnalysisHopSize() *
						i / (float)phaseVocoder.getWindowSize();
					const auto deltaPhi = omega + PhaseVocoder::principalArgument(phases[i] - phi0[i] - omega);
					psi[i] = PhaseVocoder::principalArgument(psi[i] + deltaPhi * timeStretchRatio);
					//DBG("psi: " << psi[i]);
				}
				//DBG("have run at this point");
			}

			// Store state
			phi0 = phases;
			previousPeaks = peaks;
			previousPeakCount = peakCount;

			// Reconstruct whole spectrum
			for (auto i = 0, x = 0; i < bufferSize-1; i += 2, ++x)
			{
				//DBG("bufferSize"<<bufferSize);
				//DBG("sizeOfBufferSize" << psi.size());
				buffer[i] = mags[x] * cos(psi[x]);
				buffer[i + 1] = mags[x] * sin(psi[x]);
				//DBG(i<<"   x: "<<x);
				
			}
			// TODO conjugate first half of spectrum as per dafx example?
		});
		//DBG("have run at this point");
	}

private:
	PhaseVocoder phaseVocoder;
	std::vector<FloatType> phi0;
	std::vector<FloatType> psi;
	std::vector<FloatType> psi2;
	std::vector<int> peaks;
	std::vector<int> previousPeaks;
	float pitchRatio = 0.f;
	float timeStretchRatio = 1.f;
	int previousPeakCount = 0;
};