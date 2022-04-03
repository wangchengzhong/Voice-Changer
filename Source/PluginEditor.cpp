/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EqualizerEditor.h"
//==============================================================================
VoiceChanger_wczAudioProcessorEditor::VoiceChanger_wczAudioProcessorEditor(VoiceChanger_wczAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
    , audioSetupComp(juce::StandalonePluginHolder::getInstance()->deviceManager,0,4,0,4,false,false,false,false)
    , circularMeterL([&]() { return audioProcessor.getRmsLevel(0); },juce::Colours::darkcyan)
    , circularMeterR([&]() { return audioProcessor.getRmsLevel(1); },juce::Colours::violet)
	, pEqEditor(std::make_unique<FrequalizerAudioProcessorEditor>(p))
{
    thumbnailCore = new AudioThumbnailComp(audioProcessor.formatManager, audioProcessor.transportSource, audioProcessor.thumbnailCache, audioProcessor.currentlyLoadedFile);
    addAndMakeVisible(thumbnailCore);
    thumbnailCore->addChangeListener(this);

    addAndMakeVisible(circularMeterL);
    addAndMakeVisible(circularMeterR);
    addAndMakeVisible(horizontalMeterL);
    addAndMakeVisible(horizontalMeterR);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //addAndMakeVisible(audioSetupComp);

    auto pluginHolder = juce::StandalonePluginHolder::getInstance();
    juce::AudioDeviceManager::AudioDeviceSetup anotherSetup = pluginHolder->deviceManager.getAudioDeviceSetup();
    //// DBG(audioSetupComp.deviceManager.getAudioDeviceSetup().outputDeviceName);
    //anotherSetup.inputDeviceName = juce::String("VoiceMeeter Output (VB-Audio VoiceMeeter VAIO)");
    //anotherSetup.outputDeviceName = juce::CharPointer_UTF8("\xe8\x80\xb3\xe6\x9c\xba (AirPods)");
    anotherSetup.sampleRate = 44100;
    //audioSetupComp.deviceManager.setAudioDeviceSetup(anotherSetup, false);
    setSize(1400, 600);
    //// addAndMakeVisible(bkg);
    //// addAndMakeVisible(playAudioFileComponent);
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



    realtimeButton.setButtonText(juce::CharPointer_UTF8("\xe5\xae\x9e\xe6\x97\xb6\xe6\xa8\xa1\xe5\xbc\x8f"));
    realtimeButton.onClick = [this] {realtimeButtonClicked(); };
    realtimeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkslateblue);
    AudioProcessorEditor::addAndMakeVisible(&realtimeButton);
    realtimeButton.setEnabled(true);


    offlineButton.setButtonText(juce::CharPointer_UTF8("\xe7\xa6\xbb\xe7\xba\xbf\xe6\xa8\xa1\xe5\xbc\x8f"));
    offlineButton.onClick = [this] {offlineButtonClicked(); };
    offlineButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    AudioProcessorEditor::addAndMakeVisible(&offlineButton);
    offlineButton.setEnabled(true);


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


    switchVoiceConversionButton.setButtonText("V.C.");
    switchVoiceConversionButton.onClick = [this] {switchVoiceConversionButtonClicked(); };
    switchVoiceConversionButton.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    switchVoiceConversionButton.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::green);
    addAndMakeVisible(switchVoiceConversionButton);
    switchVoiceConversionButton.setEnabled(true);



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

    openCameraButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe6\x91\x84\xe5\x83\x8f\xe5\xa4\xb4"));
    openCameraButton.onClick = [this] { openCameraButtonClicked(); };
    openCameraButton.setColour(juce::TextButton::buttonColourId, juce::Colours::yellowgreen);
    openCameraButton.setEnabled(true);
    addAndMakeVisible(&openCameraButton);
    //recWindow = new TemplateRecordingWindow("TemplateRecording", juce::Colours::grey, juce::DocumentWindow::allButtons);
    //recWindow->setUsingNativeTitleBar(true);
    //recWindow->setSize(600, 600);
    //recWindow->setCentrePosition(600, 600);
    //recWindow->setContentOwned(new BackgroundComponent(), true);
    //
    //// recWindow->centreWithSize(500, 500);
    //recWindow->setVisible(true);
    
    //  AudioProcessorEditor::addAndMakeVisible(pPlayPositionSlider.get());
    openDawButton.setButtonText(juce::CharPointer_UTF8("\xe6\x89\x93\xe5\xbc\x80\xe9\x9f\xb3\xe9\xa2\x91\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8"));
    openDawButton.onClick = [this] {openDawButtonClicked(); };
    openDawButton.setColour(juce::TextButton::buttonColourId, juce::Colours::pink);
    openDawButton.setEnabled(true);
    addAndMakeVisible(&openDawButton);

    openEqButton.setButtonText(juce::CharPointer_UTF8("\xe5\x9d\x87\xe8\xa1\xa1\xe5\x99\xa8"));
    openEqButton.onClick = [this] { openEqButtonClicked(); };
    openEqButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    openEqButton.setEnabled(true);
    addAndMakeVisible(&openEqButton);

}
VoiceChanger_wczAudioProcessorEditor::~VoiceChanger_wczAudioProcessorEditor()
{
    if (templateRecordingWindow)
        delete[] templateRecordingWindow;
    stopTimer();
}

