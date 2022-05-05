
#pragma once
#include <JuceHeader.h>
#include "EditViewState.h"

 //==============================================================================
class PlayheadComponent : public juce::Component, private juce::Timer
{
public:
    PlayheadComponent(tracktion_engine::Edit&, EditViewState&);

    void paint(juce::Graphics& g) override;
    bool hitTest(int x, int y) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    tracktion_engine::Edit& edit;
    EditViewState& editViewState;

    int xPosition = 0;
    bool firstTimer = true;
};

