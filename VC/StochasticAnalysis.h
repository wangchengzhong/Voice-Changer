#pragma once
#include"StructDefinitions.h"
class StochasticAnalysis
{
public:
	StochasticAnalysis(PicosStructArray& picos);
	~StochasticAnalysis();

	void processStochastic(Eigen::Ref<const Eigen::TRowVectorX> x);//, int N, PicosStructArray& picos, int ordenLPC);
private:
	PicosStructArray& picos;
};