//==============================================================================
void VoiceChanger_wczAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient{ juce::Colours::darkgrey,getLocalBounds().toFloat().getCentre(), juce::Colours::darkgrey.darker(0.7f), {}, true });
    pPitchSlider.get()->setValue(audioProcessor.getPitchShift());
    pPeakSlider.get()->setValue(audioProcessor.getPeakShift());

    
    int x = 12, y = 12, width = 512, height = 256;

    g.drawImage(audioProcessor.getSpectrumView(), x, y, width, height, 0, 0, width, height);

    g.setColour(juce::Colours::black);
    g.fillEllipse(circularMeterL.getBounds().toFloat());
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
    const auto leftGain = audioProcessor.getRmsLevel(0);
    const auto rightGain = audioProcessor.getRmsLevel(1);

    horizontalMeterL.setLevel(leftGain);
    horizontalMeterR.setLevel(rightGain);
    AudioProcessorEditor::repaint();
    horizontalMeterL.repaint();
    horizontalMeterR.repaint();
}

void VoiceChanger_wczAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if(source==thumbnailCore)
	{
        audioProcessor.loadFileIntoTransport(thumbnailCore->getLastDroppedFile());
        thumbnailCore->setFile(thumbnailCore->getLastDroppedFile());
	}
}


void VoiceChanger_wczAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto a = getWidth() / 8;
    auto b = getWidth() / 16;
    xjjButton.setBounds(340, 340, a, 30);
    xpyButton.setBounds(340, 390, a, 30);
    ljButton.setBounds(340, 440, a, 30);
    mmButton.setBounds(340, 490, a, 30);
    realtimeButton.setBounds(340, 540, b, 30);
    offlineButton.setBounds(340 + b, 540, b, 30);
    resetAllButton.setBounds(340, 290, a , 30);
    switchPitchMethodButton.setBounds(245, 375, AudioProcessorEditor::getWidth() / 12, 20);
    switchVoiceConversionButton.setBounds(245, 410, getWidth() / 12, 20);

    openFileButton.setBounds(570, 380, a, 40);
    playFileButton.setBounds(570, 450, a, 40);
    stopPlayFileButton.setBounds(570, 520, a, 40);
    
    openTemplateWindowButton.setBounds(800, 390, AudioProcessorEditor::getWidth() / 12, 70);
    openCameraButton.setBounds(800, 480, getWidth() / 12, 70);
    audioSetupComp.setBounds(545, 20, 400, 100);
    //playAudioFileComponent.setBounds(AudioProcessorEditor::getLocalBounds());

    circularMeterL.setBounds(970, 20, 400, 400);
    circularMeterR.setBounds(970, 20, 400, 400);

    horizontalMeterL.setBounds(970, 460, 400, 12);
    horizontalMeterR.setBounds(970, 500, 400, 12);


    openDawButton.setBounds(1075, 530, 100, 50);
    openEqButton.setBounds(1175, 530, 100, 50);

    thumbnailCore->setBounds(10, 470, 320, 110);
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
/*#if _OPEN_FILTERS
    if (comboBoxThatWasMoved == pFilterTypeComboBox.get())
    {
        audioProcessor.setFilterTypeShift((int)pFilterTypeComboBox.get()->getSelectedId(), audioProcessor.currentFilterIndex);
    }
    else if (comboBoxThatWasMoved == pFilterIndexComboBox.get())
    {
        audioProcessor.currentFilterIndex = round(pFilterIndexComboBox.get()->getSelectedId());
    }
#endif*/
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
            // juce_path.cpp line835 JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])
            thumbnailCore->lastFileDropped = file;
            thumbnailCore->sendChangeMessage();
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
    audioProcessor.transportSource.stop();
    // changeState(Stopping);
}
void VoiceChanger_wczAudioProcessorEditor::playFileButtonClicked()
{
    //ti.setState(Starting);
    audioProcessor.setState(Starting);
    // audioProcessor.transportSource.setPosition(0);
    audioProcessor.transportSource.start();
    // changeState(Starting);
}


