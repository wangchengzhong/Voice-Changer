#pragma once
#include"JuceHeader.h"
#include"PlayHeadComponent.h"
#include"TrackComponent.h"
#include"TrackHeaderComponent.h"
#include"TrackFooterComponent.h"

class EditComponent:public juce::Component,private tracktion_engine::ValueTreeAllEventListener,
	private FlaggedAsyncUpdater, private juce::ChangeListener
{
public:
	EditComponent(tracktion_engine::Edit&, tracktion_engine::SelectionManager&);
	~EditComponent();

	EditViewState& getEditViewState() { return editViewState; }
private:
	void valueTreeChanged() override{}
	void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
	
	void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override;
	void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
	void valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

	void handleAsyncUpdate() override;
	void resized() override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void buildTracks();

	tracktion_engine::Edit& edit;
	EditViewState editViewState;
	PlayHeadComponent playHead{ edit,editViewState };
	juce::OwnedArray<TrackComponent> tracks;
	juce::OwnedArray<TrackHeaderComponent> headers;
	juce::OwnedArray<TrackFooterComponent> footers;

	bool updateTracks = false, updateZoom = false;


};