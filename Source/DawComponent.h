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


    juce::TextButton settingsButton{ "Settings" }, pluginsButton{ "Plugins" },
        newEditButton{ "New" }, playPauseButton{ "Play" }, recordButton{ "Record" },
        showEditButton{ "Show Project" }, newTrackButton{ "New Track" },
        deleteButton{ "Delete" }, clearTracksButton{ "Clear Tracks" }, aboutButton{ "?" };

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