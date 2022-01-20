#pragma once
#include"JuceHeader.h"
#include"ClipComponent.h"

class RecordingClipComponent:public juce::Component, private juce::Timer
{
public:
	RecordingClipComponent(tracktion_engine::Track::Ptr t, EditViewState&);
	void paint(Graphics& g) override;
private:
	void timerCallback() override;
	void updatePosition();
	void initialiseThumnailAndPunchTime();
	void drawThumbnail(juce::Graphics& g, juce::Colour waveFormColour)const;
	bool getBoundsAndTime(juce::Rectangle<int>& bounds, juce::Range<double>& times)const;

	tracktion_engine::Track::Ptr track;
	EditViewState& editViewState;
	tracktion_engine::RecordingThumbnailManager::Thumbnail::Ptr thumbnail;

	double punchInTime = -1.0;

};