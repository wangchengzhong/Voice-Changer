#pragma once
//#include "seq.h"
#include "StructDefinitions.h"
class HSMwfwConvert
{
public:
	HSMwfwConvert(HSMModel& model,PicosStructArray& picos);
	~HSMwfwConvert();

	void processWfwConvert(PicosStructArray& picos);
	Eigen::TVectorX aaalsf(Eigen::Ref<const Eigen::TRowVectorX> aa, Eigen::TFloat f0, size_t aaa);
	Eigen::TRowVectorX lsfadap(Eigen::Ref<const Eigen::TVectorX> lsf, size_t aaa);
private:
	HSMModel& model;
	PicosStructArray& picos;
	Eigen::TFloat fmax{ 5000.0 };
	const std::complex<Eigen::TFloat> i1{0,1};
	const Eigen::TRowVector4& f0f{ model.f0f };
	int fs{ 16000 };
	double u1{ f0f(0) };
	double v1{ f0f(1) };
	double u2{ f0f(2) };
	double v2{ f0f(3) };
	Eigen::TRowVectorX f01s;
	Eigen::TRowVectorX f02s;
	Eigen::TFloat dph{ 0.0 };

	const ThxyStructArray& th{ model.th };
	const ThxyStructArray& th2{ model.th2 };
	Eigen::Index p{ th[0].u.size() };
	size_t m{ th.size() };
	size_t mm{ th2.size() };

	std::vector<Eigen::TFloat> thd;
	std::vector<Eigen::TMatrixX> thI;

	std::vector<Eigen::TFloat> th2d;
	std::vector<Eigen::TMatrixX> th2I;

	const FwxyStructArray& fw{ model.fw };
	const Eigen::RowVectorXi& fx{ fw[0].x };

	int threadNum{ 20 };
	int timesPerThread;

	std::vector<Eigen::TRowVectorX> P;
	std::vector<Eigen::TRowVectorX> PP;
	std::vector<Eigen::TRowVectorX> P2;

private:
	std::vector<int> lsfNk;
	std::vector<Eigen::TRowVectorX> lsfff;
	std::vector<Eigen::Array<double, 1, -1, 1>> lsfPP;
	int lsfFs{ 2 * 5000 };
	std::vector<Eigen::TRowVectorX> lsfR;
	std::vector<Eigen::TRowVectorX> lsfai;
	std::vector<double> lsfe;

	std::vector<double> lsfk;
	std::vector<Eigen::TRowVectorX> lsfaz1;

	//std::vector<Eigen::Reverse<Eigen::TRowVectorX>> lsfaz2;
	std::vector<Eigen::TRowVectorX> lsflsf;
	std::vector<int> lsfSize;

private:
	std::vector<Eigen::TVectorX> v;
	std::vector<double>Psum;
	std::vector<Eigen::TVectorX> vt;

private:
	Eigen::RowVectorXi adaps1; //{seq<Eigen::RowVectorXi>(2, 2, p) };
	Eigen::RowVectorXi adaps2;// {seq<Eigen::RowVectorXi>(1, 2, p) };

	std::vector<Eigen::TRowVectorXc> adapPart1, adapPart2;
	std::vector<Eigen::TRowVectorXc> temp1, temp2;
	std::vector<Eigen::TRowVectorX> adapai;

private:
	std::vector<Eigen::TRowVectorX> aivt;

	std::vector<Eigen::TRowVectorXc>eevt;
};