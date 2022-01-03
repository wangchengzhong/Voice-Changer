#pragma once
#include"JuceHeader.h"
class TrainingTemplate
{
public:
	TrainingTemplate()
	{

	}
	void train()
	{
		copyBuffer();
		// DBG(filePath.getFullPathName());
	}
	void copyBuffer()
	{

	}
private:
	const juce::File filePath{ juce::File::getCurrentWorkingDirectory() };

	const juce::File templateDirPath = filePath.getChildFile("template.wav");
	juce::AudioSampleBuffer source1Buffer;
	juce::AudioSampleBuffer source2Buffer;
	juce::AudioFormatManager formatManager;

	
};

