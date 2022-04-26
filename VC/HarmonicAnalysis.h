#pragma once
#include"types.h"
#include"StructDefinitions.h"
class HarmonicAnalysis
{
public:
	HarmonicAnalysis(Eigen::Ref<Eigen::RowVectorXi> pms, Eigen::TFloat fmax);
	~HarmonicAnalysis();

	PicosStructArray processHarmonic(Eigen::Ref<const Eigen::TRowVectorX> x, 
		Eigen::Ref<const Eigen::TRowVectorX> f0s);
	void updateSize(Eigen::Ref<Eigen::RowVectorXi>pms);
private:
	Eigen::TFloat fmax{ 5000 };
	Eigen::Ref<Eigen::RowVectorXi> pms;

	PicosStructArray picos;
	int pmsSize;
	int threadNum{ 20 };
	int timesPerThread;
	int fs{ 16000 };

	std::vector<int> Lw;
	std::vector<int> Lw2;
	// std::vector<Eigen::VectorBlock<Eigen::TRowVectorX>> trama2T;

	std::vector<Eigen::TRowVectorX> gv;
	std::vector<Eigen::TVectorX> win;
	std::vector<int> K;
	std::vector<Eigen::TMatrixXc> h;
	std::vector<std::complex<Eigen::TFloat>>i1;
	std::vector<Eigen::TMatrixXc> t1;
	std::vector<Eigen::TMatrixXc> t2;
	std::vector<Eigen::TRowVectorXc> t3;
	std::vector<Eigen::TRowVectorXc> coef;
	// std::vector<Eigen::VectorBlock<Eigen::TRowVectorXc>> coef1K;



};