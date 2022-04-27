#include"HSMwfwConvert.h"

#include "angle.h"
#include "concat.h"
#include "constants.h"
#include "poly.h"
#include"ppl.h"
#include "roots.h"
#include "seq.h"


HSMwfwConvert::HSMwfwConvert(HSMModel& model, PicosStructArray& picos)
	:model(model),picos(picos)
{
	//i1 = (0.0, 1.0);
	f01s(picos.size());
	f01s.setZero();
	f02s = f01s;

	std::transform(th.cbegin(), th.cend(), thd.begin(), [](const ThxyElementType& e) {return e.E.determinant(); });
	std::transform(th.cbegin(), th.cend(), thI.begin(), [](const ThxyElementType& e) {return e.E.inverse(); });


	std::transform(th2.cbegin(), th2.cend(), th2d.begin(), [](const ThxyElementType& e) {return e.E.determinant(); });
	std::transform(th2.cbegin(), th2.cend(), th2I.begin(), [](const ThxyElementType& e) {return e.E.inverse(); });

	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);

	P.resize(threadNum);
	PP.resize(threadNum);
	P2.resize(threadNum);
	for(auto i:P)
	{
		i(m);
	}
	for(auto i:PP)
	{
		i(mm);
	}
	for(int i = 0; i < threadNum;i++)
	{
		P2[i] = P[i];
 	}

	lsfNk.resize(threadNum);
	lsfff.resize(threadNum);
	lsfPP.resize(threadNum);
	lsfR.resize(threadNum);
	lsfai.resize(threadNum);
	lsfe.resize(threadNum);
	lsfk.resize(threadNum);
	lsfaz1.resize(threadNum);
	//lsfaz2.resize(threadNum);
	lsflsf.resize(threadNum);
	lsfSize.resize(threadNum);
	for(auto i: lsfR)
	{
		i = Eigen::TRowVectorX::Zero(p + 1).eval();
	}
	for(auto i:lsfai)
	{
		i(p);
		i.setZero();
	}
	for(auto i:lsfaz1)
	{
		i(p + 2);
	}

	v.resize(threadNum);
	Psum.resize(threadNum);
	vt.resize(threadNum);

	adaps1 = seq<Eigen::RowVectorXi>(2, 2, p);
	adaps2 = seq<Eigen::RowVectorXi>(1, 2, p);

	adapPart1.resize(threadNum);
	adapPart2.resize(threadNum);
	for(auto i: adapPart1)
		i(adaps1.size());
	for(auto i: adapPart2)
		i(adaps2.size());

	temp1.resize(threadNum);
	for(auto i: temp1)
	{
		i(adapPart1[0].size() * 2 + 1);
		i(0) = 1;
	}
	for(auto i:temp2)
	{
		i(adapPart2[0].size() * 2 + 1);
	}
	
	adapai.resize(threadNum);

	aivt.resize(threadNum);
	eevt.resize(threadNum);

	Afvt.resize(threadNum);
	E1.resize(threadNum);
	Evt.resize(threadNum);
	aavt.resize(threadNum);

	vtt.resize(threadNum);
	vttaux.resize(threadNum);
	aivtt.resize(threadNum);

	fy.resize(threadNum);
	for(auto i:fy)
	{
		i(fw[0].y.size());
		i.setZero();
	}

	ff1.resize(threadNum);
	ap1.resize(threadNum);
	aa1.resize(threadNum);

	aa2.resize(threadNum);
	pp2.resize(threadNum);

	jjant1.resize(threadNum);
	jjant2.resize(threadNum);
	for (auto i : jjant1) i = 1;
	for (auto i : jjant2) i = 1;
	scale.resize(threadNum);
	for (auto i : scale) i = 0.0;

	f2j.resize(threadNum);
	f1j.resize(threadNum);
	jj.resize(threadNum);
	for (auto i : jj) i = 0;

	nf.resize(threadNum);
	seqnf.resize(threadNum);
	win.resize(threadNum);
}
Eigen::TVectorX HSMwfwConvert::aaalsf(Eigen::Ref<const Eigen::TRowVectorX> aa, Eigen::TFloat f0, size_t aaa)
{
	lsfNk[aaa] = (int)aa.size();
	lsfff[aaa] = (seq(1, lsfNk[aaa]) * f0).eval();
	PP[aaa] = aa.array().square().eval();
	for(int j = 1; j <= p+1; j++)
	{
		lsfR[aaa](j - 1) = (1.0 / lsfNk[aaa]) * (lsfPP[aaa] * (2 * pi * (j - 1) / fs* lsfff[aaa]).array().cos()).sum();
	}
	
	lsfe[aaa] = lsfR[aaa](0);
	for(int j = 1; j<=p; j++)
	{
		if(j==1)
		{
			lsfk[aaa] = lsfR[aaa](1) / lsfR[aaa](0);
			lsfai[aaa](0) = lsfk[aaa];
			lsfe[aaa] = (1 - lsfk[aaa] * lsfk[aaa]) * lsfe[aaa];
		}
		else
		{
			lsfk[aaa] = lsfR[aaa](j);
			lsfk[aaa] = lsfk[aaa] - lsfai[aaa].head(j - 1).dot(lsfR[aaa].segment(1, j - 1).reverse());
			lsfk[aaa] = lsfk[aaa] / lsfe[aaa];
			lsfai[aaa](j - 1) -= lsfk[aaa];
			lsfai[aaa].head(j - 1) = lsfai[aaa].head(j - 1) - lsfk[aaa] * lsfai[aaa].head(j - 1).reverse();
			lsfe[aaa] = (1 - lsfk[aaa] * lsfk[aaa]) * lsfe[aaa];
		}
	}
	lsfaz1[aaa](0) = 1;
	lsfaz1[aaa].segment(1, p) = -lsfai[aaa];
	lsfaz1[aaa](p + 1) = 0;

	auto lsfaz2 = lsfaz1[aaa].reverse();
	lsflsf[aaa] = angle(concat<Eigen::TRowVectorXc>(roots(lsfaz1[aaa] + lsfaz2).transpose(), roots(lsfaz1[aaa] - lsfaz2).transpose())).eval();
	lsfSize[aaa] = (int)lsflsf[aaa].size();
	std::sort(lsflsf[aaa].data(), lsflsf[aaa].data() + lsfSize[aaa]);
	return lsflsf[aaa].segment(lsfSize[aaa] - p - 1, p).transpose();
}

