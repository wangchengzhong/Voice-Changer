#pragma once
#include"F0analysisClass.h"
#include"HarmonicAnalysisClass.h"
#include"StochasticAnalysisClass.h"
#include"PolarityAnalysisClass.h"
#include"DecomposePhaseClass.h"
#include"seq.h"
#include "StructDefinitions.h"
#include "types.h"

class HSManalysis
{
public:
	HSManalysis(Eigen::RowVectorXi& pms, PicosStructArray& picos);
	~HSManalysis()  = default;

	PicosStructArray processHSManalysis(Eigen::TRowVectorX& x);

private:
	int N{ 128 };

	Eigen::TFloat f0min{ 50 };
	Eigen::TFloat f0max{ 500 };
	Eigen::TFloat fmax{ 5000 };
	int ordenLPC{ 14 };


	int bufferLength{ 15000 };
	int temp;
	Eigen::RowVectorXi& pms;
	PicosStructArray& picos;

	Eigen::TRowVectorX f0s;

	F0analysis f0Analysis;
	HarmonicAnalysis harmonicAnalysis;
	StochasticAnalysis stochasticAnalysis;
	PolarityAnalysis polarityAnalysis;
	DecomposePhase decomposePhase;



};
