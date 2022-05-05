#pragma once
#include <JuceHeader.h>
#include "Utilities.h"
#include "EditViewState.h"
#include "PluginComponent.h"

 //==============================================================================
class TrackFooterComponent : public juce::Component,
    private FlaggedAsyncUpdater,
    private tracktion_engine::ValueTreeAllEventListener
{
public:
    TrackFooterComponent(EditViewState&, tracktion_engine::Track::Ptr);
    ~TrackFooterComponent();

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void resized() override;

private:
    void valueTreeChanged() override {}
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;

    void handleAsyncUpdate() override;

    void buildPlugins();

    EditViewState& editViewState;
    tracktion_engine::Track::Ptr track;

    juce::TextButton addButton{ "+" };
    juce::OwnedArray<PluginComponent> plugins;

    bool updatePlugins = false;
};
