#include"VoiceConversionImpl.h"
//#include <ctime>
//#include <iostream>
//using namespace std;
//int main()
//{
//	srand((unsigned)time(NULL));
//
//	for (int i = 0; i < 5; i++)
//	{
//		cout << (float)rand() / RAND_MAX << endl;
//	}
//	return 0;
//}
VoiceConversionImpl::VoiceConversionImpl(HSMModel model, Eigen::RowVectorXi pms, Eigen::TRowVectorX x, PicosStructArray picos)
	:
	sampleRate(16000)
	,model(model)
	,x(x)
	,pms(pms)
	,picos(picos)
	,hsmAnalysis(this->pms,this->picos)
	,hsmWfwConvert(model,this->picos)
	, output(bufferLength)
	,hsmSynthesize(this->picos, bufferLength,output)
	
{
}


void VoiceConversionImpl::processConversion(std::vector<double>& origBuffer, std::vector<double>& convertedBuffer, int verbose) noexcept
{
	//picos.clear();
	bufferLength = origBuffer.size();
	// x.resize(bufferLength);
	DBG("actual input buffer length:" << bufferLength);
	x.setZero(bufferLength);
	for(int i = 0; i < bufferLength; i++)
	{
		x(i) = origBuffer[i];
	}
	
	hsmAnalysis.processHSManalysis(x);
	for (auto& i : picos)
	{
		for (auto& j : i.p)
			DBG(" class: " << j);
	}
	hsmWfwConvert.processWfwConvert(picos);
	
	hsmSynthesize.processSynthesize(picos,bufferLength);
	for(int i = 0; i < bufferLength; i++)
	{
		convertedBuffer[i] = output(i);
	}
}

