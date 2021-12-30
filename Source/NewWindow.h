#pragma once
#include"JuceHeader.h"
class NewWindow : public juce::DocumentWindow
{
public:
	NewWindow(const juce::String& name, juce::Colour backgroundColor, int buttonsNeeded) :
		DocumentWindow(name, backgroundColor, buttonsNeeded)
	{
	}
	void closeButtonPressed()override
	{
		delete[] this;
	}
};