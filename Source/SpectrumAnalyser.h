#pragma once
#include<juce_dsp/juce_dsp.h>
#include<juce_gui_basics/juce_gui_basics.h>
using namespace juce;
template<typename Type>
class SpectrumAnalyser:public juce::Thread
{
public:
	SpectrumAnalyser():juce::Thread("Spectrum Analyser")
	, level(new float[spectrumNum])
	, spectrum(juce::Image::RGB, 512, 256, true)
	{
		
	}
	~SpectrumAnalyser()override = default;

	void addAudioData(const AudioBuffer<Type>& buffer, int startChannel, int numChannels)
	{
		if(abstractFifo.getFreeSpace()<buffer.getNumSamples())
			return;
		int start1, block1, start2, block2;
		abstractFifo.prepareToWrite(buffer.getNumSamples(), start1, block1, start2, block2);
		audioFifo.copyFrom(0, start1, buffer.getReadPointer(startChannel), block1);
		if (block2 > 0)
		{
			audioFifo.copyFrom(0, start2, buffer.getReadPointer(startChannel, block1), block2);
		}
		for(int channel = startChannel+1; channel<startChannel+numChannels;++channel)
		{
			if (block1 > 0)audioFifo.addFrom(0, start1, buffer.getReadPointer(channel), block1);
			if (block2 > 0)audioFifo.addFrom(0, start2, buffer.getReadPointer(channel, block1), block2);
		}
		abstractFifo.finishedWrite(block1 + block2);
		waitForData.signal();
	}
	void setupSpectralAnalyser(int audioFifoSize, Type sampleRateToUse)
	{
		sampleRate = sampleRateToUse;
		audioFifo.setSize(1, audioFifoSize);
		abstractFifo.setTotalSize(audioFifoSize);
		startThread(5);
	}

	void run()override
	{
		while (!threadShouldExit())
		{
			if(abstractFifo.getNumReady()>=fft.getSize())
			{
				ScopedLock lockedForWriting(pathCreationLock);
				fftBuffer.clear();
				int start1, block1, start2, block2;
				abstractFifo.prepareToRead(fft.getSize(), start1, block1, start2, block2);
				if (block1 > 0)fftBuffer.copyFrom(0, 0, audioFifo.getReadPointer(0, start1), block1);
				if (block2 > 0)fftBuffer.copyFrom(0, block1, audioFifo.getReadPointer(0, start2), block2);
				abstractFifo.finishedRead((block1 + block2) / 2);

				windowing.multiplyWithWindowingTable(fftBuffer.getWritePointer(0), size_t(fft.getSize()));
				fft.performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));
				
				newDataAvailable = true;
			}
			if(abstractFifo.getNumReady()<fft.getSize())
			{
				waitForData.wait(100);
			}
		}
	}
	Image& createSpectrumPlot( Colour color = Colours::lightskyblue)
	{
		{
			ScopedLock lockedForReading(pathCreationLock);
			auto leverRange = FloatVectorOperations::findMinAndMax(fftBuffer.getReadPointer(0), fft.getSize() / 2);
			auto levelMax = jmax(leverRange.getEnd(), 200.0f);
			auto levelMin = jmin(leverRange.getStart(), 0.0f);
			auto minDB = -60.0f;
			auto maxDB = 0.0f;
			for (int i = 0; i < spectrumNum; i++)
			{
				auto pos = (int)(fft.getSize() / 2) * i / spectrumNum;
				auto data = jmap(fftBuffer.getSample(0, pos), levelMin, levelMax, 0.0f, 1.0f);
				auto power = jmap(
					jlimit(minDB, maxDB, Decibels::gainToDecibels(data)),
					minDB, maxDB, 0.0f, 1.0f
				);
				level.get()[i] = power;
			}
		}
		int postPoint = 0;
		float postLevel = 0.0f;
		spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
		Graphics g(spectrum);
		for (int x = 1; x < 512; x++)
		{
			float skewedProportionX = 0.0f;
			skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)x / 512.0f) * 0.2f);
			auto fftDataIndex = jlimit(0, 1024, (int)(skewedProportionX * 1024.));
			auto lv = level.get()[fftDataIndex];
			if(std::fabs(postLevel-lv)>0.00001)
			{
				g.setColour(color);
				g.setOpacity(1.0);
				g.drawLine(
					(float)postPoint, 
					jmap(postLevel, 0.0f, 1.0f, 256.0f, 0.f),
					(float)x, 
					jmap(lv, 0.0f, 1.0f, 256.0f, 0.0f)
				);
				{
					g.setOpacity(0.3);
					Path pen;
					pen.startNewSubPath(Point<float>((float)postPoint, jmap(postLevel, 0.0f, 1.0f, 256.0f, 0.0f)));
					pen.lineTo(Point<float>((float)x, jmap(lv, 0.0f, 1.0f, 256.0f, 0.0f)));
					pen.lineTo(Point<float>((float)x, 256.0f));
					pen.lineTo(Point<float>((float)postPoint, 256.0f));
					pen.closeSubPath();
					g.fillPath(pen);
				}
				postPoint = x;
				postLevel = lv;
			}
		}
		g.setOpacity(1.0);
		return spectrum;
	}
private:
	std::atomic<bool> newDataAvailable;
	dsp::FFT fft{ 11 };
	dsp::WindowingFunction<Type>windowing{ size_t(fft.getSize()),dsp::WindowingFunction<Type>::hann,true };
	AudioBuffer<float> fftBuffer{ 1,fft.getSize() * 2 };

	AbstractFifo abstractFifo{ 48000 };
	AudioBuffer<Type> audioFifo;

	WaitableEvent waitForData;
	CriticalSection pathCreationLock;
	Type sampleRate{};
	//std::vector<float> spectralBuffer;
	//BlockCircularBuffer<float> analysisBuffer;

	int spectrumNum{ 1024 };
	std::shared_ptr<float> level;
public:
	Image spectrum;
};