#pragma once

#include"JuceHeader.h"
#include"EditViewState.h"

class TrackHeaderComponent:public juce::Component,
	private tracktion_engine::ValueTreeAllEventListener
{
public:
	TrackHeaderComponent(EditViewState&, tracktion_engine::Track::Ptr);
	virtual ~TrackHeaderComponent();

	void paint(Graphics& g) override;
	void mouseDown(const MouseEvent& event) override;
	void resized() override;
private:
	void valueTreeChanged() override{}

	void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
	EditViewState& editViewState;
	tracktion_engine::Track::Ptr track;
	juce::ValueTree inputsState;
	juce::Label trackName;
	juce::TextButton armRecordButton{ "R" }, muteButton{ "M" },
		soloButton{ "S" }, inputButton{ "I" };
};
