#pragma once
#include"JuceHeader.h"
class TrainingTemplate
{
public:
	TrainingTemplate(juce::AudioSampleBuffer a, juce::AudioSampleBuffer b, std::vector<float>& voiceChangerParameter)
		:sourceBuffer(a), targetBuffer(b), voiceChangerParameter(voiceChangerParameter)
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
	juce::AudioSampleBuffer& sourceBuffer;
	juce::AudioSampleBuffer& targetBuffer;
	std::vector<float>& voiceChangerParameter;
	// juce::AudioFormatManager formatManager;

	
};

