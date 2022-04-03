#pragma once
#include"JuceHeader.h"
#include"TransportInformation.h"
#include"vchsm/train_C.h"
//#include"D:\1a\voice_changer@wcz\VoiceChanger@wcz\VC\CppAlgo\include\vchsm\train_C.h"
#define VERBOSE_TRUE 1
class TemplateProjectingWindow :public juce::Component,
	public juce::ChangeListener, 
	private juce::Timer
	//private juce::Thread
{
public:
	//void run()override
	//{
	//	while (!threadShouldExit())
	//	{
	//		//startThread();

	//	}
	//	wait(500);
	//}
	TemplateProjectingWindow(
		VoiceChanger_wczAudioProcessor& audioProcessor
	):
		thumbnailCache(5),
		sourceThumbnail(512,formatManager,thumbnailCache),
		targetThumbnail(512,formatManager,thumbnailCache),
		audioProcessor(audioProcessor)
	{
		setSize(600, 750);
		setOpaque(true);
		addAndMakeVisible(&loadSourceButton);
		loadSourceButton.onClick = [this] { openButtonClicked(Source); };

		addAndMakeVisible(&loadTargetButton);
		loadTargetButton.onClick = [this] { openButtonClicked(Target); };

		addAndMakeVisible(&playButton);
		playButton.setButtonText("play");
		playButton.onClick = [this] { playButtonClicked(); };
		playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
		
		addAndMakeVisible(&stopButton);
		stopButton.setButtonText("stop");
		stopButton.onClick = [this] { stopButtonClicked(); };
		stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);

		addAndMakeVisible(&projectButton);
		projectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::skyblue);
		projectButton.onClick = [this] { projectButtonClicked(); };

		formatManager.registerBasicFormats();
		// transportSource.addChangeListener(this);
		sourceThumbnail.addChangeListener(this);
		targetThumbnail.addChangeListener(this);

		startTimer(40);
	}
	~TemplateProjectingWindow()override
	{
		
		// audioDeviceManager.removeAudioCallback();
	}


	void timerCallback()override
	{
		repaint();
	}
	void changeListenerCallback(juce::ChangeBroadcaster* source)override
	{
		// if (source == &transportSource)
			//transportSourceChanged();
		if (source == &sourceThumbnail || source == &targetThumbnail)
			thumbnailChanged();
	}

	void paint(juce::Graphics& g)override
	{
		g.fillAll(juce::Colours::black);
		auto area = getLocalBounds();
		juce::Rectangle<int>targetThumbnailBounds(area.removeFromBottom(260).reduced(10));
		juce::Rectangle<int>sourceThumbnailBounds(area.removeFromBottom(260).reduced(10));
		if (sourceThumbnail.getNumChannels() == 0 || targetThumbnail.getNumChannels() == 0)
			paintIfNoFileLoaded(g, sourceThumbnailBounds, targetThumbnailBounds);
		if (sourceThumbnail.getNumChannels() != 0)
		{
			paintIfFileLoaded(g, sourceThumbnail, sourceThumbnailBounds);
		}
		if (targetThumbnail.getNumChannels() != 0)
			paintIfFileLoaded(g, targetThumbnail, targetThumbnailBounds);
	}
	void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& sourceThumbnailBounds,
												const juce::Rectangle<int>& targetThumbnailBounds)
	{
		g.setColour(juce::Colours::darkgrey);
		g.fillRect(sourceThumbnailBounds);
		g.fillRect(targetThumbnailBounds);
		g.setColour(juce::Colours::white);
		g.drawFittedText(juce::CharPointer_UTF8("\xe6\xb2\xa1\xe6\x9c\x89\xe5\x8e\x9f\xe5\xa3\xb0\xe9\x9f\xb3\xe5\x8a\xa0\xe8\xbd\xbd"), sourceThumbnailBounds, juce::Justification::centred, 1);
		g.drawFittedText(juce::CharPointer_UTF8("\xe6\xb2\xa1\xe6\x9c\x89\xe7\x9b\xae\xe6\xa0\x87\xe5\xa3\xb0\xe9\x9f\xb3\xe5\x8a\xa0\xe8\xbd\xbd"), targetThumbnailBounds, juce::Justification::centred, 1);
	}
	void paintIfFileLoaded(juce::Graphics& g, juce::AudioThumbnail& thumbnail, const juce::Rectangle<int>& thumbnailBounds)
	{
		g.setColour(juce::Colours::white);
		g.fillRect(thumbnailBounds);
		
		g.setColour(juce::Colours::red);
		
		auto audioLength = (float)sourceThumbnail.getTotalLength();
		sourceThumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);

		g.setColour(juce::Colours::green);
		//auto audioPosition = (float)transportSource.getCurrentPosition();
		//auto drawPosition = (audioPosition / audioLength) * (float)thumbnailBounds.getWidth() + (float)thumbnailBounds.getX();
		//g.drawLine(drawPosition, (float)thumbnailBounds.getY(), drawPosition, (float)thumbnailBounds.getBottom(), 2.0f);
	}
	void resized() override
	{
		auto area = getLocalBounds();
		loadSourceButton.setBounds(area.removeFromTop(40).reduced(5));
		loadTargetButton.setBounds(area.removeFromTop(40).reduced(5));
		playButton.setBounds(area.removeFromTop(40).reduced(5));
		stopButton.setBounds(area.removeFromTop(40).reduced(5));
		projectButton.setBounds(area.removeFromTop(40).reduced(5));
	}


	void thumbnailChanged()
	{
		repaint();
	}

	//template<typename T>
	void openButtonClicked(TransportFileType newType)
	{
		chooser = std::make_unique<juce::FileChooser>("select file..",
			juce::File{}, "*.wav; *.flac; *.mp3");
		if (chooser->browseForFileToOpen())
		{
			auto file = chooser->getResult();
			if (file != juce::File{})
			{
				auto* reader = formatManager.createReaderFor(file);
				if (reader != nullptr)
				{
					//DBG("SammpleRate: " << reader->sampleRate);
					auto duration = (float)reader->lengthInSamples / reader->sampleRate;
					if (duration < 1000)
					{
						audioProcessor.setState(Stopping);
						audioProcessor.setTarget(newType);
						// changeState(Stopping);
						//DBG("numChannels: " << (int)reader->numChannels);
						audioProcessor.pPlayBuffer->clear();
						
						audioProcessor.pPlayBuffer->setSize((int)reader->numChannels, (int)reader->lengthInSamples);
						reader->read(
							audioProcessor.pPlayBuffer,
							0,
							(int)reader->lengthInSamples,
							0,
							true,
							true
						);
					}
					auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
					// transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
					playButton.setEnabled(true);
					if (newType == Source)
						sourceThumbnail.setSource(new juce::FileInputSource(file));
					else
						targetThumbnail.setSource(new juce::FileInputSource(file));
					// readerSource.reset(newSource.release());
				}
			}
		}
	}
	void playButtonClicked()
	{
		// audioProcessor.setState(Starting);
		// changeState(Starting);
		convertSingle(modelFile, wavFile, convertedWavFile, 1);
	}
	void stopButtonClicked()
	{
		audioProcessor.setState(Stopping);
		// changeState(Stopping);
	}
	void projectButtonClicked()
	{
		const int n = 8;
		const char* sourceAudioList[n];
		const char* targetAudioList[n];
		for(int i = 0; i < n; ++i)
		{
			char* buff = new char[100];
			std::sprintf(buff, "%s%d.wav", sourceAudioDir, i + 1);
			sourceAudioList[i] = buff;
			buff = new char[100];
			std::sprintf(buff, "%s%d.wav", targetAudioDir, i + 1);
			targetAudioList[i] = buff;
		}
		
		trainHSMModel(sourceAudioList, targetAudioList, n, 50, modelFile, VERBOSE_TRUE);
		for(int i = 0; i < n; ++i)
		{
			delete[] sourceAudioList[i];
			delete[] targetAudioList[i];
		}
	}
	bool playing{ false };
