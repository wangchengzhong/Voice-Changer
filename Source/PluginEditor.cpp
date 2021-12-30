/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
VoiceChanger_wczAudioProcessorEditor::VoiceChanger_wczAudioProcessorEditor(VoiceChanger_wczAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
    , audioSetupComp(juce::StandalonePluginHolder::getInstance()->deviceManager,0,4,0,4,false,false,false,false)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //addAndMakeVisible(audioSetupComp);
    auto pluginHolder = juce::StandalonePluginHolder::getInstance();
    juce::AudioDeviceManager::AudioDeviceSetup anotherSetup = pluginHolder->deviceManager.getAudioDeviceSetup();
    // DBG(audioSetupComp.deviceManager.getAudioDeviceSetup().outputDeviceName);
    anotherSetup.inputDeviceName = juce::String("VoiceMeeter Output (VB-Audio VoiceMeeter VAIO)");
    anotherSetup.outputDeviceName = juce::CharPointer_UTF8("\xe8\x80\xb3\xe6\x9c\xba (AirPods)");
    anotherSetup.sampleRate = 44100;
    audioSetupComp.deviceManager.setAudioDeviceSetup(anotherSetup, false);
    setSize(950, 600);
    addAndMakeVisible(bkg);
    addAndMakeVisible(audioSetupComp);
    //juce::StandalonePluginHolder::getInstance()
    // audioSetupComp
    //// = juce::String("VB-Audio VoiceMeeter VAIO");
    


    

    startTimerHz(30);
    pPitchSlider.reset(new juce::Slider("PitchShiftSlider"));
    
    addAndMakeVisible(pPitchSlider.get());
    pPitchSlider->setRange(-12, 12.0, 0.02);
    pPitchSlider->setTooltip(TRANS("higher"));
    pPitchSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pPitchSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    pPitchSlider->addListener(this);

    pPitchSlider->setBounds(20, 376, 80, 80);

    pPeakSlider.reset(new juce::Slider("PeakShiftSlider"));
    addAndMakeVisible(pPeakSlider.get());
    pPeakSlider->setRange(0.5, 2.0, 0.02);
    pPeakSlider->setTooltip(TRANS("peaker"));
    
    pPeakSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pPeakSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pPeakSlider->addListener(this);

    pPeakSlider->setBounds(90, 376, 80, 80);


#if _OPEN_FILTERS
    pFilterFreqSlider.reset(new juce::Slider("FilterFreqSlider"));
    pFilterFreqSlider->setRange(80, 8000, 1);
    pFilterFreqSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pFilterFreqSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pFilterFreqSlider->addListener(this);
    pFilterFreqSlider->setTooltip(TRANS("freq"));
    addAndMakeVisible(pFilterFreqSlider.get());

    pFilterFreqSlider->setBounds(30, 476, 80, 80);
    
    pFilterQFactorSlider.reset(new juce::Slider("FilterQFactorSlider"));
    pFilterQFactorSlider->setRange(0.01, 5, 0.01);
    pFilterQFactorSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pFilterQFactorSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pFilterQFactorSlider->addListener(this);
    pFilterQFactorSlider->setTooltip(TRANS("q"));
    addAndMakeVisible(pFilterQFactorSlider.get());

    pFilterQFactorSlider->setBounds(100, 476, 80, 80);


    pFilterGainSlider.reset(new juce::Slider("FilterGainSlider"));
    pFilterGainSlider->setRange(-50, 0, 0.1);
    pFilterGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pFilterGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pFilterGainSlider->addListener(this);
    pFilterGainSlider->setTooltip(TRANS("gain"));
    addAndMakeVisible(pFilterGainSlider.get());

    pFilterGainSlider->setBounds(170, 476, 80, 80);

    //pFilterTypeSlider.reset(new juce::Slider("FilterTypeSlider"));
    //pFilterTypeSlider->setRange(0, 7, 1);
    //pFilterTypeSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    //pFilterTypeSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    //pFilterTypeSlider->addListener(this);
    //pFilterTypeSlider->setTooltip(TRANS("filterType"));
    //addAndMakeVisible(pFilterTypeSlider.get());


    pFilterTypeComboBox.reset(new juce::ComboBox("FilterTypeComboBox"));
    pFilterTypeComboBox->addItemList(audioProcessor.filterTypeItemsUI, 1);
    pFilterTypeComboBox->onChange = [this] {comboBoxChanged(pFilterTypeComboBox.get()); };
    pFilterTypeComboBox->setSelectedId(6);
    // pFilterTypeComboBox->addListener(this);
    addAndMakeVisible(pFilterTypeComboBox.get());

    pFilterTypeComboBox->setBounds(180, 425, 130, 30);


    pFilterIndexComboBox.reset(new juce::ComboBox("FilterIndexComboBox"));
    pFilterIndexComboBox->addItemList(audioProcessor.filterIndex, 1);
    pFilterIndexComboBox->onChange = [this] {comboBoxChanged(pFilterIndexComboBox.get()); };
    // pFilterIndexComboBox->setSelectedId(1);
    // pFilterTypeComboBox->addListener(this);
    addAndMakeVisible(pFilterIndexComboBox.get());

    pFilterIndexComboBox->setBounds(180, 380, 130, 30);
#endif

    pDynamicsThresholdSlider.reset(new juce::Slider("DynamicsThresholdSlider"));
    pDynamicsThresholdSlider->setRange(-50, 0, 0.001f);
    pDynamicsThresholdSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsThresholdSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsThresholdSlider->addListener(this);
    pDynamicsThresholdSlider->setTooltip(TRANS("dynamicsThreshold"));
    addAndMakeVisible(pDynamicsThresholdSlider.get());

    pDynamicsThresholdSlider->setBounds(30, 276, 70, 70);

    pDynamicsRatioSlider.reset(new juce::Slider("DynamicsRatioSlider"));
    pDynamicsRatioSlider->setRange(1.0, 25.0, 0.01f);
    pDynamicsRatioSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsRatioSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsRatioSlider->addListener(this);
    pDynamicsRatioSlider->setTooltip(TRANS("dynamicsRatio"));
    addAndMakeVisible(pDynamicsRatioSlider.get());

    pDynamicsRatioSlider->setBounds(85, 276, 70, 70);


    pDynamicsAttackSlider.reset(new juce::Slider("DynamicsAttackSlider"));
    pDynamicsAttackSlider->setRange(0.00001, 1.0f, 0.00001f);
    pDynamicsAttackSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsAttackSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsAttackSlider->addListener(this);
    pDynamicsAttackSlider->setTooltip(TRANS("dynamicsAttack"));
    addAndMakeVisible(pDynamicsAttackSlider.get());

    pDynamicsAttackSlider->setBounds(140, 276, 70, 70);

    pDynamicsReleaseSlider.reset(new juce::Slider("DynamicsReleaseSlider"));
    pDynamicsReleaseSlider->setRange(0.001f, 2.0f, 0.001f);
    pDynamicsReleaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsReleaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsReleaseSlider->addListener(this);
    pDynamicsReleaseSlider->setTooltip(TRANS("dynamicsRelease"));
    addAndMakeVisible(pDynamicsReleaseSlider.get());

    pDynamicsReleaseSlider->setBounds(195, 276, 70, 70);


    pDynamicsMakeupGainSlider.reset(new juce::Slider("DynamicsMakeupGainSlider"));
    pDynamicsMakeupGainSlider->setRange(-5.0f, 30.0f, 0.01f);
    pDynamicsMakeupGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsMakeupGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsMakeupGainSlider->addListener(this);
    pDynamicsMakeupGainSlider->setTooltip(TRANS("dynamicsMakeupGain"));
    addAndMakeVisible(pDynamicsMakeupGainSlider.get());

    pDynamicsMakeupGainSlider->setBounds(250, 276, 70, 70);

    mmButton.setButtonText(juce::CharPointer_UTF8("\xe5\xa6\xb9\xe5\xa6\xb9"));
    mmButton.onClick = [this] { mmButtonClicked(); };
    mmButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    addAndMakeVisible(&mmButton);
    mmButton.setEnabled(true);


    xjjButton.setButtonText(juce::CharPointer_UTF8 ("\xe5\xb0\x8f\xe5\xa7\x90\xe5\xa7\x90"));
    xjjButton.onClick = [this] { xjjButtonClicked(); };
    xjjButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    addAndMakeVisible(&xjjButton);
    xjjButton.setEnabled(true);

    ljButton.setButtonText("giegie");
    ljButton.onClick = [this] {ljButtonClicked(); };
    ljButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    addAndMakeVisible(&ljButton);
    ljButton.setEnabled(true);


    xpyButton.setButtonText(juce::CharPointer_UTF8("\xe5\xb0\x8f\xe6\x9c\x8b\xe5\x8f\x8b"));
    xpyButton.onClick = [this] {xpyButtonClicked(); };
    xpyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    addAndMakeVisible(&xpyButton);
    xpyButton.setEnabled(true);



    openEffectButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80"));
    openEffectButton.onClick = [this] {openEffectButtonClicked(); };
    openEffectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkslateblue);
    addAndMakeVisible(&openEffectButton);
    openEffectButton.setEnabled(true);


    closeEffectButton.setButtonText(juce::CharPointer_UTF8("\xe5\x85\xb3\xe9\x97\xad"));
    closeEffectButton.onClick = [this] {closeEffectButtonClicked(); };
    closeEffectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    addAndMakeVisible(&closeEffectButton);
    closeEffectButton.setEnabled(true);


    resetAllButton.setButtonText("RESET");
    resetAllButton.onClick = [this] {resetAllButtonClicked(); };
    resetAllButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    addAndMakeVisible(&resetAllButton);
    resetAllButton.setEnabled(true);

    switchPitchMethodButton.setButtonText("FD Proc.");
    switchPitchMethodButton.onClick = [this] { switchPitchMethodButtonClicked(); };
    switchPitchMethodButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    switchPitchMethodButton.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::green);
    addAndMakeVisible(&switchPitchMethodButton);
    switchPitchMethodButton.setEnabled(true);

    openFileButton.setButtonText("Open");
    openFileButton.onClick = [this] { openFileButtonClicked(); };
    openFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    addAndMakeVisible(&openFileButton);

    stopPlayFileButton.setButtonText("STOP");
    stopPlayFileButton.onClick = [this] { stopPlayFileButtonClicked(); };
    stopPlayFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    stopPlayFileButton.setEnabled(false);
    addAndMakeVisible(&stopPlayFileButton);

    playFileButton.setButtonText("Play");
    playFileButton.onClick = [this] { playFileButtonClicked(); };
    playFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::hotpink);
    playFileButton.setEnabled(true);
    addAndMakeVisible(&playFileButton);

    audioProcessor.transportSource.addChangeListener(this);
}
VoiceChanger_wczAudioProcessorEditor::~VoiceChanger_wczAudioProcessorEditor()
{
}

