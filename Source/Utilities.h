#pragma once
#include"JuceHeader.h"
namespace Helpers
{
	static inline void addAndMakeVisible(juce::Component& parent,
		const juce::Array<juce::Component*>&children)
	{
		for (auto c : children)
			parent.addAndMakeVisible(c);
	}

	static inline juce::String getStringOrDefault(const juce::String stringToTest, const juce::String& stringToReturnIfEmpty)
	{
		return stringToTest.isEmpty() ? stringToReturnIfEmpty : stringToTest;
	}
	static inline juce::File findRecentEdit(const juce::File& dir)
	{
		auto files = dir.findChildFiles(juce::File::findFiles, false, "*.tracktionedit");
		if(files.size()>0)
		{
			files.sort();
			return files.getLast();
		}
		return{};
	}

	inline std::unique_ptr<juce::InputStream> createZipStreamForEmbeddedResource(const char* resourceName)
	{
		int resourceSize = 0;
		auto* res = BinaryData::getNamedResource(resourceName, resourceSize);
		auto* mstream = new juce::MemoryInputStream(res, resourceSize, true);
		return std::unique_ptr<juce::MemoryInputStream>(mstream);
	}

}

namespace PlayHeadHelpers
{
	static inline juce::String timeToTimecodeString(double seconds)
	{
		auto millisecs = juce::roundToInt(seconds * 1000.0);
		auto absMillisecs = std::abs(millisecs);
		return juce::String::formatted("%02d:%02d:%02d.%03d",
			millisecs / 3600000,
			(absMillisecs / 60000) % 60,
			(absMillisecs / 1000) % 60,
			absMillisecs % 1000);
	}

	static inline juce::String quarterNotePositionToBarsBeatsString(double quarterNotes,int numerator, int denominator)
	{
		if (numerator == 0 || denominator == 0)
			return "1|1|000";
		auto quarterNotesPerBar = (double)numerator * 4.0 / (double)denominator;
		auto beats = fmod(quarterNotes, quarterNotesPerBar) / quarterNotesPerBar * numerator;
		auto bar = (int)quarterNotes / quarterNotesPerBar + 1;
		auto beat = (int)beats + 1;
		auto ticks = (int)beats + 1;

		return juce::String::formatted("%d|%d%03d", bar, beat, ticks);
	}

	static inline juce::String getTimecodeDisplay(const juce::AudioPlayHead::CurrentPositionInfo& pos)
	{
		juce::MemoryOutputStream displayText;
		displayText << juce::String(pos.bpm, 2) << " bpm, "
			<< pos.timeSigNumerator << '/' << pos.timeSigDenominator
			<< "  -  " << timeToTimecodeString(pos.timeInSeconds)
			<< "  -  " << quarterNotePositionToBarsBeatsString(pos.ppqPosition,
				pos.timeSigNumerator,
				pos.timeSigDenominator);
		if (pos.isRecording)
			displayText << "  (recording)";
		else if (pos.isPlaying)
			displayText << "  (playing)";
		else
			displayText << "  (stopped)";

		return displayText.toString();
	}
}


class FlaggedAsyncUpdater:public juce::AsyncUpdater
{
public:
	void markAndUpdate(bool &flag)
	{
		flag = true;
		triggerAsyncUpdate();
	}

	bool compareAndReset(bool& flag)noexcept
	{
		if (!flag)
			return false;
		flag = false;
		return true;
	}

};

struct  Thumbnail:public juce::Component
{
	Thumbnail(tracktion_engine::TransportControl&tc):transport(tc)
	{}
private:
	tracktion_engine::TransportControl& transport;
	tracktion_engine::SmartThumbnail smartThumbnail{ transport.engine,tracktion_engine::AudioFile(transport.engine),*this,nullptr };
	juce::DrawableRectangle cursor;
	tracktion_engine::LambdaTimer cursorUpdater;

	void updateCursorPosition()
	{
		const double loopLength = transport.getLoopRange().getLength();
		const double proportion = loopLength == 0.0 ? 0.0 : transport.getCurrentPosition() / loopLength;
		auto r = getLocalBounds().toFloat();
		const float x = r.getWidth() * float(proportion);
		cursor.setRectangle(r.withWidth(2.0f).withX(x));
	}

};
