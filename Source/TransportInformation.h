#pragma once
#include"JuceHeader.h"
enum TransportState
{
    Stopped,
    Starting,
    Playing,
    Stopping
};
enum TransportFileType
{
    Source,
    Target,
    Mainpage
};
class TransportInformation 
{
public:
    TransportInformation(
        TransportState state,
        TransportFileType transportType 
    )
        :state(state), transportType(transportType)
    {

    }
    TransportInformation(
        TransportInformation& ti
    )
    {
        state = ti.state;
        transportType = ti.transportType;
    }
    virtual void setTarget(TransportFileType filtType) {}
    virtual void setState(TransportState newState) {}
    TransportState state;
    TransportFileType transportType;
    float audioFilePlayPoint;
    // juce::AudioSampleBuffer* pFileBufferPointingAt;
};

//void VoiceChanger_wczAudioProcessorEditor::changeState(TransportState newState)
//{
//    auto state = audioProcessor.state;
//    if (newState != state)
//    {
//        audioProcessor.state = newState;
//        state = newState;
//        switch (state)
//        {
//        case Stopped:
//            stopPlayFileButton.setEnabled(false);
//            playFileButton.setEnabled(true);
//            //audioProcessor.shouldProcessFile = false;
//            // audioProcessor.transportSource.setPosition(0.0);
//            break;
//        case Playing:
//            stopPlayFileButton.setEnabled(true);
//            //audioProcessor.shouldProcessFile = true;
//
//            break;
//        case Starting:
//            stopPlayFileButton.setEnabled(true);
//            playFileButton.setEnabled(true);
//            //audioProcessor.shouldProcessFile = true;
//            // audioProcessor.transportSource.start();
//            break;
//        case Stopping:
//            playFileButton.setEnabled(true);
//            stopPlayFileButton.setEnabled(false);
//            //audioProcessor.shouldProcessFile = false;
//            // audioProcessor.transportSource.stop();
//            break;
//        default:
//            break;
//        }
//    }
//}