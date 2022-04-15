#include "decomposephase.h"
#include "seq.h"
#include "linphaseterm.h"
#include "ppl.h"
void decomposephase(PicosStructArray& picos)
{
	int fmaxopt = 1000;

	// Like MATLAB, iterate over picos
	auto a = picos.size();
	auto t = (int)(a / 20);
	concurrency::parallel_for(size_t(0), (size_t)20, [&](size_t m)
		{
			for (int i = m * t; i < (m + 1) * t; i++)
				//for (int i = 0; i < picos.size(); i++)
				{
					auto& pk = picos[i];
					if (pk.f0 > 0)
					{
						Eigen::TFloat alfa = linphaseterm(pk.a, pk.p, fmaxopt);
						pk.alfa = -alfa;
						pk.p += seq(1, (int)pk.p.size()) * alfa;
					}
				}
		});
	for (int i = 20 * t; i < picos.size(); i++)
		//for (int i = 0; i < picos.size(); i++)
	{
		auto& pk = picos[i];
		if (pk.f0 > 0)
		{
			Eigen::TFloat alfa = linphaseterm(pk.a, pk.p, fmaxopt);
			pk.alfa = -alfa;
			pk.p += seq(1, (int)pk.p.size()) * alfa;
		}
	}
}
