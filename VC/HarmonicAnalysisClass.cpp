#include"HarmonicAnalysisClass.h"

#include "angle.h"
#include "constants.h"
#include"ppl.h"
#include "seq.h"

HarmonicAnalysis::HarmonicAnalysis(Eigen::RowVectorXi& pms, Eigen::TFloat fmax, PicosStructArray picos)
	:fmax(fmax),pms(pms),picos(picos),pmsSize(pms.size())
{
	timesPerThread = static_cast<int>(static_cast<float>(pms.size()) / (float)threadNum + 1);
	Lw.resize(threadNum); for (auto& t : Lw) { t = 0; }
	Lw2.resize(threadNum); for (auto& t : Lw2) { t = 0; }
	// trama2T.resize(threadNum);

	gv.resize(threadNum);
	win.resize(threadNum);
	K.resize(threadNum); for (auto& t : K) { t = 0; }
	h.resize(threadNum);
	// i1 = (0, 1);
	t1.resize(threadNum);
	t2.resize(threadNum);
	t3.resize(threadNum);
	coef.resize(threadNum);
	// coef1K.resize(threadNum);
}

PicosStructArray HarmonicAnalysis::processHarmonic(const Eigen::TRowVectorX& x, const Eigen::TRowVectorX f0s)
{
	//for (size_t m = (0); m < threadNum; m++)
	concurrency::parallel_for(size_t(0), (size_t)threadNum, [&](size_t m)
		{
			for(int k = m * timesPerThread+1; k < (m + 1) * timesPerThread + 1; k++)
			{
				if(k<=pmsSize)
				{
					picos[k - 1].pm = pms(k - 1);
					picos[k - 1].f0 = f0s(k - 1);
					if(f0s(k-1)>0)
					{
						Lw[m] = (int)std::ceil(2.2 * fs / std::min(f0s(k - 1), Eigen::TFloat(150.0)));
						Lw2[m] = (int)std::floor(Lw[m] / 2.0);
						Lw[m] = 2 * Lw2[m] + 1;
						auto trama2T = x.segment(pms(k - 1) - Lw2[m] - 1, 2 * Lw2[m] + 1);

						gv[m] = seq(-Lw2[m], Lw2[m]);
						win[m] = ((gv[m] * pi / Lw2[m]).array().cos() * 0.46 + 0.54).transpose();

						K[m] = (int)std::ceil(fmax / f0s(k - 1)) - 1;
						h[m].resize(Lw[m], 2 * K[m]);
						h[m].setZero();
						std::complex<Eigen::TFloat> i1(0, 1);
						h[m].col(0) = (gv[m] / fs * i1 * 2 * pi * f0s(k - 1)).array().exp().transpose();
						for(int kk = 2; kk <= K[m]; kk++)
						{
							h[m].col(kk - 1) = h[m].col(kk - 2).cwiseProduct(h[m].col(0));
						}
						for(int kk = 1; kk <=  K[m]; kk++)
						{
							h[m].col(kk - 1) = h[m].col(kk - 1).cwiseProduct(win[m]);
						}
						h[m].rightCols(K[m]) = h[m].leftCols(K[m]).conjugate();

						t1[m] = h[m].adjoint().eval();
						t2[m] = (t1[m] * h[m]).eval();
						t3[m] = (t1[m] * trama2T.transpose().cwiseProduct(win[m])).eval();
						coef[m] = t2[m].llt().solve(t3[m]).eval(); 
						auto coef1K = coef[m].head(K[m]);

						picos[k - 1].a = 2 * coef1K.cwiseAbs().transpose();
						picos[k - 1].p = angle(coef1K.transpose());
					}

				}
			}
		});
	

	return picos;
}
void HarmonicAnalysis::updateSize(Eigen::Ref<Eigen::RowVectorXi> pms)
{
	timesPerThread = static_cast<int>(static_cast<float>(pms.size()) / (float)threadNum + 1);
}


