# pragma once
#include"JuceHeader.h"
class EqualizerComponent :public juce::Component,
    public juce::ChangeListener, private juce::Timer
{
public:
    EqualizerComponent(juce::AudioProcessor& apr)
    {
        // apr.addListener(this);
        openEqButton.onClick = [this]
        {
            openEqButtonClicked();
        };
    }
	// timerCallback
private:
    void timerCallback() override
    {
	    
    }
    void openEqButtonClicked()
    {
	    
    }
    juce::TextButton openEqButton;
};