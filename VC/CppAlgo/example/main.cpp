#include <cstdio>
#include "../include/vchsm/train_C.h"
#include "../include/vchsm/convert_C.h"

#define VERBOSE_TRUE 1
#define VERBOSE_FALSE 0

int main()
{
	// NOTE: change the following directories for the data files according to your actual path.
	// train a model with the given 20 source and target speaker's audios
	// the audios files are named from 1 to 20
	const char* sourceAudioDir = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/source_train/";
	const char*  targetAudioDir = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/target_train/";
	const int numTrainSamples = 20;
	const char* sourceAudioList[numTrainSamples];
	const char* targetAudioList[numTrainSamples];
	for (int i = 0; i < numTrainSamples; ++i)
	{
		char* buff = new char[100];
		std::sprintf(buff, "%s%d.wav", sourceAudioDir, i + 1);
		sourceAudioList[i] = buff;
		buff = new char[100];
		std::sprintf(buff, "%s%d.wav", targetAudioDir, i + 1);
		targetAudioList[i] = buff;
	}
	// model file to be generated
	const char* modelFile = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/ModelsModel.dat";
	// start training	
	trainHSMModel(sourceAudioList, targetAudioList, numTrainSamples, 4, modelFile, VERBOSE_TRUE);
	// deallocate
	for (int i = 0; i < numTrainSamples; ++i)
	{
		delete[] sourceAudioList[i];
		delete[] targetAudioList[i];
	}
	// perform conversion
	const char* testAudio = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/test/jal_in_42_3.wav";
	const char* testAudioConverted = "D:/1a/voice_changer@wcz/VoiceChanger@wcz/VC/Audios/test/jal_in_42_3_c.wav";
	convertSingle(modelFile, testAudio, testAudioConverted, VERBOSE_TRUE);
	// now we can compare the above audio before and after conversion
	std::getchar();
}