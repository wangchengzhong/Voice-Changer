#pragma once
#include"types.h"
#include"StructDefinitions.h"
class HarmonicAnalysis
{
public:
	HarmonicAnalysis(Eigen::Ref<Eigen::RowVectorXi> pms, Eigen::TFloat fmax);
	~HarmonicAnalysis();

	PicosStructArray processHarmonic(Eigen::Ref<const Eigen::TRowVectorX> x, 
		Eigen::Ref<const Eigen::RowVectorXi> pms,
		Eigen::Ref<const Eigen::TRowVectorX> f0s);
private:
	Eigen::TFloat fmax{ 5000 };
	Eigen::Ref<Eigen::RowVectorXi> pms;

	PicosStructArray picos;
	int pmsSize;
	int threadNum{ 20 };
	int timesPerThread;


};