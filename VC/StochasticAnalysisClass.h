#pragma once
#include"StructDefinitions.h"
class StochasticAnalysis
{
public:
	StochasticAnalysis(PicosStructArray& picos, int xLength);
	~StochasticAnalysis() = default;

	void processStochastic(Eigen::Ref<const Eigen::TRowVectorX> x);//, int N, PicosStructArray& picos, int ordenLPC);
	std::tuple<Eigen::TFloat, Eigen::TFloat, Eigen::TFloat, Eigen::TFloat> bymcaquat(Eigen::TFloat p1, Eigen::TFloat p2, Eigen::TFloat f1, Eigen::TFloat f2, Eigen::TFloat N, Eigen::TFloat fs, size_t m);
	void updateSize(PicosStructArray& picos);
private:
	PicosStructArray& picos;
	int N{ 128 };
	int ordenLPC{ 14 };
	int fs{ 16000 };

	int Npm;
	Eigen::TRowVectorX yd;
	int left;
	Eigen::Index size;
	Eigen::TRowVectorX seq1;
	Eigen::TRowVectorX seq2;

	Eigen::TRowVectorX seq3;
	Eigen::TRowVectorX seq4;

	Eigen::TRowVectorX seq5;
	int threadNum{ 20 };
	int timesPerThread;


	std::vector<long long> minSize;
	std::vector<double>a1;
	std::vector<double> a2;
	std::vector<double>c0, c1, c2, c3;
	std::vector<double>c01, c11, c21, c31;

	std::vector<Eigen::TFloat> w1, w2, M;

	// Eigen::TRowVectorX ye;
	// Eigen::VectorBlock<Eigen::TRowVectorX> ye_;
	Eigen::TRowVectorX hnnN;

	std::vector<Eigen::TRowVectorX> trama;
	std::vector<Eigen::TRowVectorX> R;
};