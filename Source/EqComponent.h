# pragma once
#include"JuceHeader.h"
#include"NewWindow.h"

class EqComponent :public juce::Component
{
public:
    EqComponent(VoiceChanger_wczAudioProcessor& audioProcessor):audioProcessor(audioProcessor)
    {

    }
	// timerCallback
private:
    VoiceChanger_wczAudioProcessor& audioProcessor;

};