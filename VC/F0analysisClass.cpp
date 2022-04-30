#include"F0analysisClass.h"
#include"ppl.h"
#include "sliceByIndices.h"
#include"almostEqual.h"
#include"JuceHeader.h"
F0analysis::F0analysis(
	// Eigen::Ref<Eigen::TRowVectorX> x, 
	Eigen::RowVectorXi& pms, 
	Eigen::TFloat f0min, 
	Eigen::TFloat f0max)
		:pms(pms),f0min(f0min),f0max(f0max)
{
	timesPerThread = static_cast<int>(static_cast<float>(pms.size()) / (float)threadNum + 1);
	lagmin = static_cast<int>(std::ceil(fs / f0max));
	lagmax = static_cast<int>(std::floor(fs / f0min));
	fact = 0.01 * fs / (pms(1) - pms(0));
	vuvcost = 0.14 * fact;
	uvvcost = 0.14 * fact;
	octjump = 0.35 * fact;

	L = static_cast<int>(std::ceil(3.0 * fs / f0min));
	if ((L % 2) == 0)
	{
		L2 = L / 2;
		L++;
	}
	else
	{
		L2 = (L - 1) / 2;
	}
	Lz = static_cast<int>(std::ceil(1.5 * L));
	while(Lp2<Lz)  
	{
		Lp2 *= 2;
	}
	Lxlmax = static_cast<int>(std::ceil(0.5 * fs / f0min));
	Ldc = 2 * Lxlmax;

	w = Eigen::TRowVectorX::Zero(Lp2);
	w.head(L) = 0.5 * (1 - (2 * pi * seq(0, L - 1).array() / L).cos());

	tt = fft(w).array().abs2().matrix().cast<std::complex<Eigen::TFloat>>().eval();

	rw_temp = ifft(tt).real().eval();
	///////////
	rw = (1 / rw_temp(0)) * rw_temp.head(L2);
	xgmax = 0.0;////// x.cwiseAbs().maxCoeff();
	dat.resize(pms.size());

	rx.resize(threadNum);
	for(auto& t:rx)
	{
		t.resize(L2);
	}

	//rk.resize(Ncandidates - 1);
	//fk.resize(Ncandidates - 1);
	

	rk.resize(threadNum);
	fk.resize(threadNum);
	for(auto& t:rk)
	{
		t = Eigen::TRowVectorX::Zero(Ncandidates - 1);
		//t.setZero();
	}
	for(auto& t:fk)
	{
		t = Eigen::TRowVectorX::Zero(Ncandidates - 1);
		t.setZero();
	}

	trama.resize(threadNum);
	tramaTemp.resize(threadNum);

	innerLength.resize(threadNum);
	xlmax.resize(threadNum);

	ftemp.resize(threadNum);
	ra_.resize(threadNum);
	ra.resize(threadNum);

	tmax.resize(threadNum);
	rmax.resize(threadNum);
	fmax.resize(threadNum);
	jj.resize(threadNum);
	rmin.resize(threadNum);

	ruv.resize(threadNum);

	f0s.resize(pms.size());
	f0s.setZero();
}


