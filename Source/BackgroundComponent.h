#pragma once
#include"JuceHeader.h"

class BackgroundComponent :public juce::Component
{
public:
	BackgroundComponent();
	~BackgroundComponent() override;

	void paint(juce::Graphics&) override;
	void resized() override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackgroundComponent);
};