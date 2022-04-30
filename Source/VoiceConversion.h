#include"FIFOSamplePipeForVC.h"
#include"MatchBuffer.h"
#define SETTING_SEQUENCE_MS         3
#define SETTING_SEEKWINDOW_MS       4
#define SETTING_OVERLAP_MS          5
#define SETTING_NOMINAL_INPUT_SEQUENCE      6
#define SETTING_NOMINAL_OUTPUT_SEQUENCE     7

class VoiceConversion:public FIFOProcessor
{
private:
	class BufferMatch* pBufferMatch;
	long samplesOutput;
public:
	VoiceConversion(int sampleRate, HSMModel model);
	~VoiceConversion();
	void setChannels(uint numChannels);
	// void setSampleRate(uint srate);
	virtual void putSamples(
		const SAMPLETYPE* samples,
		uint numSample
	);
	virtual void setSequenceLength(int sequenceLength);
	// bool setSettings(int settingId, int value);
	virtual uint receiveSamples(SAMPLETYPE* output, uint maxSample);
	virtual uint receiveSamples(uint maxSamples);
	virtual void clear();
	
};