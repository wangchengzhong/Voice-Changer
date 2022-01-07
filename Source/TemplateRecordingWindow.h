#pragma once
#include"JuceHeader.h"
#include"TemplateRecording.h"
#include"TrainingTemplate.h"
#include"NewWindow.h"
#include"TemplateProjectingWindow.h"
class TemplateRecordingWindow :public juce::Component//public juce::DocumentWindow,public juce::Component
{

public:
	TemplateRecordingWindow( VoiceChanger_wczAudioProcessor& audioProcessor)
		: audioProcessor(audioProcessor)
	{
		
		setSize(600, 300);
		Component::setOpaque(true);
		Component::addAndMakeVisible(liveAudioScroller);

		Component::addAndMakeVisible(explanationLabel);
		explanationLabel.setFont(juce::Font(15.0f));
		explanationLabel.setEditable(false, false, false);
		explanationLabel.setColour(juce::TextEditor::textColourId, juce::Colours::cornflowerblue);
		explanationLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::green);

		Component::addAndMakeVisible(recordButton);
		recordButton.setColour(juce::TextButton::textColourOnId, juce::Colours::darkseagreen);
		recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkseagreen);
		recordButton.onClick = [this]
		{
			if (recorder.isRecording())
			{
				stopRecording();
			}
			else
			{
				startRecording();
			}
		};

		Component::addAndMakeVisible(recordingThumbnail);
		juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
			[this](bool granted)
		{
			int numInputChannels = granted ? 2 : 0;
			audioDeviceManager.initialise(numInputChannels, 2, nullptr, true, {}, nullptr);
		}
		);
		audioDeviceManager.addAudioCallback(&liveAudioScroller);
		audioDeviceManager.addAudioCallback(&recorder);
		
		Component::addAndMakeVisible(trainButton);
		trainButton.setColour(juce::TextButton::buttonColourId, juce::Colours::mediumvioletred);
		trainButton.onClick = [this]
		{
			trainButtonClicked();
		};
	}
	~TemplateRecordingWindow() override
	{
		audioDeviceManager.removeAudioCallback(&recorder);
		audioDeviceManager.removeAudioCallback(&liveAudioScroller);
	}
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);
	}
	void resized() override
	{
		auto area = Component::getLocalBounds();
		liveAudioScroller.setBounds(area.removeFromTop(80).reduced(8));
		recordingThumbnail.setBounds(area.removeFromTop(80).reduced(8));
		recordButton.setBounds(area.removeFromBottom(50).reduced(8));
		//explanationLabel.setBounds(area.removeFromTop(80).reduced(10));
		trainButton.setBounds(area.removeFromTop(90).reduced(10));
	}

	void trainButtonClicked()
	{
		if (templateProjectingWindow)
		{
			templateProjectingWindow->broughtToFront();
		}
		else
		{
			templateProjectingWindow = new NewWindow(juce::String("templateProjectingWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);
			
			templateProjectingWindow->setContentOwned(new TemplateProjectingWindow(audioProcessor), true);
			templateProjectingWindow->addToDesktop();
			templateProjectingWindow->centreWithSize(600, 750);
			templateProjectingWindow->setVisible(true);
		}
	}
private:
	VoiceChanger_wczAudioProcessor& audioProcessor;
	juce::AudioDeviceManager audioDeviceManager;
	LiveScrollingAudioDisplay liveAudioScroller;
	RecordingThumbnail recordingThumbnail;
	AudioRecorder recorder{ recordingThumbnail.getAudioThumbnail() };
	juce::Label explanationLabel{ {}, juce::String(
		juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x85\xa5\xe6\xa8\xa1\xe6\x9d\xbf\xe8\xbf\x9b\xe8\xa1\x8c\xe8\xae\xad\xe7\xbb\x83\xe6\x8c\x89\xe4\xb8\x8b\xe5\xbd\x95\xe5\x88\xb6\xe9\x94\xae\xe5\xbc\x80\xe5\xa7\x8b\xe5\xbd\x95\xe9\x9f\xb3"))
	};
	juce::TextButton recordButton{ juce::CharPointer_UTF8("\xe5\xbd\x95\xe9\x9f\xb3") };
	juce::TextButton trainButton{ juce::CharPointer_UTF8("\xe8\xae\xad\xe7\xbb\x83\xe6\x98\xa0\xe5\xb0\x84") };
	juce::File lastRecording;

	juce::Component::SafePointer<juce::DocumentWindow> templateProjectingWindow;
	void startRecording()
	{
		if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage))
		{
			SafePointer<TemplateRecordingWindow> safeThis(this);
			juce::RuntimePermissions::request(
				juce::RuntimePermissions::writeExternalStorage,
				[safeThis](bool granted)mutable
				{
					if (granted)
						safeThis->startRecording();
				}
			);
			return;
		}
		auto parentDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
		lastRecording = parentDir.getNonexistentChildFile("template speech", ".wav");
		recorder.startRecording(lastRecording);
		recordButton.setButtonText(juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2"));
		recordingThumbnail.setDisplayFullThumbnail(false);
	}

	void stopRecording()
	{
		recorder.stop();
		lastRecording = juce::File();
		recordButton.setButtonText(juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x88\xb6"));
		recordingThumbnail.setDisplayFullThumbnail(true);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateRecordingWindow)
};