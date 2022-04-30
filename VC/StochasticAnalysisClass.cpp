#include"StochasticAnalysisClass.h"

#include "bylevdurb.h"
#include "concat.h"
#include "constants.h"
#include "filter.h"
#include"ppl.h"
#include "seq.h"
#define Tolerance_double 1.0e-12

StochasticAnalysis::StochasticAnalysis(PicosStructArray& picos, int xLength)
	:picos(picos),left(0),size(0)
{
	yd.setZero(xLength);
	Npm = (int)picos.size();
	timesPerThread = static_cast<int>(static_cast<float>(Npm) / (float)threadNum + 1);
	seq1 = seq(0, N - 1) / Eigen::TFloat(N);
	seq3 = seq(0, N - 1);
	seq4 = seq(1, N).reverse();
	seq5 = seq(-N, -1);

	//trama.resize(N);
	//R.resize(1 + ordenLPC);

	minSize.resize(threadNum);
	a1.resize(threadNum);
	a2.resize(threadNum);
	c0.resize(threadNum);c1.resize(threadNum);c2.resize(threadNum);c3.resize(threadNum);
	c01.resize(threadNum); c11.resize(threadNum); c21.resize(threadNum); c31.resize(threadNum);
	w1.resize(threadNum);
	w2.resize(threadNum);
	M.resize(threadNum);

	trama.resize(threadNum);
	for(auto& i: trama)
	{
		i.setZero(N);
	}
	R.resize(threadNum);
	for(auto& i: R)
	{
		i.setZero(1 + ordenLPC);
	}

	
}

void StochasticAnalysis::updateSize(PicosStructArray& picos)
{
	Npm = picos.size();
	timesPerThread = static_cast<int>(static_cast<float>(Npm) / (float)threadNum + 1);
}

std::tuple<Eigen::TFloat, Eigen::TFloat, Eigen::TFloat, Eigen::TFloat> StochasticAnalysis::bymcaquat(Eigen::TFloat p1, Eigen::TFloat p2, Eigen::TFloat f1, Eigen::TFloat f2, Eigen::TFloat N, Eigen::TFloat fs, size_t m)
{
	w1[m] = 2 * pi * f1 / fs;
	w2[m] = 2 * pi * f2 / fs;
	M[m] = round((1 / (2 * pi)) * ((p1 + w1[m] * N - p2) + 0.5 * N * (w2[m] - w1[m])));
	c01[m] = p1;
	c11[m] = w1[m];
	c21[m] = (3 / (N * N)) * (p2 - p1 - w1[m] * N + 2 * pi * M[m]) - (w2[m] - w1[m]) / N;;
	c31[m] = (-2 / (N * N * N)) * (p2 - p1 - w1[m] * N + 2 * pi * M[m]) + (w2[m] - w1[m]) / (N * N);
	return std::make_tuple(c01[m], c11[m], c21[m], c31[m]);
}
void StochasticAnalysis::processStochastic(Eigen::Ref<const Eigen::TRowVectorX> x)//, int N, PicosStructArray& picos, int ordenLPC)
{
	//yd.resize(x.size());
	//yd.setZero();

	left = picos[0].pm - N - 1;
	size = picos[0].a.size();

	seq2 = (2 * pi / fs * picos[0].f0) * seq(-N, -1);
	auto seq5 = seq(-N, -1);
	for(int j = 1; j<=size;j++)
	{
		yd.segment(left, N) += ((picos[0].a(j - 1) * seq1).array() * (j * seq2.array() + picos[0].p(j - 1)).cos()).matrix();
	}

	concurrency::parallel_for(size_t(0), (size_t)threadNum, [&](size_t m)
	{
		for(int k = m*timesPerThread;k<(m+1)*timesPerThread;k++)
		{
			if(k==0)
				continue;
			if(k<Npm)
			{
				minSize[m] = std::min(picos[k - 1].a.size(), picos[k].a.size());
				for(int j = 1; j <= minSize[m]; j++)
				{
					a1[m] = picos[k - 1].a(j - 1);
					a2[m] = picos[k].a(j - 1);
					std::tie(c0[m],c1[m],c2[m],c3[m]) = bymcaquat(picos[k - 1].p(j - 1), picos[k].p(j - 1), j * picos[k - 1].f0, j * picos[k].f0, N, fs, m);
					yd.segment(picos[k - 1].pm - 1, N).array() += (a1[m] + ((a2[m] - a1[m]) / N) * seq3.array()) * (c0[m] + seq3.array() * (c1[m] + seq3.array() * (c2[m] + c3[m] * seq3.array()))).cos();
				}
				for(int j = (int)picos[k].a.size()+1;j<=picos[k-1].a.size();j++)
				{
					yd.segment(picos[k - 1].pm - 1, N).array() += (picos[k - 1].a(j - 1) / N * seq4.array())
						* ((2 * pi * j * picos[k - 1].f0 / fs) * seq3.array() + picos[k - 1].p(j - 1)).cos();
				}
				for (int j = (int)picos[k - 1].a.size() + 1; j <= picos[k].a.size(); j++)
				{
					yd.segment(picos[k].pm - N - 1, N).array() += (picos[k].a(j - 1) / N * seq3).array() * (2 * pi * j * picos[k].f0 / fs * seq5.array() + picos[k].p(j - 1)).cos();
				}
			}
		}
	});

	Eigen::TRowVectorX ye = concat(x - yd, Eigen::TRowVectorX::Zero(24));
	filter(ye);
	ye.tail(24).setZero();
	Eigen::VectorBlock<Eigen::TRowVectorX> ye_ = ye.tail(ye.size() - 24);
	hnnN = std::sqrt(8.0 / 3.0) * (0.5 - 0.5 * (2 * pi * seq1).array().cos());

	concurrency::parallel_for(size_t(0), (size_t)(threadNum), [&](size_t m)
	{
		for(int k = m*timesPerThread+1;k<(m+1)*timesPerThread+1;k++)
		{
			if (k <= Npm)
			{
				trama[m] = ye_.segment(picos[k - 1].pm - static_cast<int>(std::floor(N / 2.0)) - 1, N).cwiseProduct(hnnN);
				if (trama[m].isZero(Tolerance_double))
				{
					picos[k - 1].e = Eigen::TRowVectorX::Zero(ordenLPC + 1);
					picos[k - 1].e(0) = std::numeric_limits<Eigen::TFloat>::infinity();
				}
				else
				{
					R[m].setZero();
					for(int j = 0; j<=ordenLPC; j++)
					{
						R[m](j) = trama[m].tail(N - j).dot(trama[m].head(N - j));

					}
					picos[k-1].e = std::sqrt(N)* bylevdurb(R[m]);
				}
			}
		}
	});
}


