#include"VoiceConversionImpl.h"
VoiceConversionImpl::VoiceConversionImpl(HSMModel& model)
	:picos(pms.size())
	,hsmAnalysis(pms)
	,hsmWfwConvert(model,picos)
	,hsmSynthesize()
{

}
void VoiceConversionImpl::processConversion(std::vector<double>& origBuffer, std::vector<double>& convertedBuffer, int verbose) noexcept
{
	bufferLength = origBuffer.size();
	picos = hsmAnalysis.processHSManalysis(x);
	hsmWfwConvert.processWfwConvert(picos);
	hsmSynthesize.processSynthesize(picos,bufferLength);
}

