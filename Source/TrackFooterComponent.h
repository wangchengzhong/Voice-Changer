#pragma once
#include<JuceHeader.h>
#include"Utilities.h"
#include"EditViewState.h"
#include"PluginComponent.h"

class TrackFooterComponent:public juce::Component,
	private FlaggedAsyncUpdater,private  tracktion_engine::ValueTreeAllEventListener
{
public:
	TrackFooterComponent(EditViewState&, tracktion_engine::Track::Ptr);
	~TrackFooterComponent();
	void paint(Graphics& g) override;
	void mouseDown(const MouseEvent& event) override;
	void resized() override;

private:
	void valueTreeChanged() override{}
	void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override;
	void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
	void valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

	void handleAsyncUpdate() override;

	void buildPlugins();

	EditViewState& editViewState;
	tracktion_engine::Track::Ptr track;

	juce::TextButton addButton{ "+" };
	juce::OwnedArray<PluginComponent> plugins;
	bool updatePlugins = false;

};
