#pragma once
#include"F0analysis.h"
#include"HarmonicAnalysis.h"
#include"StochasticAnalysis.h"
#include"PolarityAnalysis.h"
#include"DecomposePhase.h"
#include"seq.h"
#include "StructDefinitions.h"
#include "types.h"

class HSManalysis
{
public:
	HSManalysis(Eigen::RowVectorXi pms);
	~HSManalysis();

	PicosStructArray processHSManalysis(Eigen::Ref<Eigen::TRowVectorX> x);

private:
	F0analysis f0Analysis;
	HarmonicAnalysis harmoniceAnalysis;
	StochasticAnalysis stochasticAnalysis;
	PolarityAnalysis polarityAnalysis;
	DecomposePhase decomposePhase;

	int N;

	Eigen::TFloat f0min{ 60 };
	Eigen::TFloat f0max{ 500 };
	Eigen::TFloat fmax{ 5000 };
	int ordenLPC{ 14 };


	int bufferLength;
	int temp;
	Eigen::RowVectorXi& pms;
	PicosStructArray picos;

	Eigen::TRowVectorX f0s;

};
