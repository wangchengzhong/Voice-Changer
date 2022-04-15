#include"HarmonicAnalysis.h"
HarmonicAnalysis::HarmonicAnalysis(Eigen::Ref<Eigen::RowVectorXi> pms, Eigen::TFloat fmax)
	:fmax(fmax),pms(pms)
{
	
}
PicosStructArray HarmonicAnalysis::processHarmonic(Eigen::Ref<const Eigen::TRowVectorX> x, Eigen::Ref<const Eigen::RowVectorXi> pms, Eigen::Ref<const Eigen::TRowVectorX> f0s)
{
	return picos;
}


