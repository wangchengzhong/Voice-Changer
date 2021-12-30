#pragma once
#include"JuceHeader.h"

class TemplateRecordingWindow :public juce::DocumentWindow
{
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateRecordingWindow);
public:
	TemplateRecordingWindow(const juce::String& name, juce::Colour backgroundColor, int buttonsNeeded)
		:DocumentWindow(name, backgroundColor, buttonsNeeded)
	{
		setSize(500, 500);
	}
	void closeButtonPressed()override
	{
		delete this;
	}
};