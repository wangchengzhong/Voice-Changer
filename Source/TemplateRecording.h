#pragma once
#include"JuceHeader.h"
#include"AudioLiveScrollingDisplay.h"

class AudioRecorder :public juce::AudioIODeviceCallback
{
public:
	AudioRecorder(juce::AudioThumbnail& thumbnailToUpdate)
		: thumbnail(thumbnailToUpdate)
	{
		backgroundThread.startThread();
	}
	~AudioRecorder()override
	{
		stop();
	}

	void startRecording(const juce::File& file)
	{
		stop();
		if (sampleRate > 0)
		{
			file.deleteFile();
			if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream()))
			{
				juce::WavAudioFormat wavFormat;
				if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 1, 16, {}, 0))
				{
					fileStream.release();
					threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));
					
					nextSampleNum = 0;

					const juce::ScopedLock sl(writerLock);
					activeWriter = threadedWriter.get();
				}
			}
		}
	}



	void stop()
	{
		const juce::ScopedLock sl(writerLock);
		activeWriter = nullptr;
	}

	bool isRecording()const
	{
		return activeWriter.load() != nullptr;
	}

	void audioDeviceAboutToStart(juce::AudioIODevice* device)override
	{
		sampleRate = device->getCurrentSampleRate();
	}
	void audioDeviceStopped()override
	{
		sampleRate = 0;
	}
	void audioDeviceIOCallback(
		const float** inputChannelData,
		int numInputChannels,
		float** outputChannelData,
		int numOutputChannels,
		int numSamples
	)override
	{
		const juce::ScopedLock sl(writerLock);
		if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
		{
			activeWriter.load()->write(inputChannelData, numSamples);

			juce::AudioBuffer<float> buffer(const_cast<float**>(inputChannelData), thumbnail.getNumChannels(), numSamples);
			thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
			nextSampleNum += numSamples;
		}
		for (int i = 0; i < numOutputChannels; ++i)
		{
			if (outputChannelData[i] != nullptr)
			{
				juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
			}
		}
	}
	
private:
	juce::AudioThumbnail& thumbnail;
	juce::TimeSliceThread backgroundThread{ "Audio Recorder Thread" };
	std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
	
	double sampleRate = 0.0;
	juce::int64 nextSampleNum = 0;

	juce::CriticalSection writerLock;
	std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
};



class RecordingThumbnail :public juce::Component,
	private juce::ChangeListener
{
public:
	RecordingThumbnail()
	{
		formatManager.registerBasicFormats();
		thumbnail.addChangeListener(this);

	}
	~RecordingThumbnail()override
	{
		thumbnail.removeChangeListener(this);
	}
	juce::AudioThumbnail& getAudioThumbnail()
	{
		return thumbnail;
	}
	void setDisplayFullThumbnail(bool displayFull)
	{
		displayFullThumb = displayFull;
		repaint();
	}
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::darkgrey);
		g.setColour(juce::Colours::lightgrey);
		if (thumbnail.getTotalLength() > 0.0)
		{
			auto endTime = displayFullThumb ? thumbnail.getTotalLength() :
				juce::jmax(30.0, thumbnail.getTotalLength());
			auto thumbArea = getLocalBounds();
			thumbnail.drawChannels(g, thumbArea.reduced(2), 0.0, endTime, 1.0f);
		}
		else
		{
			g.setFont(14.0f);
			g.drawFittedText("no file recorded", getLocalBounds(), juce::Justification::centred, 2);
		}
	}
private:
	juce::AudioFormatManager formatManager;
	juce::AudioThumbnailCache thumbnailCache{ 10 };
	juce::AudioThumbnail thumbnail{ 512,formatManager,thumbnailCache };

	bool displayFullThumb = false;

	void changeListenerCallback(juce::ChangeBroadcaster* source)override
	{
		if (source == &thumbnail)
			repaint();
	}
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordingThumbnail)
};