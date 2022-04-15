#pragma once
#include"types.h"
#include "FFT.h"
#include"constants.h"
#include"seq.h"
#include"vector"
struct DatElement
{
	Eigen::TRowVectorX r, f, ac;
	Eigen::RowVectorXi ant;
};

class F0analysis
{
public:
	F0analysis(
		// Eigen::Ref<Eigen::TRowVectorX> x,
		Eigen::Ref<Eigen::RowVectorXi> pms,
		Eigen::TFloat f0min,
		Eigen::TFloat f0max
	);
	~F0analysis();

	Eigen::TRowVectorX processF0(const Eigen::Ref<const Eigen::TRowVectorX>& x, const Eigen::Ref<const Eigen::RowVectorXi>&pms);
	void updateSize(const Eigen::Ref<const Eigen::RowVectorXi>& pms);
private:
	Eigen::TFloat f0min;
	Eigen::TFloat f0max;
	Eigen::Ref<Eigen::RowVectorXi> pms;
	double fs{ 16000.0 };
	//Eigen::Ref<Eigen::TRowVectorX> x;



private:
	int Ncandidates{ 6 };
	int lagmin;
	int lagmax;
	Eigen::TFloat fact;
	Eigen::TFloat voith{ 0.45 };
	Eigen::TFloat silth{ 0.03 };
	Eigen::TFloat octcost{ 0.01 };
	Eigen::TFloat vuvcost;
	Eigen::TFloat uvvcost;
	Eigen::TFloat uvuvcost{ 0.0 };
	Eigen::TFloat octjump;
	int L;
	int L2{ 0 };
	int Lz;
	int Lp2{ 2 };
	int Lxlmax;
	int Ldc;
	Eigen::TRowVectorX w;
	Eigen::TRowVectorXc tt;
	Eigen::TRowVectorX rw_temp;
	Eigen::TRowVectorX rw;
	Eigen::TFloat xgmax;
	std::vector<DatElement> dat;

	std::vector<Eigen::TRowVectorX> rx;

	std::vector<Eigen::TRowVectorX> rk;
	std::vector<Eigen::TRowVectorX> fk;


	int threadNum{ 20 };
	int timesPerThread;

private:
	std::vector<Eigen::TRowVectorX> trama;
	std::vector<Eigen::TRowVectorX> tramaTemp;

	std::vector<int> innerLength;
	std::vector<Eigen::TFloat> xlmax;

	std::vector<Eigen::TRowVectorXc> ftemp;
	std::vector<Eigen::TRowVectorX> ra_;
	std::vector<Eigen::TRowVectorX> ra;

	std::vector<double> tmax;
	std::vector<double> rmax;
	std::vector<Eigen::TFloat> fmax;
	std::vector<Eigen::Index> jj;
	std::vector<double> rmin;

	std::vector<Eigen::TFloat> ruv;
	/////

	Eigen::TFloat mincost;
	int jjmincost;
	Eigen::TFloat cost;

	Eigen::TRowVectorX f0s;
	Eigen::Index jjmincostFinal;

};
