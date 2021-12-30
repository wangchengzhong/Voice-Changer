#pragma once
#include"JuceHeader.h"
class LiveScrollingAudioDisplay :public juce::AudioVisualiserComponent,
	public juce::AudioIODeviceCallback
{
public:
	LiveScrollingAudioDisplay() :AudioVisualiserComponent(1)
	{
		setSamplesPerBlock(256);
		setBufferSize(1024);
	}
	void audioDeviceAboutToStart(juce::AudioIODevice*)override
	{
		clear();
		
	}
	void audioDeviceStopped()override
	{
		clear();
	}
	void audioDeviceIOCallback(
		const float** inputChannelData,
		int numInputChannels,
		float** outputChannelData,
		int numOutputChannels,
		int numberOfSamples
	)override
	{
		for (int i = 0; i < numberOfSamples; ++i)
		{
			float inputSample = 0;
			for (int channel = 0; channel < numInputChannels; ++channel)
			{
				if (const float* inputChannel = inputChannelData[channel])
					inputSample += inputChannel[i];
			}
			inputSample *= 10.0f;
			pushSample(&inputSample, 1);
		}
		for (int j = 0; j < numOutputChannels; ++j)
		{
			if (float* outputChannel = outputChannelData[j])
			{
				juce::zeromem(outputChannel, (size_t)numberOfSamples * sizeof(float));
			}

		}
	}
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LiveScrollingAudioDisplay)
};