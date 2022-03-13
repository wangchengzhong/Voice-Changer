#pragma once
#include <JuceHeader.h>
#include"EditComponent.h"
#include"EngineHelpers.h"
#include"ExtendedUIBehaviour.h"
#include"TransportToolbarFactory.h"

class DawComponent:public juce::Component,
                   public juce::Button::Listener,
                   public juce::Slider::Listener,
					private juce::ChangeListener
{
public:
    DawComponent(juce::Component& parent);
    ~DawComponent();

    void paint(Graphics& g) override;
    void resized() override;
private:
    void buttonClicked(Button*) override;
    void sliderValueChanged(Slider* slider) override;

    tracktion_engine::Engine engine{ ProjectInfo::projectName,std::make_unique<ExtendedUIBehaviour>(),nullptr };
    tracktion_engine::SelectionManager selectionManager{ engine };
    std::unique_ptr<tracktion_engine::Edit> edit;

    std::unique_ptr<EditComponent>editComponent;


    juce::TextButton settingsButton{ juce::CharPointer_UTF8("\xe8\xae\xbe\xe7\xbd\xae") }, pluginsButton{ juce::CharPointer_UTF8("\xe6\x8f\x92\xe4\xbb\xb6") },
        newEditButton{ "New" }, playPauseButton{ "Play" }, recordButton{ "Record" },
        showEditButton{ juce::CharPointer_UTF8("\xe6\x98\xbe\xe7\xa4\xba\xe5\xb7\xa5\xe7\xa8\x8b") }, newTrackButton{ juce::CharPointer_UTF8("\xe6\x96\xb0\xe5\xbb\xba\xe8\xbd\xa8\xe9\x81\x93") },
        deleteButton{ juce::CharPointer_UTF8("\xe5\x88\xa0\xe9\x99\xa4\xe9\x9f\xb3\xe9\xa2\x91\xe5\x9d\x97") }, clearTracksButton{ juce::CharPointer_UTF8("\xe6\xb8\x85\xe9\x99\xa4\xe6\x89\x80\xe6\x9c\x89\xe8\xbd\xa8\xe9\x81\x93") }, aboutButton{ "?" };

    juce::Label editNameLabel{ "No Edit Loaded" };
    juce::ToggleButton showWaveformButton{ "Show Waveforms" };

    void setupGUI();
    void updatePlayButton()
    {
        if (edit == nullptr) return;
        playPauseButton.setButtonText(edit->getTransport().isPlaying() ? "Stop" : "Play");

    }

    void updateRecordButtonText()
    {
        if (edit == nullptr) return;
        recordButton.setButtonText(edit->getTransport().isRecording() ? "Abort" : "Record");

    }

    void onRecordTracks();
    void setSongTitle(const juce::String& title);
    void createOrLoadEdit(juce::File editFile = {}, bool loadOnly = false);
    void changeListenerCallback(ChangeBroadcaster* source) override
    {
	    if(edit!=nullptr&&source==&edit->getTransport())
	    {
            updatePlayButton();
            updateRecordButtonText();
	    }
        else if(source==&selectionManager)
        {
            auto sel = selectionManager.getSelectedObject(0);
            deleteButton.setEnabled(dynamic_cast<tracktion_engine::Clip*>(sel) != nullptr
                || dynamic_cast<tracktion_engine::Track*>(sel) != nullptr
                || dynamic_cast<tracktion_engine::Plugin*>(sel));


        }
    }
    void createTracksAndAssignInputs()
    {
        auto& dm = engine.getDeviceManager();
        for(int i = 0; i< dm.getNumMidiInDevices();i++)
        {
	        if(auto mip=dm.getMidiInDevice(i))
	        {
                mip->setEndToEndEnabled(true);
                mip->setEnabled(true);
	        }
        }
        edit->getTransport().ensureContextAllocated();
        if(tracktion_engine::getAudioTracks(*edit).size()==0)
        {
            int trackNum = 0;
            for(auto* instance:edit->getAllInputDevices())
            {
	            if(instance->getInputDevice().getDeviceType()==tracktion_engine::InputDevice::physicalMidiDevice)
	            {
		            if(auto* t = EngineHelpers::getOrInsertAudioTrackAt(*edit,trackNum))
		            {
                        instance->setTargetTrack(*t, 0, true);
                        instance->setRecordingEnabled(*t, true);

                        trackNum++;
		            }
	            }
            }
        }
        edit->restartPlayback();
    }

    juce::Toolbar toolbar;
    TransportToolbarItemFactory factory;
    Component& parent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DawComponent);
};