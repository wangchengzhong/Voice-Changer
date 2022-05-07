#pragma once
//#include "seq.h"
#include "StructDefinitions.h"
class HSMwfwConvert
{
public:
	HSMwfwConvert(HSMModel model,PicosStructArray& picos);
	~HSMwfwConvert() = default;

	void processWfwConvert(PicosStructArray& picos);
	Eigen::TVectorX aaalsf(Eigen::Ref<const Eigen::TRowVectorX> aa, Eigen::TFloat f0, size_t aaa);
	Eigen::TRowVectorX lsfadap(Eigen::Ref<const Eigen::TVectorX> lsf, size_t aaa);
	void updateSize(PicosStructArray& picos);
private:
	int threadNum{ 20 };
	int timesPerThread;

	HSMModel model;
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
	/**
	 * %%%%%%%%%%%%%%%%%%%%%%%%%%%% & th,th2
	 */
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

	std::vector<Eigen::TRowVectorX> P;
	std::vector<Eigen::TRowVectorX> PP;
	std::vector<Eigen::TRowVectorX> P2;

private:
	std::vector<int> lsfNk;
	std::vector<Eigen::Matrix<double,1, -1, 1>> lsfff;
	std::vector<Eigen::Array<double, 1, -1, 1>> lsfPP;
	const int lsfFs{ 10000 };
	std::vector<Eigen::Matrix<double, 1, -1, 1>> lsfR;
	std::vector<Eigen::TRowVectorX> lsfai;
	std::vector<double> lsfe;

	std::vector<double> lsfk;
	std::vector<Eigen::TRowVectorX> lsfaz1;
	std::vector<Eigen::TRowVectorX> lsfaz2;
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
	std::vector<Eigen::TRowVectorXc> adapTemp1, adapTemp2;
	std::vector<Eigen::TRowVectorX> adapai;

private:
	std::vector<Eigen::TRowVectorX> aivt;

	std::vector<Eigen::TMatrixXc>eevt;
	std::vector<Eigen::TRowVectorXc> Afvt;
	std::vector<double> E1;
	std::vector<Eigen::TRowVectorX>Evt;
	std::vector<Eigen::TRowVectorX> aavt;

	std::vector<Eigen::TVectorX>vtt;

	std::vector<Eigen::TVectorX> vttaux;

	std::vector<Eigen::TRowVectorX> aivtt;
	std::vector<Eigen::TRowVectorX> fy;

	std::vector<Eigen::TRowVectorX> ff1;
	std::vector<Eigen::TRowVectorXc>ap1;

	std::vector<Eigen::TRowVectorX> aa1;
	std::vector<Eigen::TRowVectorX> aa2;
	std::vector<Eigen::TRowVectorX> pp2;

	std::vector<int> jjant1;
	std::vector<int> jjant2;

	std::vector<Eigen::TFloat> scale;
	std::vector<double> f2j;
	std::vector<double> f1j;
	std::vector<Eigen::Index> jj;

	Eigen::TFloat flimsm{ 1000.0 };
	std::vector<int> nf;
	std::vector<Eigen::Matrix<int, 1, -1, 1>> seqnf;
	std::vector<Eigen::TRowVectorX> win;
	std::vector<Eigen::Matrix<double, 1, -1, 1>>g;
	std::vector<Eigen::Matrix<double, 1, -1, 1>>gsm;
	std::vector<Eigen::RowVectorXi> ind;

};