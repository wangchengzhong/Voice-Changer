#include"DecomposePhaseClass.h"

#include"ppl.h"
#include "seq.h"
#include "sumcos.h"

DecomposePhase::DecomposePhase(PicosStructArray& picos)
	:picos(picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
	linpAlfa.resize(threadNum);
	K.resize(threadNum); for (auto& t : K) { t = 0; }
	
	alfa.resize(threadNum); for (auto& t : alfa) { t = 0.0; }

	imax.resize(threadNum); for (auto& t : imax) { t = 0; }
}

void DecomposePhase::updateSize(PicosStructArray& picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
}

Eigen::TFloat DecomposePhase::linphaseterm(Eigen::Ref<const Eigen::TRowVectorX> aa, Eigen::Ref<const Eigen::TRowVectorX> pp, int fmaxopt, size_t m)
{
	K[m] = (int)std::ceil(aa.size() * fmaxopt / fmax);
	const auto aa_ = aa.head(K[m]);
	const auto pp_ = pp.head(K[m]);
	linpAlfa[m] = sumcos(seq(1, K[m]).cwiseProduct(aa_), pp_, Modo::sin);
	if (linpAlfa[m].size() > 1)
	{
		imax[m] = 0;
		linpAlfa[m].unaryExpr([&aa_, &pp_](Eigen::TFloat element) {
			return aa_.dot((seq(1, (double)pp_.size()) * element + pp_).array().cos().matrix());
			}).maxCoeff(&imax[m]);
			return linpAlfa[m](imax[m]);
	}
	else if (linpAlfa[m].size() == 1)
		return linpAlfa[m].value();
	else
	{
		return 0;
	}
}

void DecomposePhase::processDecompose(PicosStructArray& picos)
{
	concurrency::parallel_for(size_t(0), size_t(threadNum), [&](size_t m)
	{
		for(int i = m*timesPerThread; i < (m+1)*timesPerThread;i++)
		{
			if(i<picos.size())
			{
				auto& pk = picos[i];
				if(pk.f0>0)
				{
					alfa[m] = linphaseterm(pk.a, pk.p, fmaxopt,m);
					pk.alfa = -alfa[m];
					pk.p += seq(1, (int)pk.p.size()) * alfa[m];
				}
			}
		}
	});
}


