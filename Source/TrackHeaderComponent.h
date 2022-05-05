#pragma once
#include <JuceHeader.h>
#include "EditViewState.h"

 //==============================================================================
class TrackHeaderComponent : public juce::Component,
    private tracktion_engine::ValueTreeAllEventListener
{
public:
    TrackHeaderComponent(EditViewState&, tracktion_engine::Track::Ptr);
    virtual ~TrackHeaderComponent();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void resized() override;

private:
    void valueTreeChanged() override {}
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;

    EditViewState& editViewState;
    tracktion_engine::Track::Ptr track;

    juce::ValueTree inputsState;
    juce::Label trackName;
    juce::TextButton armRecordButton{ "R" }, muteButton{ "M" }, soloButton{ "S" }, inputButton{ "I" };
};


