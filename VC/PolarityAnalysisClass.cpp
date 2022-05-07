#include"PolarityAnalysisClass.h"
#include"ppl.h"
#include"constants.h"
PolarityAnalysis::PolarityAnalysis(PicosStructArray& picos)
	:picos(picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
	E.resize(threadNum); for (auto& t : E) { t = 0.0; }
	dP.resize(threadNum); for (auto& t : dP) { t = 0.0; }
	dN.resize(threadNum); for (auto& t : dN) { t = 0.0; }
	alfa.resize(threadNum); for (auto& t : alfa) { t = 0.0; }
}

void PolarityAnalysis::updateSize(PicosStructArray& picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
}

int PolarityAnalysis::processPolarity(PicosStructArray picos)
{
	concurrency::parallel_for(size_t(0), (size_t)threadNum, [&](size_t m)
	{
		for(int k = m*timesPerThread+1; k<(m+1)*timesPerThread+1;k++)
		{
			if(k<=picos.size())
			{
				// int pol = 0; int polE = 0;
				if(picos[k-1].f0>0)
				{
					E[m] = picos[k - 1].a * picos[k - 1].a.transpose();
					alfa[m] = 2 * picos[k - 1].p(0) - picos[k - 1].p(1);
					alfa[m] = alfa[m] - 2 * pi * std::floor(alfa[m] / (2 * pi));
					dP[m] = std::min(std::abs(alfa[m]), std::abs(2 * pi - alfa[m]));
					dN[m] = std::abs(alfa[m] - pi);
					if (dN[m] < dP[m])
					{
						juce::ScopedLock s1(minusLock);
						pol = pol - 1;
						polE = polE - E[m];
					}
					else if(dP[m]<dN[m])
					{
						juce::ScopedLock s1(minusLock);
						pol = pol + 1;
						polE = polE + E[m];
					}
				}
			}
		}
	});
	if (pol < 0)
		pol = -1;
	else if (pol > 0)
		pol = 1;
	else if (polE < 0)
		pol = -1;
	else
		pol = 1;
	
	if(pol==-1)
	{
		for(PicosElement& element:picos)
		{
			if (element.f0 > 0)
				element.p.array() += pi;
		}
	}
	return pol;
}
