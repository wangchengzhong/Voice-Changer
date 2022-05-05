

 /*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com
*/

#pragma once
#include <JuceHeader.h>

#include "PlayheadComponent.h"

#include "TrackComponent.h"
#include "TrackHeaderComponent.h"
#include "TrackFooterComponent.h"

//==============================================================================
class EditComponent : public juce::Component,
    private tracktion_engine::ValueTreeAllEventListener,
    private FlaggedAsyncUpdater,
    private juce::ChangeListener
{
public:
    EditComponent(tracktion_engine::Edit&, tracktion_engine::SelectionManager&);
    ~EditComponent();

    EditViewState& getEditViewState() { return editViewState; }

private:
    void valueTreeChanged() override {}

    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override;
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override;
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override;

    void handleAsyncUpdate() override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster*) override { repaint(); }


    void buildTracks();

    tracktion_engine::Edit& edit;

    EditViewState editViewState;

    PlayheadComponent playhead{ edit, editViewState };
    juce::OwnedArray<TrackComponent> tracks;
    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::OwnedArray<TrackFooterComponent> footers;

    bool updateTracks = false, updateZoom = false;
};
