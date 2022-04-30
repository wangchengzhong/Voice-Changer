#include"VoiceConversion.h"
#include <assert.h>
#include <stdlib.h>


#define TEST_FLOAT_EQUAL(a, b)  (fabs(a - b) < 1e-10)

VoiceConversion::VoiceConversion(int sampleRate, HSMModel model)
{
	pBufferMatch = BufferMatch::newInstance(sampleRate, model);
	setOutPipe(pBufferMatch);
}
VoiceConversion::~VoiceConversion()
{
	delete pBufferMatch;
}

void VoiceConversion::setChannels(uint numChannels)
{
	pBufferMatch->setChannels((int)numChannels);
}

//void VoiceConversion::setSampleRate(uint srate)
//{
//	pBufferMatch->setParameters((int)srate);
//}

void VoiceConversion::putSamples(const SAMPLETYPE* samples, uint numSample)
{
	pBufferMatch->putSamples(samples, numSample);
}

void VoiceConversion::setSequenceLength(int sequenceLength)
{
	pBufferMatch->setSequenceLength(sequenceLength);
}

//bool VoiceConversion::setSettings(int settingId, int value)
//{
//	int sampleRate, sequenceMs, seekWindowMs, overlapMs;
//
//	pBufferMatch->getParameters(&sampleRate, &sequenceMs, &seekWindowMs, &overlapMs);
//	switch (settingId)
//	{
//	case SETTING_SEQUENCE_MS:
//		pBufferMatch->setParameters(sampleRate, value, seekWindowMs, overlapMs);
//		return true;
//	case SETTING_SEEKWINDOW_MS:
//		pBufferMatch->setParameters(sampleRate, sequenceMs, value, overlapMs);
//		return true;
//
//	case SETTING_OVERLAP_MS:
//		pBufferMatch->setParameters(sampleRate, sequenceMs, seekWindowMs, value);
//		return true;
//	default:
//		return false;
//	}
//}
void VoiceConversion::clear()
{
	pBufferMatch->clear();
}

uint VoiceConversion::receiveSamples(SAMPLETYPE* output, uint maxSample)
{
	uint ret = FIFOProcessor::receiveSamples(output, maxSample);
	samplesOutput += (long)ret;
	return ret;
}

uint VoiceConversion::receiveSamples(uint maxSamples)
{
	// pBufferMatch->receiveSamples(maxSamples);
	uint ret = FIFOProcessor::receiveSamples(maxSamples);
	samplesOutput += (long)ret;
	return ret;
}