//==============================================================================
void VoiceChanger_wczAudioProcessorEditor::paint (juce::Graphics& g)
{
    pPitchSlider.get()->setValue(audioProcessor.getPitchShift());
    pPeakSlider.get()->setValue(audioProcessor.getPeakShift());
#if _OPEN_FILTERS
    pFilterFreqSlider.get()->setValue(audioProcessor.getFilterFreqShift(audioProcessor.currentFilterIndex));
    pFilterQFactorSlider.get()->setValue(audioProcessor.getFilterQFactorShift(audioProcessor.currentFilterIndex));

    pFilterTypeComboBox.get()->setSelectedId(audioProcessor.getFilterTypeShift(audioProcessor.currentFilterIndex));
    // pFilterTypeComboBox.get()->setComponentID()
    pFilterGainSlider.get()->setValue(audioProcessor.getFilterGainShift(audioProcessor.currentFilterIndex));
#endif
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    //g.fillAll(juce::Colours::darkslategrey);
    int x = 12, y = 12, width = 512, height = 256;

    g.drawImage(audioProcessor.getSpectrumView(), x, y, width, height, 0, 0, width, height);
}
void VoiceChanger_wczAudioProcessorEditor::timerCallback()
{
    repaint();
}
void VoiceChanger_wczAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    xjjButton.setBounds(340, 340, getWidth() / 6, 30);
    xpyButton.setBounds(340, 390, getWidth() / 6, 30);
    ljButton.setBounds(340, 440, getWidth() / 6, 30);
    mmButton.setBounds(340, 490, getWidth() / 6, 30);
    openEffectButton.setBounds(340, 540, (getWidth()) / 12, 30);
    closeEffectButton.setBounds(340 + (getWidth()) / 12, 540, (getWidth()) / 12, 30);
    resetAllButton.setBounds(340, 290, (getWidth()/6) , 30);
    switchPitchMethodButton.setBounds(245, 510, getWidth() / 8, 20);

    openFileButton.setBounds(570, 360, getWidth() / 5, 40);
    playFileButton.setBounds(570, 430, getWidth() / 5, 40);
    stopPlayFileButton.setBounds(570, 500, getWidth() / 5, 40);

    audioSetupComp.setBounds(545, 20, 400, 100);
    bkg.setBounds(getLocalBounds());

}
void VoiceChanger_wczAudioProcessorEditor::sliderValueChanged(juce::Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == pPitchSlider.get())
    {
        audioProcessor.setPitchShift((float)pPitchSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pPeakSlider.get())
    {
        audioProcessor.setPeakShift((float)pPeakSlider.get()->getValue());
    }
#if _OPEN_FILTERS
    else if (sliderThatWasMoved == pFilterFreqSlider.get())
    {
        audioProcessor.setFilterFreqShift((float)pFilterFreqSlider.get()->getValue(), audioProcessor.currentFilterIndex);
    }
    else if (sliderThatWasMoved == pFilterQFactorSlider.get())
    {
        audioProcessor.setFilterQFactorShift((float)pFilterQFactorSlider.get()->getValue(), audioProcessor.currentFilterIndex);
    }
    else if (sliderThatWasMoved == pFilterGainSlider.get())
    {
        audioProcessor.setFilterGainShift((float)pFilterGainSlider.get()->getValue(), audioProcessor.currentFilterIndex);
    }

#endif
    else if (sliderThatWasMoved == pDynamicsThresholdSlider.get())
    {
        audioProcessor.setDynamicsThresholdShift((float)pDynamicsThresholdSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsRatioSlider.get())
    {
        audioProcessor.setDynamicsRatioShift((float)pDynamicsRatioSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsAttackSlider.get())
    {
        audioProcessor.setDynamicsAttackShift((float)pDynamicsAttackSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsReleaseSlider.get())
    {
        audioProcessor.setDynamicsReleaseShift((float)pDynamicsReleaseSlider.get()->getValue());
    }
    else if (sliderThatWasMoved == pDynamicsMakeupGainSlider.get())
    {
        audioProcessor.setDynamicsMakeupGainShift((float)pDynamicsMakeupGainSlider.get()->getValue());
    }
}



void VoiceChanger_wczAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatWasMoved)
{
#if _OPEN_FILTERS
    if (comboBoxThatWasMoved == pFilterTypeComboBox.get())
    {
        audioProcessor.setFilterTypeShift((int)pFilterTypeComboBox.get()->getSelectedId(), audioProcessor.currentFilterIndex);
    }
    else if (comboBoxThatWasMoved == pFilterIndexComboBox.get())
    {
        audioProcessor.currentFilterIndex = round(pFilterIndexComboBox.get()->getSelectedId());
    }
#endif
}




void VoiceChanger_wczAudioProcessorEditor::ljButtonClicked()
{
    audioProcessor.setPitchShift(-5);
    // audioProcessor.updateBackendFilterControls();
}
void VoiceChanger_wczAudioProcessorEditor::xjjButtonClicked()
{
    audioProcessor.setPitchShift(2);
}
void VoiceChanger_wczAudioProcessorEditor::mmButtonClicked()
{
    audioProcessor.setPitchShift(4);
}
void VoiceChanger_wczAudioProcessorEditor::xpyButtonClicked()
{
    audioProcessor.setPitchShift(6);
}


void VoiceChanger_wczAudioProcessorEditor::changeState(TransportState newState)
{
    if (newState != state)
    {
        state = newState;
        switch (state)
        {
        case Stopped:
            stopPlayFileButton.setEnabled(false);
            playFileButton.setEnabled(true);
            // audioProcessor.transportSource.setPosition(0.0);
            break;
        case Playing:
            stopPlayFileButton.setEnabled(true);
            break;
        case Starting:
            stopPlayFileButton.setEnabled(true);
            playFileButton.setEnabled(true);
            audioProcessor.transportSource.start();
            break;
        case Stopping:
            playFileButton.setEnabled(true);
            stopPlayFileButton.setEnabled(false);
            audioProcessor.transportSource.stop();
            break;
        default:
            break;
        }
    }
}

void VoiceChanger_wczAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor.transportSource)
    {
        if (audioProcessor.transportSource.isPlaying())
        {
            changeState(Playing);
        }
        else
        {
            changeState(Stopped);
        }
    }
}


