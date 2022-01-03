//#pragma once
//
//#include"JuceHeader.h"
//
//class PlayAudioFileComponent :public juce::Component, 
//	private juce::Timer,
//	public juce::ChangeListener,
//	public juce::FileDragAndDropTarget,
//	public juce::ChangeBroadcaster,
//	public juce::ScrollBar::Listener
//{
//public:
//	PlayAudioFileComponent(juce::AudioFormatManager& formatManager,
//		juce::AudioTransportSource& source,
//		juce::Slider& slider)
//	{
//
//	}
//	~PlayAudioFileComponent() override
//	{
//
//	}
//
//	void paint(juce::Graphics&) override;
//	void resized() override;
//	void timerCallback() override;
//	// void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
//
//
//	std::unique_ptr<juce::Slider> pPlayPositionSlider;
//	int duration{ 300 };
//	float readFilePosition{ 0 };
//	bool shouldUpdatePosition{ true };
//
//private:
//	juce::AudioDeviceManager audioDeviceManager;
//	juce::AudioFormatManager formatManager;
//	juce::TimeSliceThread thread { "audio file preview" };
//	juce::DirectoryContentsList directoryList{ nullptr,thread };
//	juce::FileTreeComponent fileTreeComp{ directoryList };
//	juce::Label explanation{ {},"play" };
//	juce::URL currentAudioFile;
//	juce::AudioSourcePlayer audioSourcePlayer;
//	juce::AudioTransportSource transportSource;
//	
//	std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;
//
//	std::unique_ptr<PlayAudioFileComponent> thumbnail;
//
//
//	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayAudioFileComponent);
//};
