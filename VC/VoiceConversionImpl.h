#pragma once
#include <vector>
#include "modelSerialization.h"
#include"HSManalysisClass.h"
#include"HSMwfwConvertClass.h"
#include"HSMsynthesizeClass.h"
class VoiceConversionImpl
{
public:
	VoiceConversionImpl(HSMModel model, Eigen::RowVectorXi pms, Eigen::TRowVectorX x, PicosStructArray picos);
	~VoiceConversionImpl() = default;
	
	void processConversion(std::vector<double>& origBuffer, std::vector<double>& convertedBuffer, int verbose) noexcept;
	void resize();

	HSMModel model;
	int sampleRate;
	int bufferLength{ 15000 };
	Eigen::RowVectorXi pms;
	Eigen::TRowVectorX x;
	PicosStructArray picos;
	Eigen::TRowVectorX output;

	HSManalysis hsmAnalysis;
	HSMwfwConvert hsmWfwConvert;
	HSMsynthesize hsmSynthesize;






};