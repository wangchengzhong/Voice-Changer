#include"HSManalysis.h"

#include "StructDefinitions.h"
#include "types.h"

HSManalysis::HSManalysis(PicosStructArray& picos)
	:picos(picos)
	,f0Analysis(pms,f0min,f0max)
	,harmoniceAnalysis(pms,fmax)
	//,stochasticAnalysis(picos)
	,polarityAnalysis(picos)
	,decomposePhase(picos)
{
	
}
void HSManalysis::processHSManalysis(Eigen::Ref<Eigen::TRowVectorX> x)
{
	x *= 0.5 / x.maxCoeff();
	bufferLength = (int)x.size();
	temp = 1 + std::floor((bufferLength - 3.0 * 16000 / f0min) / N);
	pms = 1 + (int)std::ceil(1.5 * 16000 / f0min) + N * seq<Eigen::RowVectorXi>(0, temp).array();
	f0s = f0Analysis.processF0(x, pms);
	// picos = harmoniceAnalysis.processHarmonic(x,pms,f0s);
	// stochasticAnalysis.processStochastic(picos);
	//polarityAnalysis.processPolarity(picos);
	//decomposePhase.processDecompose(picos);
}
