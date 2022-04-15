#pragma once
#include <vector>
#include "modelSerialization.h"
#include"HSManalysis.h"
#include"HSMwfwConvert.h"
#include"HSMsynthesize.h"
class VoiceConversionImpl
{
public:
	VoiceConversionImpl(HSMModel& model);
	~VoiceConversionImpl();
	
	void processConversion(std::vector<double>& origBuffer, std::vector<double>& convertedBuffer, int verbose) noexcept;
	void resize();

	HSMModel model;
	HSManalysis hsmAnalysis;
	HSMwfwConvert hsmWfwConvert;
	HSMsynthesize hsmSynthesize;
	int sampleRate;
	int bufferLength;

	PicosStructArray picos;
	Eigen::TRowVectorX x;



};