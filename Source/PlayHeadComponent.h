#pragma once

#include<JuceHeader.h>
#include"editviewstate.h"

class PlayHeadComponent:public juce::Component, private juce::Timer
{
public:
	PlayHeadComponent(tracktion_engine::Edit&, EditViewState&);
	void paint(juce::Graphics& g)override;
	bool hitTest(int x, int y)override;
	void mouseDrag(const MouseEvent& event) override;
	void mouseDown(const MouseEvent& event) override;
	void mouseUp(const MouseEvent& event) override;

private:
	void timerCallback() override;
	tracktion_engine::Edit& edit;
	EditViewState& editViewState;

	int xPosition = 0;
	bool firstTimer = true;
};