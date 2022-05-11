#include "HSMsynthesizeClass.h"

#include "concat.h"
#include "constants.h"
#include "ppl.h"
#include "seq.h"

HSMsynthesize::HSMsynthesize(PicosStructArray& picos, int bufferLength, Eigen::TRowVectorX& output)
	:picos(picos),bufferLength(bufferLength),Npm(0)
	, timesPerThread(static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1))
	,y(output)
{
	y.resize(bufferLength);
	y.setZero();
	
	gen.seed(rd());

	n2.resize(threadNum);
	n1.resize(threadNum);
	n3.resize(threadNum);
	N12.resize(threadNum);
	N23.resize(threadNum);

	seq1.resize(threadNum);
	seq2.resize(threadNum);
	win.resize(threadNum);
	trama.resize(threadNum);
	x.resize(threadNum);
	na_filter.resize(threadNum);
	y_filter.resize(threadNum);
	an_filter.resize(threadNum);
}


Eigen::TRowVectorX HSMsynthesize::filter(const Eigen::TRowVectorX& a, const Eigen::TRowVectorX& x, int m)
{
	na_filter[m] = a.size() - 1;
	an_filter[m] = a / a(0);
	y_filter[m].setZero(x.size());
	for(Eigen::Index i = 0; i < na_filter[m]; i++)
	{
		y_filter[m](i) = x(i) / a(0) - an_filter[m].segment(1, i).dot(y.head(i).reverse());
	}
	for(Eigen::Index i = na_filter[m]; i< y_filter[m].size(); i++)
	{
		y_filter[m](i) = x(i) / a(0) - an_filter[m].segment(1, na_filter[m]).dot(y_filter[m].segment(i - na_filter[m], na_filter[m]).reverse());
	}
	return y_filter[m];
}

Eigen::TRowVectorX HSMsynthesize::processSynthesize(PicosStructArray& picos, int bufferLength)
{
	Npm = picos.size();
	// const auto& picos = picos;
	concurrency::parallel_for(size_t(0), size_t(threadNum), [&](size_t m)
		{
			for(size_t k = m*timesPerThread+1;k<(m+1)*timesPerThread+1;k++)
			{
				if(k<=Npm)
				{
					n2[m] = picos[k - 1].pm;
					n1[m] = 0; n3[m] = 0;
					if(k==1)
					{
						n3[m] = picos[k].pm;
						n1[m] = std::max(1, n2[m] - (n3[m] - n2[m]));
					}
					else if(k==Npm)
					{
						n1[m] = picos[k - 2].pm;
						n3[m] = std::min(bufferLength, n2[m] + n2[m] - n1[m]);
					}
					else
					{
						n1[m] = picos[k - 2].pm;
						n3[m] = picos[k].pm;
					}
					N12[m] = n2[m] - n1[m];
					N23[m] = n3[m] - n2[m];

					seq1[m] = seq(0, 1.0 / N12[m], (N12[m] - 1) / (Eigen::TFloat)N12[m]);
					seq2[m] = seq(1.0 / N23[m], 1.0 / N23[m], 1).reverse();
					win[m] = concat<Eigen::TRowVectorX>(seq1[m], seq2[m]);
					trama[m].resize(n3[m] - n1[m]);
					trama[m].setZero();
					for(Eigen::Index j = 1; j <= picos[k-1].a.size();j++)
					{
						trama[m] += picos[k - 1].a(j - 1) * (2 * pi * seq(-N12[m], N23[m] - 1).array() * (j * picos[k - 1].f0) / fs + picos[k - 1].p(j - 1) + j * picos[k - 1].alfa).cos().matrix();
					}
					x[m].resize(n3[m] - n1[m]);
					for(Eigen::Index i = 0; i < x[m].size(); i++)
					{
						x[m](i) = nd(gen);
					}
					trama[m] += filter(picos[k - 1].e, x[m], m);
					y.segment(n1[m] - 1, n3[m] - n1[m]) += trama[m].cwiseProduct(win[m]);
				}
			}
		});
	return y;
}