private:
	const char* sourceAudioDir = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/source_train/";
	const char* targetAudioDir = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/target_train/";
	const int numTrainSamples = 8;
	//const char* modelFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/ModelsModel.dat";
	const char* modelFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Models/Model.dat";
	const char* wavFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/source_train/1.wav";
	const char* convertedWavFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/test/jal_in_50_3_c.wav";
	// const char*  parentDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).cappendText("source_train/");
	// auto sourceAudioDir1 = parentDir.createDirectory();
	VoiceChanger_wczAudioProcessor& audioProcessor;

	juce::TextButton playButton;
	juce::TextButton stopButton;

	juce::AudioDeviceManager audioDeviceManager;
	juce::TextButton projectButton{ juce::CharPointer_UTF8("\xe5\xbc\x80\xe5\xa7\x8b\xe8\xae\xad\xe7\xbb\x83") };
	juce::TextButton loadSourceButton{ juce::CharPointer_UTF8("\xe5\x8e\x9f\xe5\xa3\xb0\xe9\x9f\xb3") };
	juce::TextButton loadTargetButton{ juce::CharPointer_UTF8("\xe6\x83\xb3\xe5\x8f\x98\xe6\x88\x90\xe7\x9a\x84\xe5\xa3\xb0\xe9\x9f\xb3") };
	

	std::unique_ptr<juce::FileChooser> chooser;

	juce::AudioFormatManager formatManager;

	juce::AudioThumbnailCache thumbnailCache;
	juce::AudioThumbnail sourceThumbnail;
	juce::AudioThumbnail targetThumbnail;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TemplateProjectingWindow)
};