Eigen::TRowVectorX F0analysis::processF0(const Eigen::Ref<const Eigen::TRowVectorX>& x, const Eigen::Ref<const Eigen::RowVectorXi>& pms)//(Eigen::Ref<Eigen::TRowVectorX> x, int fs, Eigen::Ref<Eigen::RowVectorXi> pms, Eigen::Ref<Eigen::TFloat> f0min, Eigen::Ref<Eigen::TFloat> f0max)
{
	if (pms.size() != this->pms.size())
		updateSize(pms);
	xgmax = x.cwiseAbs().maxCoeff();
	DBG(x.size());
	DBG(pms.size());
	concurrency::parallel_for(size_t(0), (size_t)threadNum, [&](size_t m)
	//for(size_t m = (0); m<(size_t)threadNum;m++)
	{
		for(Eigen::Index k = m*timesPerThread+1;k<(m+1)*timesPerThread+1;k++)
		{
			if (k <= pms.size())
			{
				//Eigen::TRowVectorX trama;
				if (pms(k - 1) - L2 < 1)
				{
					tramaTemp[m] = x.head(pms(k - 1) + L2);
					trama[m] = concat(Eigen::TRowVectorX::Zero(L - tramaTemp[m].size()), tramaTemp[m]);
				}
				else if(pms(k-1)+L2>x.size())
				{
					innerLength[m] = (int)x.size() - (pms(k - 1) - L2) + 1;
					auto tramaTemp = x.tail(innerLength[m]);
					trama[m] = concat(tramaTemp, Eigen::TRowVectorX::Zero(L - tramaTemp.size()));
				}
				else
				{
					trama[m] = x.segment(pms(k - 1) - L2 - 1, 2 * L2 + 1);
				}

				trama[m].array() -= trama[m].segment(L2 - Ldc, 2 * Ldc + 1).sum() / (2 * Ldc + 1.0);
				xlmax[m] = trama[m].segment(L2 - Lxlmax, 2 * Lxlmax + 1).cwiseAbs().maxCoeff();

				ftemp[m] = 
					fft(concat(trama[m], Eigen::TRowVectorX::Zero(Lp2 - L))
						.cwiseProduct(w)
						.eval()).array().abs2().matrix()
						.cast<std::complex<Eigen::TFloat>>().eval();
				ra_[m] = ifft(ftemp[m]).real().eval();
				ra[m] = (1 / ra_[m](0)) * ra_[m].head(L2);

				rx[m] = ra[m].cwiseQuotient(rw).cwiseMax(0.0);
				rk[m].setZero(Ncandidates-1);
				fk[m].setZero(Ncandidates-1);
				for(int j = lagmin+1; j<=lagmax+1;j++)
				{
					if(rx[m](j-1)>0.5*voith&&rx[m](j-2)<rx[m](j-1)&&rx[m](j)<rx[m](j-1))
					{
						tmax[m] = 
							0.5 * (rx[m](j - 2) - rx[m](j)) / 
							(rx[m](j - 2) - 2.0 * rx[m](j - 1) + rx[m](j));
						rmax[m] = rx[m](j - 1) + 
							0.5 * tmax[m] * (rx[m](j) - rx[m](j - 2) + 
								tmax[m] * (rx[m](j - 2) - 2 * rx[m](j - 1) 
									+ rx[m](j)));
						if (rmax[m] > 1)
							rmax[m] = 1 / rmax[m];
						tmax[m] = (tmax[m] + j - 1) / fs;
						fmax[m] = 1 / tmax[m];
						rmax[m] = rmax[m] - octcost * std::log2(f0min * tmax[m]);
						//jj[m] = -1;
						Eigen::Index jjj = -1;
						rmin[m] = (rk[m]).minCoeff(&jjj);
						if(rmax[m] > rmin[m])
						{
							rk[m](jjj) = rmax[m];
							fk[m](jjj) = fmax[m];
						}
					}
				}

				ruv[m] = voith + std::max<Eigen::TFloat>(Eigen::TFloat(0.0), Eigen::TFloat(2.0) - std::min(Eigen::TFloat(1.0), xlmax[m] / xgmax) / (silth / (1.0 + voith)));
				auto jj = fk[m].array() > 0;
				dat[k - 1].r = concat(indexByLogical(rk[m], jj) 
					+ octcost * (f0min / indexByLogical(fk[m], jj).array()).unaryExpr([](const auto& e) 
						{return std::log2(e); }).matrix(), ruv[m]);
				dat[k - 1].f = concat(indexByLogical(fk[m], jj), 0);
				dat[k - 1].ac = Eigen::TRowVectorX::Zero(dat[k - 1].f.size());
				dat[k - 1].ant = Eigen::RowVectorXi::Zero(dat[k - 1].f.size());
			}
			

		}
		
	}
	);
	dat[0].ac = -dat[0].r;
	for (int k = 2; k <= pms.size(); k++)
	{
		for (int j = 1; j <= dat[k - 1].f.size(); j++)
		{
			mincost = std::numeric_limits<Eigen::TFloat>::infinity();
			jjmincost = -1;
			for (int jj = 1; jj <= dat[k - 2].f.size(); jj++)
			{
				cost = dat[k - 2].ac(jj - 1) - dat[k - 1].r(j - 1);
				if (almostEqual(dat[k - 1].f(j - 1), 0) && almostEqual(dat[k - 2].f(jj - 1), 0))
					cost = cost + uvuvcost;
				else if (almostEqual(dat[k - 1].f(j - 1), 0))
					cost = cost + vuvcost;
				else if (almostEqual(dat[k - 2].f(jj - 1), 0))
					cost = cost + uvvcost;
				else
					cost = cost + octjump * std::abs(std::log2(dat[k - 1].f(j - 1) / dat[k - 2].f(jj - 1)));
				if (cost < mincost)
				{
					mincost = cost;
					jjmincost = jj;
				}
			}
			dat[k - 1].ac(j - 1) = mincost;
			dat[k - 1].ant(j - 1) = jjmincost;
		}
	}

	//f0s(pms.size());
	f0s.setZero();
	// [mincost,jjmincost]=min(dat(length(dat)).ac);
	jjmincostFinal = -1;
	dat.back().ac.minCoeff(&jjmincostFinal);
	// since jjmincost is acquired through Eigen, it is already 0-based indexing.
	f0s(f0s.size() - 1) = dat.back().f(jjmincostFinal);
	jjmincostFinal = dat.back().ant(jjmincostFinal); // ant contains 1-based indices.Thereafter, jjmincost is 0-based
	for (auto k = pms.size() - 1; k >= 1; k--)
	{
		f0s(k - 1) = dat[k - 1].f(jjmincostFinal - 1);
		jjmincostFinal = dat[k - 1].ant(jjmincostFinal - 1);
	}

	return f0s;
}
void F0analysis::updateSize(const Eigen::Ref<const Eigen::RowVectorXi>& pms)
{
	fact = 0.01 * fs / (pms(1) - pms(0));
	vuvcost = 0.14 * fact;
	uvvcost = 0.14 * fact;
	octjump = 0.35 * fact;
	dat.resize(pms.size());
	timesPerThread = static_cast<int>(static_cast<float>(pms.size()) / (float)threadNum + 1);

	f0s.resize(pms.size());
	f0s.setZero();
}

