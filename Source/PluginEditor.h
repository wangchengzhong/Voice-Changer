/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include"BackgroundComponent.h"
#include"TemplateRecordingWindow.h"
#include"NewWindow.h"
//==============================================================================
/**
*/
class VoiceChanger_wczAudioProcessorEditor  : public juce::AudioProcessorEditor
    ,public juce::Slider::Listener,public juce::ComboBox::Listener, private juce::Timer
    , public juce::ChangeListener
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
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    VoiceChanger_wczAudioProcessor& audioProcessor;

    void mmButtonClicked();
    void xjjButtonClicked();
    void ljButtonClicked();
    void xpyButtonClicked();
    void openEffectButtonClicked();
    void closeEffectButtonClicked();
    void resetAllButtonClicked();
    void switchPitchMethodButtonClicked();

    void stopPlayFileButtonClicked();
    void playFileButtonClicked();
    void openFileButtonClicked();
    void openTemplateWindowButtonClicked();
    // void singModeClicked();


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
    juce::TextButton openEffectButton;
    juce::TextButton closeEffectButton;

    juce::TextButton openFileButton;
    juce::TextButton playFileButton;
    juce::TextButton stopPlayFileButton;
    std::unique_ptr<juce::Slider> pPlayPositionSlider;

    juce::TextButton openTemplateWindowButton;

    juce::TextButton resetAllButton;

    juce::ToggleButton switchPitchMethodButton;

    juce::AudioDeviceSelectorComponent audioSetupComp;

    BackgroundComponent bkg;
    
    void changeState(TransportState newState);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    TransportState state;
    
    // TemplateRecordingWindow* recWindow;
    juce::Component::SafePointer<juce::DocumentWindow> templateRecordingWindow;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceChanger_wczAudioProcessorEditor)
};
