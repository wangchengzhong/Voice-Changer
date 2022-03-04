#pragma once
#include"JuceHeader.h"
#include"D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/CppAlgo/include/vchsm/convert_C.h"

class VoiceConversionInterface
{
public:
	VoiceConversionInterface(juce::AudioBuffer<float>& buffer)
		: buffer(buffer)
	{
		
	}

private:
	juce::AudioBuffer<float>& buffer;
	juce::AudioBuffer<float> downSampledBuffer;
	juce::AudioBuffer<float> upSampledBuffer;

};