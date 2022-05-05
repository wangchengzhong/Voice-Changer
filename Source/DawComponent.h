#pragma once
#include <JuceHeader.h>
#include"EditComponent.h"
#include"EngineHelpers.h"
#include"ExtendedUIBehaviour.h"
#include"TransportToolbarFactory.h"
#include"PluginProcessor.h"
/*详见论文6.6.2：引擎通过 GUI 界面操纵编辑控制模块各参数，从而控制底层音频流；它在构
建时需引入引擎编辑控制模块和选择管理器。其中，编辑控制由主处理器引用传
来，负责各轨道的激活、音频文件位置管理等；选择管理器在创建界面时构建。
引擎界面组件分为主控按钮和单轨编辑两大模块。主控按钮直接通过选择管
理器获取用户的编辑目标轨道，而后执行轨道的新建、删除等；它还控制整个编辑
器的音频流来源。单轨编辑模块在主控按钮控制下构建轨道或删除轨道。它负责
轨道的播放、录制、编辑，并负责每个轨道内部的细节。它的子模块中，轨道音频
缩略图组件、轨道信息设置组件、轨道外挂插件组件可以动态构建和销毁；而进
度条显示和视图状态用于管理轨道整体，是静态的。*/
class DawComponent:public juce::Component,
                   public juce::Button::Listener,
                   public juce::Slider::Listener,
					private juce::ChangeListener
{
public:
    DawComponent(juce::Component& parent
        , tracktion_engine::Engine& engine
		,tracktion_engine::Edit& edit
        , VoiceChanger_wczAudioProcessor& audioProcessor);
    ~DawComponent();

    void paint(Graphics& g) override;
    void resized() override;
private:
    void buttonClicked(Button*) override;
    void sliderValueChanged(Slider* slider) override;

    //tracktion_engine::Engine engine{ ProjectInfo::projectName,std::make_unique<ExtendedUIBehaviour>(),nullptr };
    tracktion_engine::SelectionManager selectionManager{ engine };
    // std::unique_ptr<tracktion_engine::Edit> edit;

    std::unique_ptr<EditComponent>editComponent;


    juce::TextButton settingsButton{ juce::CharPointer_UTF8("\xe8\xae\xbe\xe7\xbd\xae") },
		pluginsButton{ juce::CharPointer_UTF8("\xe6\x8f\x92\xe4\xbb\xb6") },
        newEditButton{ "New" },
		playPauseButton{ "Play" },
		recordButton{ "Record" },
        triggerDawStreamButton{ juce::CharPointer_UTF8("\xe8\xa7\xa6\xe5\x8f\x91\xe7\xbc\x96\xe8\xbe\x91\xe5\x99\xa8\xe9\x9f\xb3\xe9\xa2\x91") },
		newTrackButton{ juce::CharPointer_UTF8("\xe6\x96\xb0\xe5\xbb\xba\xe8\xbd\xa8\xe9\x81\x93") },
        deleteButton{ juce::CharPointer_UTF8("\xe5\x88\xa0\xe9\x99\xa4\xe9\x9f\xb3\xe9\xa2\x91\xe5\x9d\x97") },
		clearTracksButton{ juce::CharPointer_UTF8("\xe6\xb8\x85\xe9\x99\xa4\xe6\x89\x80\xe6\x9c\x89\xe8\xbd\xa8\xe9\x81\x93") },
		recordMixButton{ juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x88\xb6\xe9\x9f\xb3\xe9\xa2\x91\xe7\xbc\xa9\xe6\xb7\xb7") };

    juce::Label editNameLabel{ "No Edit Loaded" };
    juce::ToggleButton showWaveformButton{ "Show Waveforms" };

    void setupGUI();
    void updatePlayButton()
    {
        if (&edit == nullptr) return;
        playPauseButton.setButtonText(edit.getTransport().isPlaying() ? "Stop" : "Play");

    }

    void updateRecordButtonText()
    {
        if (&edit == nullptr) return;
        recordButton.setButtonText(edit.getTransport().isRecording() ? "Abort" : "Record");

    }

    void onRecordTracks();
    void setSongTitle(const juce::String& title);
    void createOrLoadEdit(juce::File editFile = {}, bool loadOnly = false);
    void changeListenerCallback(ChangeBroadcaster* source) override
    {
	    if(&edit!=nullptr&&source==&edit.getTransport())
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
        edit.getTransport().ensureContextAllocated();
        if(tracktion_engine::getAudioTracks(edit).size()==0)
        {
            int trackNum = 0;
            for(auto* instance:edit.getAllInputDevices())
            {
	            if(instance->getInputDevice().getDeviceType()==tracktion_engine::InputDevice::physicalMidiDevice)
	            {
		            if(auto* t = EngineHelpers::getOrInsertAudioTrackAt(edit,trackNum))
		            {
                        instance->setTargetTrack(*t, 0, true);
                        instance->setRecordingEnabled(*t, true);

                        trackNum++;
		            }
	            }
            }
        }
        edit.restartPlayback();
    }

    juce::Toolbar toolbar;
    TransportToolbarItemFactory factory;
    Component& parent;
    tracktion_engine::Engine& engine;
    tracktion_engine::Edit& edit;
    VoiceChanger_wczAudioProcessor& audioProcessor;
	std::atomic<bool>& isDawStream;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DawComponent);
};