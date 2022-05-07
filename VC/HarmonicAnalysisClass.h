#pragma once
#include"types.h"
#include"StructDefinitions.h"
class HarmonicAnalysis
{
public:
	HarmonicAnalysis(Eigen::RowVectorXi& pms, Eigen::TFloat fmax, PicosStructArray picos);
	~HarmonicAnalysis() = default;

	PicosStructArray processHarmonic(const Eigen::TRowVectorX& x, 
		const Eigen::TRowVectorX f0s);
	void updateSize(Eigen::Ref<Eigen::RowVectorXi>pms);
private:
	int fs{ 16000 };
	int threadNum{ 20 };
	int timesPerThread;
	Eigen::TFloat fmax{ 5000 };
	PicosStructArray picos;
	Eigen::RowVectorXi& pms;
	// ::TRowVectorX f0s;
	
	int pmsSize;

	std::vector<int> Lw;
	std::vector<int> Lw2;
	// std::vector<Eigen::VectorBlock<Eigen::TRowVectorX>> trama2T;

	std::vector<Eigen::TRowVectorX> gv;
	std::vector<Eigen::TVectorX> win;
	std::vector<int> K;
	std::vector<Eigen::TMatrixXc> h;
	std::complex<Eigen::TFloat>i1;// (0, 1);
	std::vector<Eigen::Matrix<std::complex<double>, -1, -1, 1>> t1;
	std::vector<Eigen::Matrix<std::complex<double>, -1, -1, 0>> t2;
	std::vector<Eigen::Matrix<std::complex<double>, -1, 1, 0>> t3;
	std::vector<Eigen::Matrix<std::complex<double>, -1, 1, 0>> coef;
	// std::vector<Eigen::VectorBlock<Eigen::TRowVectorXc>> coef1K;
};