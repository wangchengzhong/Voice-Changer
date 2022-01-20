#pragma once
#include"JuceHeader.h"
#include"Utilities.h"
#include"ClipComponent.h"
#include"RecordingClipComponent.h"

class TrackComponent:public juce::Component,
	private tracktion_engine::ValueTreeAllEventListener,
	private FlaggedAsyncUpdater,
	private juce::ChangeListener
{
public:
	TrackComponent(EditViewState&, tracktion_engine::Track::Ptr);
	~TrackComponent();
	void paint(Graphics& g) override;
	void mouseDown(const MouseEvent& event) override;
	void resized() override;
private:
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void valueTreeChanged() override;
	void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
	void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override;
	void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
	void valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

	void handleAsyncUpdate() override;
	void buildClips();
	void buildRecordClips();

	EditViewState& editViewState;
	tracktion_engine::Track::Ptr track;
	juce::OwnedArray<ClipComponent>clips;
	std::unique_ptr<RecordingClipComponent> recordingClip;

	bool updateClips = false;
	bool updatePositions = false;
	bool updateRecordClips = false;
};