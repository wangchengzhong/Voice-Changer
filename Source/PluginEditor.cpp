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
    AudioProcessorEditor::setSize(950, 600);
    // addAndMakeVisible(bkg);
    // addAndMakeVisible(playAudioFileComponent);
    AudioProcessorEditor::addAndMakeVisible(audioSetupComp);
    //juce::StandalonePluginHolder::getInstance()
    // audioSetupComp
    //// = juce::String("VB-Audio VoiceMeeter VAIO");
    

    Timer::startTimerHz(30);
    
    pPitchSlider.reset(new juce::Slider("PitchShiftSlider"));
    
    AudioProcessorEditor::addAndMakeVisible(pPitchSlider.get());
    pPitchSlider->setRange(-12, 12.0, 0.02);
    pPitchSlider->setTooltip(TRANS("higher"));
    pPitchSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pPitchSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 15);
    pPitchSlider->addListener(this);

    pPitchSlider->setBounds(20, 376, 80, 80);

    pPeakSlider.reset(new juce::Slider("PeakShiftSlider"));
    AudioProcessorEditor::addAndMakeVisible(pPeakSlider.get());
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
    AudioProcessorEditor::addAndMakeVisible(pFilterFreqSlider.get());

    pFilterFreqSlider->setBounds(30, 476, 80, 80);
    
    pFilterQFactorSlider.reset(new juce::Slider("FilterQFactorSlider"));
    pFilterQFactorSlider->setRange(0.01, 5, 0.01);
    pFilterQFactorSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pFilterQFactorSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pFilterQFactorSlider->addListener(this);
    pFilterQFactorSlider->setTooltip(TRANS("q"));
    AudioProcessorEditor::addAndMakeVisible(pFilterQFactorSlider.get());

    pFilterQFactorSlider->setBounds(100, 476, 80, 80);


    pFilterGainSlider.reset(new juce::Slider("FilterGainSlider"));
    pFilterGainSlider->setRange(-50, 0, 0.1);
    pFilterGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pFilterGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 15);
    pFilterGainSlider->addListener(this);
    pFilterGainSlider->setTooltip(TRANS("gain"));
    AudioProcessorEditor::addAndMakeVisible(pFilterGainSlider.get());

    pFilterGainSlider->setBounds(170, 476, 80, 80);

    pFilterTypeComboBox.reset(new juce::ComboBox("FilterTypeComboBox"));
    pFilterTypeComboBox->addItemList(audioProcessor.filterTypeItemsUI, 1);
    pFilterTypeComboBox->onChange = [this] {comboBoxChanged(pFilterTypeComboBox.get()); };
    pFilterTypeComboBox->setSelectedId(6);
    // pFilterTypeComboBox->addListener(this);
    AudioProcessorEditor::addAndMakeVisible(pFilterTypeComboBox.get());

    pFilterTypeComboBox->setBounds(180, 425, 130, 30);

    pFilterIndexComboBox.reset(new juce::ComboBox("FilterIndexComboBox"));
    pFilterIndexComboBox->addItemList(audioProcessor.filterIndex, 1);
    pFilterIndexComboBox->onChange = [this] {comboBoxChanged(pFilterIndexComboBox.get()); };
    // pFilterIndexComboBox->setSelectedId(1);
    // pFilterTypeComboBox->addListener(this);
    AudioProcessorEditor::addAndMakeVisible(pFilterIndexComboBox.get());

    pFilterIndexComboBox->setBounds(180, 380, 130, 30);
