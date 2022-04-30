#include"HSMwfwConvertClass.h"

#include "angle.h"
#include "concat.h"
#include "constants.h"
#include "poly.h"
#include"ppl.h"
#include "roots.h"
#include "seq.h"
#include "sliceByIndices.h"


HSMwfwConvert::HSMwfwConvert(HSMModel model, PicosStructArray& picos)
	:model(model),picos(picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
	//i1 = (0.0, 1.0);
	f01s.resize(picos.size());
	f01s.setZero();
	f02s.resize(picos.size());
	f02s = f01s;
	thd.resize(th.size()); thI.resize(th.size());
	std::transform(th.cbegin(), th.cend(), thd.begin(), [](const ThxyElementType& e) {return e.E.determinant(); });
	std::transform(th.cbegin(), th.cend(), thI.begin(), [](const ThxyElementType& e) {return e.E.inverse(); });

	th2d.resize(th2.size()); th2I.resize(th2.size());
	std::transform(th2.cbegin(), th2.cend(), th2d.begin(), [](const ThxyElementType& e) {return e.E.determinant(); });
	std::transform(th2.cbegin(), th2.cend(), th2I.begin(), [](const ThxyElementType& e) {return e.E.inverse(); });

	thd.resize(threadNum);
	thI.resize(threadNum);

	th2d.resize(threadNum);
	th2I.resize(threadNum);

	P.resize(threadNum);
	PP.resize(threadNum);
	P2.resize(threadNum);

	for(auto& i:P)
	{
		i.resize(m);
	}
	for(auto& i:PP)
	{
		i.resize(mm);
	}
	for(int i = 0; i < threadNum;i++)
	{
		P2[i] = P[i];
 	}

	lsfNk.resize(threadNum);
	lsfff.resize(threadNum);
	lsfPP.resize(threadNum);
	lsfR.resize(threadNum);
	for (auto& i : lsfR)
	{
		// i.setZero(p + 1).eval();
		i = Eigen::TRowVectorX::Zero((int)(p + 1)).eval();
	}
	lsfai.resize(threadNum);
	for (auto& i : lsfai)
	{
		i.resize(p);
		//i.setZero(p+2);
	}
	lsfe.resize(threadNum);
	lsfk.resize(threadNum);

	lsfaz1.resize(threadNum);
	for (auto& i : lsfaz1)
	{
		i.resize(p + 2);
	}
	lsfaz2.resize(threadNum);
	lsflsf.resize(threadNum);
	lsfSize.resize(threadNum);
	
	
	

	v.resize(threadNum);
	Psum.resize(threadNum);
	vt.resize(threadNum);

	adaps1 = seq<Eigen::RowVectorXi>(2, 2, (int)p);
	adaps2 = seq<Eigen::RowVectorXi>(1, 2, (int)p);

	adapPart1.resize(threadNum);
	adapPart2.resize(threadNum);
	for(auto& i: adapPart1)
		i.resize(adaps1.size());
	for(auto& i: adapPart2)
		i.resize(adaps2.size());

	temp1.resize(threadNum);
	for(auto& i: temp1)
	{
		i.resize(adapPart1[0].size() * 2 + 1);
		i(0) = 1;
	}
	temp2.resize(threadNum);
	for(auto& i:temp2)
	{
		i.resize(adapPart2[0].size() * 2 + 1);
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
	for(auto& i:fy)
	{
		i.resize(fw[0].y.size());
		i.setZero();
	}

	ff1.resize(threadNum);
	ap1.resize(threadNum);
	aa1.resize(threadNum);

	aa2.resize(threadNum);
	pp2.resize(threadNum);

	jjant1.resize(threadNum);
	jjant2.resize(threadNum);
	for (auto& i : jjant1) i = 1;
	for (auto& i : jjant2) i = 1;
	scale.resize(threadNum);
	for (auto i : scale) i = 0.0;

	f2j.resize(threadNum);
	f1j.resize(threadNum);
	jj.resize(threadNum);
	for (auto& i : jj) i = 0;

	nf.resize(threadNum);
	seqnf.resize(threadNum);
	win.resize(threadNum);

	g.resize(threadNum);
	gsm.resize(threadNum);
	ind.resize(threadNum);
}

void HSMwfwConvert::updateSize(PicosStructArray& picos)
{
	timesPerThread = static_cast<int>(static_cast<float>(picos.size()) / (float)threadNum + 1);
	f01s.resize(picos.size());
	f01s.setZero();
	f02s = f01s;
}

Eigen::TVectorX HSMwfwConvert::aaalsf(Eigen::Ref<const Eigen::TRowVectorX> aa, Eigen::TFloat f0, size_t r)
{
	// const int fs = 10000;
	const int p = (int)this->p;
	lsfNk[r] = (int)aa.size();
	lsfff[r] = (seq(1, lsfNk[r]) * f0).eval();
	lsfPP[r] = aa.array().square().eval();
	//auto R = Eigen::TRowVectorX::Zero(p + 1).eval();
	for(int j = 1; j <= p+1; j++)
	{
		lsfR[r](j - 1) = (1.0 / lsfNk[r]) * (lsfPP[r] * (2 * pi * (j - 1) / lsfFs * lsfff[r]).array().cos()).sum();
	}

	//lsfR[r] = R;
	lsfe[r] = lsfR[r](0);
	for(int j = 1; j<=p; j++)
	{
		if(j==1)
		{
			lsfk[r] = lsfR[r](1) / lsfR[r](0);
			lsfai[r](0) = lsfk[r];
			lsfe[r] = (1 - lsfk[r] * lsfk[r]) * lsfe[r];
		}
		else
		{
			lsfk[r] = lsfR[r](j);
			lsfk[r] = lsfk[r] - lsfai[r].head(j - 1).dot(lsfR[r].segment(1, j - 1).reverse());
			lsfk[r] = lsfk[r] / lsfe[r];
			lsfai[r](j - 1) = lsfk[r];
			Eigen::TRowVectorX aux = lsfai[r].head(j - 1) - lsfk[r] * lsfai[r].head(j - 1).reverse();
			lsfai[r].head(j - 1) = aux;// = lsfai[r].head(j - 1) - lsfk[r] * lsfai[r].head(j - 1).reverse();
			lsfe[r] = (1 - lsfk[r] * lsfk[r]) * lsfe[r];
		}
	}
	lsfaz1[r](0) = 1;
	lsfaz1[r].segment(1, p) = -lsfai[r];
	lsfaz1[r](p + 1) = 0;

	lsfaz2[r] = lsfaz1[r].reverse();
	lsflsf[r] = angle(concat<Eigen::TRowVectorXc>(roots(lsfaz1[r] + lsfaz2[r]).transpose(), roots(lsfaz1[r] - lsfaz2[r]).transpose())).eval();
	lsfSize[r] = (int)lsflsf[r].size();
	std::sort(lsflsf[r].data(), lsflsf[r].data() + lsfSize[r]);
	return lsflsf[r].segment(lsfSize[r] - p - 1, p).transpose();
}

Eigen::TRowVectorX HSMwfwConvert::lsfadap(Eigen::Ref<const Eigen::TVectorX> lsf, size_t r)
{
	int p = (int)lsf.size();
	for(Eigen::Index j = 0; j < adaps1.size(); j++)
	{
		adapPart1[r](j) = std::exp(i1 * lsf(adaps1(j) - 1));
	}
	for(Eigen::Index j = 0; j < adaps2.size(); j++)
	{
		adapPart2[r](j) = std::exp(i1 * lsf(adaps2(j) - 1));
	}
	temp1[r].segment(1, adapPart1[r].size()) = adapPart1[r];
	temp1[r].tail(adapPart1[r].size()) = adapPart1[r];
	temp2[r].head(adapPart2[r].size()) = adapPart2[r];
	temp2[r](adapPart2[r].size()) = -1;
	temp2[r].tail(adapPart1[r].size()) = adapPart2[r].conjugate();
	adapai[r] = (0.5 * (poly(temp1[r]) + poly(temp2[r]))).real().eval();
	return adapai[r].head(adapai.size() - 1);
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
	//concurrency::parallel_for(size_t(0), size_t(threadNum), [&](size_t r)
	for (size_t r = (0); r < threadNum; r++)
	{
		for(int k = r*timesPerThread+1; k<(r+1)*timesPerThread+1;k++)
		{
			if(k<=picos.size())
			{
				if(picos[k-1].f0==0)
					continue;
				v[r] = aaalsf(picos[k - 1].a, picos[k - 1].f0, r);
				P[r].setZero();
				for(size_t j = 1; j<=m; j++)
				{
					P[r](j - 1) = th[j - 1].a / std::sqrt(thd[j - 1]) * std::exp(-0.5 * ((v[r] - th[j - 1].u).transpose() * thI[j - 1] * (v[r] - th[j - 1].u)).value());
				}
				Psum[r] = P[r].sum();
				P[r] /= Psum[r];


				vt[r](v[r].size());
				vt[r].setZero();
				for(size_t j = 1; j<=m;j++)
				{
					vt[r] = vt[r] + P[r](j - 1) * (th[j - 1].v + th[j - 1].R * thI[j - 1] * (v[r] - th[j - 1].u));
				}
				aivt[r] = lsfadap(vt[r], r);
				eevt[r](p + 1, (int)std::ceil(fmax / f02s(k - 1)) - 1);
				eevt[r].setOnes();
				eevt[r](1, 0) = std::exp(-i1 * pi * f02s(k - 1) / fmax);
				for(Eigen::Index jj = 2; jj <= eevt[r].cols(); jj++)
				{
					eevt[r](1, jj - 1) = eevt[r](1, jj - 2);
				}
				for(Eigen::Index jj = 3; jj<=p+1;jj++)
				{
					eevt[r].row(jj - 1) = eevt[r].row(jj - 2).cwiseProduct(eevt[r].row(1));
				}
				Afvt[r] = (1 / (aivt[r] * eevt[r]).array()).matrix().eval();
				E1[r] = picos[k - 1].a.squaredNorm();
				Afvt[r] = Afvt[r] * std::sqrt(E1[r] / Afvt[r].dot(Afvt[r]).real());
				Evt[r] = Afvt[r].cwiseProduct(Afvt[r].conjugate()).real();
				aavt[r] = Evt[r].cwiseSqrt();
				PP[r].setZero();
				for(size_t j = 1; j<=mm; j++)
				{
					PP[r](j - 1) = th2[j - 1].a / std::sqrt(th2d[j - 1]) * std::exp(-0.5 * ((vt[r] - th2[j - 1].u).transpose() * th2I[j - 1] * (vt[r] - th2[j - 1].u)).value());
				}

				PP[r] = PP[r] / PP[r].sum();
				vtt[r].resize(vt.size());
				vtt[r].setZero();
				for(size_t j = 1; j<=mm; j++)
				{
					vtt[r] = vtt[r] + PP[r](j - 1) * (th2[j - 1].v + th2[j - 1].R * th2I[j - 1] * (vt[r] - th2[j - 1].u));
				}

				vttaux[r].resize(vtt[m].size() + 2);
				vttaux[r](0) = 0;
				vttaux[r](vttaux[r].size() - 1) = pi;
				vttaux[r].segment(1, vtt[r].size()) = vtt[r];
				for(Eigen::Index j = 2; j<=vttaux[r].size()-1;j++)
				{
					if (vttaux[r](j - 1) <= vttaux[r](j - 2) && vttaux[r](j - 1) < vttaux[r](j) && vttaux[r](j - 2) < vttaux[r](j))
					{
						vtt[r](j - 2) = 0.99 * vttaux[r](j - 2) + 0.01 * vttaux[r](j);
					}
				}
				aivtt[r] = lsfadap(vtt[r], r);
				aivtt[r] *= (picos[k - 1].e(0) / aivtt[r](0));
				for(size_t j = 1; j <= m; j++)
				{
					fy[r] += P[r](j - 1) * fw[j - 1].y;
				}
				ff1[r].resize(picos[k - 1].a.size() + 2);
				ff1[r](0) = 0.0;
				ff1[r](ff1[r].size() - 1) = fmax;
				ff1[r].segment(1, picos[k - 1].a.size()) = seq(1, picos[k - 1].a.size()) * f01s(k - 1);
				ap1[r].resize(picos[k - 1].a.size() + 2);
				ap1[r].segment(1, picos[k - 1].a.size()) = picos[k - 1].a.array() * (i1 * picos[k - 1].p).array().exp();
				ap1[r](0) = picos[k - 1].a(0);
				ap1[r](ap1[r].size() - 1) = 0;
				aa1[r].resize(picos[k - 1].a.size() + 2);
				aa1[r](0) = std::log(picos[k - 1].a(0));
				aa1[r](aa1.size() - 1) = std::log(picos[k - 1].a.minCoeff());
				aa1[r].segment(1, picos[k - 1].a.size()) = picos[k - 1].a.array().log();

				aa2[r].resize((int)std::ceil(fmax / f02s(k - 1)) - 1);
				aa2[r].setZero();
				pp2[r] = aa2[r];

				for(Eigen::Index j = 1; j <= aa2[r].size(); j++)
				{
					f2j[r] = j * f02s(k - 1);
					for(jj[r] = jjant1[r]; jj[r]<=fy[r].size()-1;jj[r]++)
					{
						if(f2j[r]>=fy[r](jj[r]-1)&&f2j[r]<fy[r](jj[r]))
						{
							jjant1[r] = (int)jj[r];
							break;
						}
					}
					f1j[r] = fx(jj[r] - 1) + (fx(jj[r]) - fx(jj[r] - 1)) * (f2j[r] - fy[r](jj[r] - 1)) / (fy[r](jj[r]) - fy[r](jj[r] - 1));
					if(f1j[r]<fmax)
					{
						for(jj[r] = jjant2[r];jj[r]<ff1[r].size()-1;jj[r]++)
						{
							if (f1j[r] >= ff1[r](jj[r] - 1) && f1j[r] < ff1[r](jj[r]))
							{
								jjant2[r] = (int)jj[r];
								break;
							}
						}
						aa2[r](j - 1) = std::exp(aa1[r](jj[r] - 1) + (aa1[r](jj[r]) - aa1[r](jj[r] - 1)) * (f1j[r] - ff1[r](jj[r] - 1)) / (ff1[r](jj[r]) - ff1[r](jj[r] - 1)));
						pp2[r](j - 1) = angle(ap1[r](jj[r] - 1) + (ap1[r](jj[r]) - ap1[r](jj[r] - 1)) * (f1j[r] - ff1[r](jj[r] - 1)) / (ff1[r](jj[r]) - ff1[r](jj[r] - 1)));
					}
					else
					{
						if (scale[r] == 0)
						{
							scale[r] = aa2[r](j - 2) / std::abs(Afvt[r](j - 2));
						}
						aa2[r](j - 1) = scale[r] * std::abs(Afvt[r](j - 1));
						pp2[r](j - 1) = angle(Afvt[r](j - 1));
					}
				}
				nf[r] = (int)std::ceil(flimsm / f02s(k - 1) - 1);
				seqnf[r] = seq<Eigen::RowVectorXi>(-nf[r], nf[r]).eval();
				win[r] = 1 - (f02s(k - 1) / flimsm) * seqnf[r].cast<Eigen::TFloat>().array();
				win[r] /= win[r].sum();

				g[r] = (aavt[r].array() / aa2[r].array()).log().matrix().eval();
				gsm[r] = g[r];
				ind[r].resize(seqnf[r].size());
				for(Eigen::Index j = 1; j <= g[r].size(); j++)
				{
					ind[r] = seqnf[r].array() + j;
					if(j<nf[r])
					{
						for(Eigen::Index jj = 1; jj < ind[r].size(); ++jj)
						{
							if(ind[r](jj-1)<0)
							{
								ind[r](jj - 1) = -ind[r](jj - 1);
							}
							else if(ind[r](jj-1)==0)
							{
								ind[r](jj - 1) = 1;
							}
							else
							{
								break;
							}
						}
					}
					else if(j>g[r].size()-nf[r])
					{
						for(auto jj = ind[r].size();jj>=1;jj--)
						{
							if(ind[r](jj-1)>g[r].size()+1)
							{
								ind[r](jj - 1) = 2 * (int)g[r].size() + 2 - ind[r](jj - 1);
							}
							else if(ind[r](jj-1)==g[r].size()+1)
							{
								ind[r](jj - 1) = (int)g[r].size();
							}
							else
							{
								break;
							}
						}
					}
					gsm[r](j - 1) = sliceByIndices(g[r], ind[r], IndexBase::One).dot(win[r]);
				}
				g[r] = gsm[r].array().exp();
				aa2[r] = aa2[r].array() * g[r].array();
				aa2[r] *= std::sqrt(E1[r] / aa2[r].squaredNorm());

				picos[k - 1].f0 = f02s(k - 1);
				picos[k - 1].a = aa2[r];
				picos[k - 1].p = pp2[r];
				picos[k - 1].e = aivtt[r];
			}
		}
	}//);
}

