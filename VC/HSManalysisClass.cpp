#include"HSManalysisClass.h"
#include "StructDefinitions.h"
#include "types.h"
#include"JuceHeader.h"
HSManalysis::HSManalysis(Eigen::RowVectorXi& pms, PicosStructArray& picos)
	:pms(pms)
	, picos(picos), f0s(pms.size())
	,f0Analysis(pms,f0min,f0max)
	, harmonicAnalysis(pms,fmax,picos)
	,stochasticAnalysis(picos, bufferLength)
	,polarityAnalysis(picos)
	,decomposePhase(picos)
	
{

}


PicosStructArray HSManalysis::processHSManalysis(Eigen::TRowVectorX& x)
{
	x *= 0.5 / x.maxCoeff();
	bufferLength = (int)x.size();
	// DBG(bufferLength);
	temp = 1 + std::floor((bufferLength - 3.0 * 16000 / f0min) / N);// 1 +

	pms = 1 + (int)std::ceil(1.5 * 16000 / f0min) + N * seq<Eigen::RowVectorXi>(0, temp).array();
	//f0s.resize(pms.size());
	f0s = f0Analysis.processF0(x, pms);
	//for (auto& i : f0s)
	//	DBG(i);
	picos = harmonicAnalysis.processHarmonic(x,f0s);
	stochasticAnalysis.processStochastic(x);
	polarityAnalysis.processPolarity(picos);
	decomposePhase.processDecompose(picos);
	return picos;
}