#endif

    pDynamicsThresholdSlider.reset(new juce::Slider("DynamicsThresholdSlider"));
    pDynamicsThresholdSlider->setRange(-50, 0, 0.001f);
    pDynamicsThresholdSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsThresholdSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsThresholdSlider->addListener(this);
    pDynamicsThresholdSlider->setTooltip(TRANS("dynamicsThreshold"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsThresholdSlider.get());

    pDynamicsThresholdSlider->setBounds(30, 276, 70, 70);

    pDynamicsRatioSlider.reset(new juce::Slider("DynamicsRatioSlider"));
    pDynamicsRatioSlider->setRange(1.0, 25.0, 0.01f);
    pDynamicsRatioSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsRatioSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsRatioSlider->addListener(this);
    pDynamicsRatioSlider->setTooltip(TRANS("dynamicsRatio"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsRatioSlider.get());

    pDynamicsRatioSlider->setBounds(85, 276, 70, 70);


    pDynamicsAttackSlider.reset(new juce::Slider("DynamicsAttackSlider"));
    pDynamicsAttackSlider->setRange(0.00001, 1.0f, 0.00001f);
    pDynamicsAttackSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsAttackSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsAttackSlider->addListener(this);
    pDynamicsAttackSlider->setTooltip(TRANS("dynamicsAttack"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsAttackSlider.get());

    pDynamicsAttackSlider->setBounds(140, 276, 70, 70);

    pDynamicsReleaseSlider.reset(new juce::Slider("DynamicsReleaseSlider"));
    pDynamicsReleaseSlider->setRange(0.001f, 2.0f, 0.001f);
    pDynamicsReleaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsReleaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsReleaseSlider->addListener(this);
    pDynamicsReleaseSlider->setTooltip(TRANS("dynamicsRelease"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsReleaseSlider.get());

    pDynamicsReleaseSlider->setBounds(195, 276, 70, 70);


    pDynamicsMakeupGainSlider.reset(new juce::Slider("DynamicsMakeupGainSlider"));
    pDynamicsMakeupGainSlider->setRange(-60.0f, 30.0f, 0.01f);
    pDynamicsMakeupGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pDynamicsMakeupGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 15);
    pDynamicsMakeupGainSlider->addListener(this);
    pDynamicsMakeupGainSlider->setTooltip(TRANS("dynamicsMakeupGain"));
    AudioProcessorEditor::addAndMakeVisible(pDynamicsMakeupGainSlider.get());

    pDynamicsMakeupGainSlider->setBounds(250, 276, 70, 70);

    mmButton.setButtonText(juce::CharPointer_UTF8("\xe5\xa6\xb9\xe5\xa6\xb9"));
    mmButton.onClick = [this] { mmButtonClicked(); };
    mmButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    AudioProcessorEditor::addAndMakeVisible(&mmButton);
    mmButton.setEnabled(true);


    xjjButton.setButtonText(juce::CharPointer_UTF8 ("\xe5\xb0\x8f\xe5\xa7\x90\xe5\xa7\x90"));
    xjjButton.onClick = [this] { xjjButtonClicked(); };
    xjjButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    AudioProcessorEditor::addAndMakeVisible(&xjjButton);
    xjjButton.setEnabled(true);

    ljButton.setButtonText("giegie");
    ljButton.onClick = [this] {ljButtonClicked(); };
    ljButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    AudioProcessorEditor::addAndMakeVisible(&ljButton);
    ljButton.setEnabled(true);


    xpyButton.setButtonText(juce::CharPointer_UTF8("\xe5\xb0\x8f\xe6\x9c\x8b\xe5\x8f\x8b"));
    xpyButton.onClick = [this] {xpyButtonClicked(); };
    xpyButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    AudioProcessorEditor::addAndMakeVisible(&xpyButton);
    xpyButton.setEnabled(true);



    openEffectButton.setButtonText(juce::CharPointer_UTF8("\xe5\xae\x9e\xe6\x97\xb6\xe6\xa8\xa1\xe5\xbc\x8f"));
    openEffectButton.onClick = [this] {openEffectButtonClicked(); };
    openEffectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkslateblue);
    AudioProcessorEditor::addAndMakeVisible(&openEffectButton);
    openEffectButton.setEnabled(true);


    closeEffectButton.setButtonText(juce::CharPointer_UTF8("\xe7\xa6\xbb\xe7\xba\xbf\xe6\xa8\xa1\xe5\xbc\x8f"));
    closeEffectButton.onClick = [this] {closeEffectButtonClicked(); };
    closeEffectButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    AudioProcessorEditor::addAndMakeVisible(&closeEffectButton);
    closeEffectButton.setEnabled(true);


    resetAllButton.setButtonText("RESET");
    resetAllButton.onClick = [this] {resetAllButtonClicked(); };
    resetAllButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    AudioProcessorEditor::addAndMakeVisible(&resetAllButton);
    resetAllButton.setEnabled(true);

    switchPitchMethodButton.setButtonText("FD Proc.");
    switchPitchMethodButton.onClick = [this] { switchPitchMethodButtonClicked(); };
    switchPitchMethodButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    switchPitchMethodButton.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::green);
    AudioProcessorEditor::addAndMakeVisible(&switchPitchMethodButton);
    switchPitchMethodButton.setEnabled(true);

    openFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe6\x9c\xac\xe5\x9c\xb0\xe6\x96\x87\xe4\xbb\xb6"));
    openFileButton.onClick = [this] { openFileButtonClicked(); };
    openFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::mediumseagreen);
    AudioProcessorEditor::addAndMakeVisible(&openFileButton);

    stopPlayFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x9a\x82\xe5\x81\x9c"));
    stopPlayFileButton.onClick = [this] { stopPlayFileButtonClicked(); };
    stopPlayFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    stopPlayFileButton.setEnabled(true);
    AudioProcessorEditor::addAndMakeVisible(&stopPlayFileButton);

    playFileButton.setButtonText(juce::CharPointer_UTF8("\xe6\x92\xad\xe6\x94\xbe"));
    playFileButton.onClick = [this] { playFileButtonClicked(); };
    playFileButton.setColour(juce::TextButton::buttonColourId, juce::Colours::hotpink);
    playFileButton.setEnabled(true);
    AudioProcessorEditor::addAndMakeVisible(&playFileButton);

    // pPlayPositionSlider.reset(new juce::Slider("PlayPositionSlider"));
    // pPlayPositionSlider->setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    // pPlayPositionSlider->setRange(0, duration, 0.01f);
    // pPlayPositionSlider->addListener(this);
    // addAndMakeVisible(pPlayPositionSlider.get());
    // pPlayPositionSlider->setBounds(545, 320, 370, 30);

    // audioProcessor.transportSource.addChangeListener(this);


    openTemplateWindowButton.setButtonText(juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x85\xa5\xe6\xa8\xa1\xe6\x9d\xbf"));
    openTemplateWindowButton.onClick = [this] { openTemplateWindowButtonClicked(); };
    openTemplateWindowButton.setColour(juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
    openTemplateWindowButton.setEnabled(true);
    AudioProcessorEditor::addAndMakeVisible(&openTemplateWindowButton);
    //recWindow = new TemplateRecordingWindow("TemplateRecording", juce::Colours::grey, juce::DocumentWindow::allButtons);
    //recWindow->setUsingNativeTitleBar(true);
    //recWindow->setSize(600, 600);
    //recWindow->setCentrePosition(600, 600);
    //recWindow->setContentOwned(new BackgroundComponent(), true);
    //
    //// recWindow->centreWithSize(500, 500);
    //recWindow->setVisible(true);
    
   //  AudioProcessorEditor::addAndMakeVisible(pPlayPositionSlider.get());
}
VoiceChanger_wczAudioProcessorEditor::~VoiceChanger_wczAudioProcessorEditor()
{
    if (templateRecordingWindow)
        delete[] templateRecordingWindow;
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
    // pPlayPositionSlider.get()->setValue(audioProcessor.nPlayAudioFilePosition / audioProcessor.nPlayAudioFileSampleNum);
#endif
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (AudioProcessorEditor::getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    //g.fillAll(juce::Colours::darkslategrey);
    int x = 12, y = 12, width = 512, height = 256;

    g.drawImage(audioProcessor.getSpectrumView(), x, y, width, height, 0, 0, width, height);
    // readFilePosition = audioProcessor.nPlayAudioFileSampleNum == 0 ? 0 : audioProcessor.nPlayAudioFilePosition / audioProcessor.nPlayAudioFileSampleNum;
    // pPlayPositionSlider.get()->setValue(readFilePosition);
}
bool operator!=(const TransportInformation A, const TransportInformation B)
{
    if (A.state != B.state)
        return true;
    if (A.transportType != B.transportType)
        return true;
    if (A.audioFilePlayPoint != B.audioFilePlayPoint)
        return true;
    return false;
}
void VoiceChanger_wczAudioProcessorEditor::timerCallback()
{
    AudioProcessorEditor::repaint();
}

void VoiceChanger_wczAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    xjjButton.setBounds(340, 340, AudioProcessorEditor::getWidth() / 6, 30);
    xpyButton.setBounds(340, 390, AudioProcessorEditor::getWidth() / 6, 30);
    ljButton.setBounds(340, 440, AudioProcessorEditor::getWidth() / 6, 30);
    mmButton.setBounds(340, 490, AudioProcessorEditor::getWidth() / 6, 30);
    openEffectButton.setBounds(340, 540, (AudioProcessorEditor::getWidth()) / 12, 30);
    closeEffectButton.setBounds(340 + (AudioProcessorEditor::getWidth()) / 12, 540, (AudioProcessorEditor::getWidth()) / 12, 30);
    resetAllButton.setBounds(340, 290, (AudioProcessorEditor::getWidth()/6) , 30);
    switchPitchMethodButton.setBounds(245, 510, AudioProcessorEditor::getWidth() / 8, 20);

    openFileButton.setBounds(570, 380, AudioProcessorEditor::getWidth() / 5, 40);
    playFileButton.setBounds(570, 450, AudioProcessorEditor::getWidth() / 5, 40);
    stopPlayFileButton.setBounds(570, 520, AudioProcessorEditor::getWidth() / 5, 40);
    
    openTemplateWindowButton.setBounds(800, 400, AudioProcessorEditor::getWidth() / 8, 150);

    audioSetupComp.setBounds(545, 20, 400, 100);
    //playAudioFileComponent.setBounds(AudioProcessorEditor::getLocalBounds());

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
    //else if (sliderThatWasMoved == pPlayPositionSlider.get())
    {
        if(&juce::MouseEvent::mouseWasClicked)
        // audioProcessor.transportSource.setPosition(pPlayPositionSlider.get()->getValue());
        //if (shouldUpdatePosition)
        {
            //audioProcessor.nPlayAudioFilePosition = (int)(pPlayPositionSlider.get()->getValue() * audioProcessor.nPlayAudioFileSampleNum);
            // shouldUpdatePosition = false;
        }
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




void VoiceChanger_wczAudioProcessorEditor::openFileButtonClicked()
{
    
    juce::FileChooser chooser("choose a WAV or AIFF file",juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3; *.flac");
    //auto chooserFlags = juce::FileBrowserComponent::openMode
    //    | juce::FileBrowserComponent::canSelectFiles;
    if(chooser.browseForFileToOpen())
    //chooser.launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        //auto file = fc.getResult();
        auto file = chooser.getResult();
        if (file == juce::File{})
            return;
        std::unique_ptr<juce::AudioFormatReader>reader(audioProcessor.formatManager.createReaderFor(file));
        if (reader.get() != nullptr)
        {
            
            //transportInfo.pFileBufferPointingAt = &audioProcessor.fileBuffer;
            duration = (float)reader->lengthInSamples / reader->sampleRate;
            //pPlayPositionSlider->setRange(0, duration, 0.01f);
            // playAudioFileComponent.pPlayPositionSlider->setRange(0, duration, 0.01f);
            audioProcessor.nPlayAudioFileSampleNum = reader->sampleRate;
            if (duration < 1000)
            {
                audioProcessor.setState(Stopping);
                audioProcessor.setTarget(Mainpage);
                audioProcessor.fileBuffer.clear();
                audioProcessor.fileBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                reader->read(
                    &audioProcessor.fileBuffer,
                    0,
                    (int)reader->lengthInSamples,
                    0,
                    true,
                    true
                );
                audioProcessor.nPlayAudioFilePosition = 0;
                //readFilePosition = 0;
                audioProcessor.canReadSampleBuffer = true;
            }
        }
    }
    //);
}
void VoiceChanger_wczAudioProcessorEditor::stopPlayFileButtonClicked()
{
    //ti.setState(Stopping);
    audioProcessor.setState(Stopping);
    // changeState(Stopping);
}
void VoiceChanger_wczAudioProcessorEditor::playFileButtonClicked()
{
    //ti.setState(Starting);
    audioProcessor.setState(Starting);
    // changeState(Starting);
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


void VoiceChanger_wczAudioProcessorEditor::openTemplateWindowButtonClicked()
{
    if (templateRecordingWindow)
    {
        templateRecordingWindow->broughtToFront();
    }
    else
    {
        templateRecordingWindow = new NewWindow(juce::String("templateRecordingWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);// new TemplateRecordingWindow();
        templateRecordingWindow->setContentOwned(new TemplateRecordingWindow(audioProcessor), true);
        templateRecordingWindow->addToDesktop();
        templateRecordingWindow->centreWithSize(600, 300);
        templateRecordingWindow->setVisible(true);
        // templateRecordingWindow->setName(juce::CharPointer_UTF8("\xe6\xa8\xa1\xe6\x9d\xbf"));
    }
}