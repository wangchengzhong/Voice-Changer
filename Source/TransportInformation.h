#pragma once
#include"JuceHeader.h"
enum TransportState
{
    Stopped,
    Starting,
    Playing,
    Stopping
};
struct TransportInformation {
    TransportState state;
    float audioFilePlayPoint;
    juce::AudioSampleBuffer* pFileBufferPointingAt;
};