void VoiceChanger_wczAudioProcessorEditor::realtimeButtonClicked()
{
    audioProcessor.realtimeMode = true;
    audioProcessor.spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));

}
void VoiceChanger_wczAudioProcessorEditor::offlineButtonClicked()
{
    audioProcessor.realtimeMode = false;
    audioProcessor.spectrum.clear(juce::Rectangle<int>(512, 256), juce::Colour(0, 0, 0));
}

void VoiceChanger_wczAudioProcessorEditor::resetAllButtonClicked()
{
    // changeState(close);
    pPitchSlider->setValue(0.0);
    pPeakSlider->setValue(1.0);
}
void VoiceChanger_wczAudioProcessorEditor::switchPitchMethodButtonClicked()
{
    if (audioProcessor.useFD)
        audioProcessor.useFD = false;
    else
        audioProcessor.useFD = true;
}

void VoiceChanger_wczAudioProcessorEditor::switchVoiceConversionButtonClicked()
{
    if (audioProcessor.openVoiceConversion)
        audioProcessor.openVoiceConversion = false;
    else
    {
        audioProcessor.openVoiceConversion = true;
    }
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

void VoiceChanger_wczAudioProcessorEditor::openCameraButtonClicked()
{
    if (cameraWindow)
    {
        cameraWindow->broughtToFront();
    }
    else
    {
        cameraWindow = new NewWindow(juce::String("cameraWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);
        cameraWindow->setContentOwned(new CameraWindow(), true);
        cameraWindow->addToDesktop();
        cameraWindow->centreWithSize(500, 500);
        cameraWindow->setVisible(true);
    }
}

void VoiceChanger_wczAudioProcessorEditor::openDawButtonClicked()
{
    if (dawWindow)
    {
        dawWindow->broughtToFront();
    }
    else
    {
        dawWindow = new NewWindow(juce::String("dawWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);// new TemplateRecordingWindow();
        dawWindow->setContentOwned(new DawComponent(*this), true);
        dawWindow->addToDesktop();
        dawWindow->centreWithSize(800, 600);
        dawWindow->setVisible(true);
        // templateRecordingWindow->setName(juce::CharPointer_UTF8("\xe6\xa8\xa1\xe6\x9d\xbf"));
    }
}

void VoiceChanger_wczAudioProcessorEditor::openEqButtonClicked()
{
	if(eqWindow)
	{
        eqWindow->broughtToFront();
	}
	else
	{
        eqWindow = new NewWindow(juce::String("eqWindow"), juce::Colours::darkslategrey, juce::DocumentWindow::allButtons);
#if _OPEN_FILTERS
		eqWindow->setContentOwned(pEqEditor.get(), true);
#endif
		eqWindow->centreWithSize(900, 500);
		eqWindow->addToDesktop();
        eqWindow->setVisible(true);
	}
}
