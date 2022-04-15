/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include"TemplateRecordingWindow.h"
#include"NewWindow.h"
#include"TransportInformation.h"
#include"CameraWindow.h"
#include"CircularMeter.h"
#include"HorizontalMeter.h"
#include"EqualizerEditor.h"
#include"DawComponent.h"
#include"AudioThumbnailCore.h"
#include"CustomLookAndFeel.h"
//==============================================================================
/**
*/
class VoiceChanger_wczAudioProcessorEditor
    : public juce::AudioProcessorEditor
    ,public juce::Slider::Listener
    ,public juce::ComboBox::Listener
    , private juce::Timer
	, private  juce::ChangeListener
    // , public juce::ChangeListener
    //,public juce::ComponentListener
     //,
    //,public juce::AudioAppComponent
    //,private juce::Timer
{
public:
    VoiceChanger_wczAudioProcessorEditor (VoiceChanger_wczAudioProcessor&);
    ~VoiceChanger_wczAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved) override;
    void timerCallback() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatWasMoved) override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    // int currentFilterIndex{ audioProcessor.currentFilterIndex };

    VoiceChanger_wczAudioProcessor& audioProcessor;

    void mmButtonClicked();
    void xjjButtonClicked();
    void ljButtonClicked();
    void xpyButtonClicked();
    void realtimeButtonClicked();
    void offlineButtonClicked();
    void resetAllButtonClicked();
    void switchPitchMethodButtonClicked();
    void switchVoiceConversionButtonClicked();

    void stopPlayFileButtonClicked();
    void playFileButtonClicked();
    void openFileButtonClicked();
    void openTemplateWindowButtonClicked();
    void openCameraButtonClicked();
    void openDawButtonClicked();
    void openEqButtonClicked();
    // void singModeClicked();

    juce::Image backgroundTexture = juce::ImageFileFormat::
        loadFrom(BinaryData::background_texture_dark_headline_jpg,
            BinaryData::background_texture_dark_headline_jpgSize);

    OtherLookAndFeel otherLookAndFeel;
    std::unique_ptr<juce::Slider> pPitchSlider;
    std::unique_ptr<juce::Slider> pPeakSlider;
#if _OPEN_FILTERS
    std::unique_ptr<juce::Slider> pFilterFreqSlider;
    std::unique_ptr<juce::Slider> pFilterQFactorSlider;
    std::unique_ptr<juce::Slider> pFilterGainSlider;
    //std::unique_ptr<juce::Slider> pFilterTypeSlider;

    std::unique_ptr<juce::ComboBox> pFilterTypeComboBox;
    std::unique_ptr<juce::ComboBox> pFilterIndexComboBox;
#endif
    std::unique_ptr<juce::Slider> pDynamicsThresholdSlider;
    std::unique_ptr<juce::Slider> pDynamicsRatioSlider;
    std::unique_ptr<juce::Slider> pDynamicsAttackSlider;
    std::unique_ptr<juce::Slider> pDynamicsReleaseSlider;
    std::unique_ptr<juce::Slider> pDynamicsMakeupGainSlider;



    juce::TextButton mmButton;
    juce::TextButton xjjButton;
    juce::TextButton ljButton;
    juce::TextButton xpyButton;
    juce::TextButton realtimeButton;
    juce::TextButton offlineButton;

    juce::TextButton openFileButton;
    juce::TextButton playFileButton;
    juce::TextButton stopPlayFileButton;
    // std::unique_ptr<juce::Slider> pPlayPositionSlider;

    

    juce::TextButton resetAllButton;

    juce::ImageButton switchPitchMethodButton;
    juce::ImageButton switchVoiceConversionButton;
    juce::Label timeDomainLabel;
    juce::Label freqDomainLabel;
    juce::Label specificConversionLabel;
    juce::Label generalConversionLabel;

    juce::TextButton openTemplateWindowButton;
    juce::TextButton openCameraButton;
    juce::TextButton openInnerRecordingButton;
    juce::TextButton closeInnerRecordingButton;


    juce::TextButton openDawButton;

    juce::TextButton openEqButton;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    int duration{ 300 };
   


    
    // TransportState state;
    // juce::AudioSampleBuffer*& pData{ audioProcessor.pPlayBuffer };
    // TemplateRecordingWindow* recWindow;
    juce::Component::SafePointer<juce::DocumentWindow> templateRecordingWindow;
    juce::Component::SafePointer<juce::DocumentWindow> cameraWindow;
    juce::Component::SafePointer<juce::DocumentWindow> dawWindow;
    juce::Component::SafePointer<juce::DocumentWindow> eqWindow;
    std::unique_ptr<FrequalizerAudioProcessorEditor> pEqEditor;

    Gui::CicularMeter circularMeterL, circularMeterR;
    Gui::HorizontalMeter horizontalMeterL, horizontalMeterR;
    ScopedPointer<AudioThumbnailComp> thumbnailCore;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	int framesElapsed = 0;
    float maxRmsLeft{}, maxRmsRight{};

public:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChanger_wczAudioProcessorEditor)
};
