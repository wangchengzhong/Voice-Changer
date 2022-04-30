#pragma once
#include <random>

#include"StructDefinitions.h"
class HSMsynthesize
{
public:
	HSMsynthesize(PicosStructArray& picos, int bufferLength);
	~HSMsynthesize() = default;
	Eigen::TRowVectorX processSynthesize(PicosStructArray& picos, int bufferLength);
	Eigen::TRowVectorX filter(const Eigen::TRowVectorX& a, const Eigen::TRowVectorX& x, int m);
private:
	const int fs{ 16000 };
	PicosStructArray& picos;
	int bufferLength;

	int threadNum{ 20 };
	int timesPerThread;

	Eigen::TRowVectorX y;
	size_t Npm;

	std::random_device rd;
	std::mt19937 gen;
	std::normal_distribution<Eigen::TFloat> nd;

	std::vector<int> n2;
	std::vector<int>n1, n3;

	std::vector<int> N12, N23;

	std::vector<Eigen::TRowVectorX> seq1, seq2;
	std::vector<Eigen::Matrix<double, 1, -1, 1>>win;
	std::vector<Eigen::TRowVectorX> trama;
	std::vector<Eigen::TRowVectorX> x;

private:
	std::vector<long long> na_filter;
	std::vector<Eigen::TRowVectorX> y_filter, an_filter;
};