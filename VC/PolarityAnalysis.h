#pragma once

#include "StructDefinitions.h"

class PolarityAnalysis
{
public:
	PolarityAnalysis(PicosStructArray& picos);
	~PolarityAnalysis();
	void updateSize(PicosStructArray& picos);
	int processPolarity(PicosStructArray picos);
private:
	PicosStructArray& picos;
	int pol = 0;
	Eigen::TFloat polE = 0.0;

	int threadNum{ 20 };
	int timesPerThread;

	std::vector<Eigen::TFloat> E;
	std::vector<Eigen::TFloat> alfa;

	std::vector<Eigen::TFloat> dP;
	std::vector<Eigen::TFloat> dN;
};
