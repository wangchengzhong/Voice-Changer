#pragma once
#include"StructDefinitions.h"
class DecomposePhase
{
public:
	DecomposePhase(PicosStructArray& picos);
	~DecomposePhase();

	void processDecompose(PicosStructArray& picos);
	void updateSize(PicosStructArray& picos);
	Eigen::TFloat linphaseterm(Eigen::Ref<const Eigen::TRowVectorX>aa, Eigen::Ref<const Eigen::TRowVectorX>pp, int fmaxopt, size_t m);
private:
	PicosStructArray& picos;
	int fmaxopt{ 1000 };
	Eigen::TFloat fmax{ 5000.0 };

	int threadNum{ 20 };
	int timesPerThread;

	std::vector<Eigen::TFloat>alfa;

private:
	std::vector<int> K;
	std::vector<Eigen::TRowVectorX> linpAlfa;
	std::vector<int> imax;

};