Eigen::TRowVectorX HSMwfwConvert::lsfadap(Eigen::Ref<const Eigen::TVectorX> lsf, size_t aaa)
{
	int p = (int)lsf.size();
	for(Eigen::Index j = 0; j < adaps1.size(); j++)
	{
		adapPart1[aaa](j) = std::exp(i1 * lsf(adaps1(j) - 1));
	}
	for(Eigen::Index j = 0; j < adaps2.size(); j++)
	{
		adapPart2[aaa](j) = std::exp(i1 * lsf(adaps2(j) - 1));
	}
	temp1[aaa].segment(1, adapPart1[aaa].size()) = adapPart1[aaa];
	temp1[aaa].tail(adapPart1[aaa].size()) = adapPart1[aaa];
	temp2[aaa].head(adapPart2[aaa].size()) = adapPart2[aaa];
	temp2[aaa](adapPart2[aaa].size()) = -1;
	temp2[aaa].tail(adapPart1[aaa].size()) = adapPart2[aaa].conjugate();
	adapai[aaa] = (0.5 * (poly(temp1[aaa]) + poly(temp2[aaa]))).real().eval();
	return adapai[aaa].head(adapai.size() - 1);
}


void HSMwfwConvert::processWfwConvert(PicosStructArray& picos)
{
	for(size_t k = 1; k <= picos.size(); k++)
	{
		f01s(k - 1) = picos[k - 1].f0;
		if (f01s(k - 1) > 0)
		{
			f02s(k - 1) = std::pow(10, u2 + v2 * (std::log10(f01s(k - 1)) - u1) / v1);
			if (k > 1 && f01s(k - 2) > 0)
			{
				dph = dph + (picos[k - 1].pm - picos[k - 2].pm) * pi * ((f02s(k - 1) - f01s(k - 1)) + (f02s(k - 2) - f01s(k - 2))) / fs;
				picos[k - 1].alfa += dph;
			}
			else
			{
				dph = 0;
			}
		}
		else
		{
			f02s(k - 1) = 0;
			dph = 0;
		}
	}
	concurrency::parallel_for(size_t(0), size_t(threadNum), [&](size_t aaa)
	{
		for(int k = aaa*timesPerThread+1; k<(aaa+1)*timesPerThread+1;k++)
		{
			if(k<=picos.size())
			{
				if(picos[k-1].f0==0)
					continue;
				v[aaa] = aaalsf(picos[k - 1].a, picos[k - 1].f0, aaa);
				P[aaa].setZero();
				for(size_t j = 1; j<=m; j++)
				{
					P[aaa](j - 1) = th[j - 1].a / std::sqrt(thd[j - 1]) * std::exp(-0.5 * ((v[aaa] - th[j - 1].u).transpose() * thI[j - 1] * (v[aaa] - th[j - 1].u)).value());
				}
				Psum[aaa] = P[aaa].sum();
				P[aaa] /= Psum[aaa];


				vt[aaa](v[aaa].size());
				vt[aaa].setZero();
				for(size_t j = 1; j<=m;j++)
				{
					vt[aaa] = vt[aaa] + P[aaa](j - 1) * (th[j - 1].v + th[j - 1].R * thI[j - 1] * (v[aaa] - th[j - 1].u));
				}
				aivt[aaa] = lsfadap(vt[aaa], aaa);
				eevt[aaa](p + 1, (int)std::ceil(fmax / f02s(k - 1)) - 1);
				eevt[aaa].setOnes();
				eevt[aaa](1, 0) = std::exp(-i1 * pi * f02s(k - 1) / fmax);
				for(Eigen::Index jj = 2; jj <= eevt[aaa].cols(); jj++)
				{
					eevt[aaa](1, jj - 1) = eevt[aaa](1, jj - 2);
				}
				for(Eigen::Index jj = 3; jj<=p+1;jj++)
				{
					eevt[aaa].row(jj - 1) = eevt[aaa].row(jj - 2).cwiseProduct(eevt[aaa].row(1));
				}
				Afvt[aaa] = (1 / (aivt[aaa] * eevt[aaa]).array()).matrix().eval();
				E1[aaa] = picos[k - 1].a.squaredNorm();
				Afvt[aaa] = Afvt[aaa] * std::sqrt(E1[aaa] / Afvt[aaa].dot(Afvt[aaa]).real());
				Evt[aaa] = Afvt[aaa].cwiseProduct(Afvt[aaa].conjugate()).real();
				aavt[aaa] = Evt[aaa].cwiseSqrt();
				PP[aaa].setZero();
				for(size_t j = 1; j<=mm; j++)
				{
					PP[aaa](j - 1) = th2[j - 1].a / std::sqrt(th2d[j - 1]) * std::exp(-0.5 * ((vt[aaa] - th2[j - 1].u).transpose() * th2I[j - 1] * (vt[aaa] - th2[j - 1].u)).value());
				}

				PP[aaa] = PP[aaa] / PP[aaa].sum();
				vtt[aaa](vt.size());
				vtt[aaa].setZero();
				for(size_t j = 1; j<=mm; j++)
				{
					vtt[aaa] = vtt[aaa] + PP[aaa](j - 1) * (th2[j - 1].v + th2[j - 1].R * th2I[j - 1] * (vt[aaa] - th2[j - 1].u));
				}

				vttaux[aaa](vtt[m].size() + 2);
				vttaux[aaa](0) = 0;
				vttaux[aaa](vttaux[aaa].size() - 1) = pi;
				vttaux[aaa].segment(1, vtt[aaa].size()) = vtt[aaa];
				for(Eigen::Index j = 2; j<=vttaux[aaa].size()-1;j++)
				{
					if (vttaux[aaa](j - 1) <= vttaux[aaa](j - 2) && vttaux[aaa](j - 1) < vttaux[aaa](j) && vttaux[aaa](j - 2) < vttaux[aaa](j))
					{
						vtt[aaa](j - 2) = 0.99 * vttaux[aaa](j - 2) + 0.01 * vttaux[aaa](j);
					}
				}
				aivtt[aaa] = lsfadap(vtt[aaa], aaa);
				aivtt[aaa] *= (picos[k - 1].e(0) / aivtt[aaa](0));
				for(size_t j = 1; j <= m; j++)
				{
					fy[aaa] += P[aaa](j - 1) * fw[j - 1].y;
				}
				ff1[aaa](picos[k - 1].a.size() + 2);
				ff1[aaa](0) = 0.0;
				ff1[aaa](ff1[aaa].size() - 1) = fmax;
				ff1[aaa].segment(1, picos[k - 1].a.size()) = seq(1, picos[k - 1].a.size()) * f01s(k - 1);
				ap1[aaa](picos[k - 1].a.size() + 2);
				ap1[aaa].segment(1, picos[k - 1].a.size()) = picos[k - 1].a.array() * (i1 * picos[k - 1].p).array().exp();
				ap1[aaa](0) = picos[k - 1].a(0);
				ap1[aaa](ap1[aaa].size() - 1) = 0;
				aa1[aaa](picos[k - 1].a.size() + 2);
				aa1[aaa](0) = std::log(picos[k - 1].a(0));
				aa1[aaa](aa1.size() - 1) = std::log(picos[k - 1].a.minCoeff());
				aa1[aaa].segment(1, picos[k - 1].a.size()) = picos[k - 1].a.array().log();

				aa2[aaa]((int)std::ceil(fmax / f02s(k - 1)) - 1);
				aa2[aaa].setZero();
				pp2[aaa] = aa2[aaa];

				for(Eigen::Index j = 1; j <= aa2[aaa].size(); j++)
				{
					f2j[aaa] = j * f02s(k - 1);
					for(jj[aaa] = jjant1[aaa]; jj[aaa]<=fy[aaa].size()-1;jj[aaa]++)
					{
						if(f2j[aaa]>=fy[aaa](jj[aaa]-1)&&f2j[aaa]<fy[aaa](jj[aaa]))
						{
							jjant1[aaa] = (int)jj[aaa];
							break;
						}
					}
					f1j[aaa] = fx(jj[aaa] - 1) + (fx(jj[aaa]) - fx(jj[aaa] - 1)) * (f2j[aaa] - fy[aaa](jj[aaa] - 1)) / (fy[aaa](jj[aaa]) - fy[aaa](jj[aaa] - 1));
					if(f1j[aaa]<fmax)
					{
						for(jj[aaa] = jjant2[aaa];jj[aaa]<ff1[aaa].size()-1;jj[aaa]++)
						{
							if (f1j[aaa] >= ff1[aaa](jj[aaa] - 1) && f1j[aaa] < ff1[aaa](jj[aaa]))
							{
								jjant2[aaa] = (int)jj[aaa];
								break;
							}
						}
						aa2[aaa](j - 1) = std::exp(aa1[aaa](jj[aaa] - 1) + (aa1[aaa](jj[aaa]) - aa1[aaa](jj[aaa] - 1)) * (f1j[aaa] - ff1[aaa](jj[aaa] - 1)) / (ff1[aaa](jj[aaa]) - ff1[aaa](jj[aaa] - 1)));
						pp2[aaa](j - 1) = angle(ap1[aaa](jj[aaa] - 1) + (ap1[aaa](jj[aaa]) - ap1[aaa](jj[aaa] - 1)) * (f1j[aaa] - ff1[aaa](jj[aaa] - 1)) / (ff1[aaa](jj[aaa]) - ff1[aaa](jj[aaa] - 1)));
					}
					else
					{
						if (scale[aaa] == 0)
						{
							scale[aaa] = aa2[aaa](j - 2) / std::abs(Afvt[aaa](j - 2));
						}
						aa2[aaa](j - 1) = scale[aaa] * std::abs(Afvt[aaa](j - 1));
						pp2[aaa](j - 1) = angle(Afvt[aaa](j - 1));
					}
				}
				nf[aaa] = (int)std::ceil(flimsm / f02s(k - 1) - 1);
				seqnf[aaa] = seq<Eigen::RowVectorXi>(-nf[aaa], nf[aaa]).eval();
				win[aaa] = 1 - (f02s(k - 1) / flimsm) * seqnf[aaa].cast<Eigen::TFloat>().array();
				win[aaa] /= win[aaa].sum();


			}
		}
	});
	
}

