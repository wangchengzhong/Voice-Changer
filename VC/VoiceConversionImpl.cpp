#include"VoiceConversionImpl.h"
VoiceConversionImpl::VoiceConversionImpl(HSMModel& model)
	:hsmAnalysis(picos)
	,hsmWfwConvert(model)
	,hsmSynthesize()
{

}
void VoiceConversionImpl::processConversion(std::vector<double>& origBuffer, std::vector<double>& convertedBuffer, int verbose) noexcept
{
	bufferLength = origBuffer.size();
	hsmAnalysis.processHSManalysis(x);
	hsmWfwConvert.processWfwConvert(picos);
	hsmSynthesize.processSynthesize(picos,bufferLength);
}