void VoiceChanger_wczAudioProcessorEditor::openFileButtonClicked()
{
    juce::FileChooser chooser("choose a WAV or AIFF file",juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3");
    if (chooser.browseForFileToOpen())
    {
        juce::File myFile;
        myFile = chooser.getResult();
        juce::AudioFormatReader* reader = audioProcessor.formatManager.createReaderFor(myFile);

        if (reader != nullptr)
        {
            playFileButton.setEnabled(true);
            std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

            audioProcessor.transportSource.setSource(tempSource.get(), 0, nullptr, reader->sampleRate);
            audioProcessor.readerSource.reset(tempSource.release());
        }
    }
}
void VoiceChanger_wczAudioProcessorEditor::stopPlayFileButtonClicked()
{
    changeState(Stopping);
}
void VoiceChanger_wczAudioProcessorEditor::playFileButtonClicked()
{
    changeState(Starting);
}



void VoiceChanger_wczAudioProcessorEditor::openEffectButtonClicked()
{
    audioProcessor.realtimeMode = true;

}
void VoiceChanger_wczAudioProcessorEditor::closeEffectButtonClicked()
{
    audioProcessor.realtimeMode = false;
}

void VoiceChanger_wczAudioProcessorEditor::resetAllButtonClicked()
{
    // changeState(close);
    pPitchSlider->setValue(0.0);
    pPeakSlider->setValue(1.0);
#if _OPEN_FILTERS
    pFilterFreqSlider->setValue(800.0);
    pFilterQFactorSlider->setValue(1.0);
    pFilterGainSlider->setValue(0.0);
#endif
    // pFilterTypeSlider->setValue(6);
    // pFilterTypeComboBox->setComponentID("0");
}
void VoiceChanger_wczAudioProcessorEditor::switchPitchMethodButtonClicked()
{
    if (audioProcessor.useFD)
        audioProcessor.useFD = false;
    else
        audioProcessor.useFD = true;
}
