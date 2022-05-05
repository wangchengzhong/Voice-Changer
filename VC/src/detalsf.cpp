#include "detalsf.h"
#include "seq.h"
#include "constants.h"
#include "angle.h"
#include "concat.h"
#include "roots.h"
#include <algorithm>


/**
* Implementation of detalsf.m
*/
Eigen::TMatrixX detalsf(const PicosStructArray & picos, int p)
{
	constexpr int fmax = 5000;
	constexpr int fs = 2 * fmax;

	Eigen::TMatrixX LSF(p, picos.size());
	LSF.setZero();

	// 在 for 循环外定义已知大小的向量以提高效率
	for (int k = 1; k <= picos.size(); k++)
	{
		Eigen::TRowVectorX R(p + 1);
		Eigen::TRowVectorX ai(p);
		Eigen::TRowVectorX az1(p + 2);
		auto Nk = picos[k - 1].a.size();
		if (Nk == 0)
			continue;
		auto PP = picos[k - 1].a.array().square().eval();
		auto ff = (seq(1, (int)Nk) * picos[k - 1].f0).eval();
		//R 的每个元素将在此分配
		for (int j = 1; j <= p + 1; j++)
		{
			R(j - 1) = (1.0 / Nk) * (PP.array() * ((2 * pi*(j - 1) / fs)*ff.array()).cos()).sum();
		}

		ai.setZero();
		auto e = R(0);
		for (int j = 1; j <= p; j++)
		{
			if (j == 1)
			{
				auto K = R(1) / R(0);
				ai(0) = K;
				e = (1 - K*K) *e;
			}
			else
			{
				auto K = R(j);
				K = K - ai.head(j - 1).dot(R.segment(1, j - 1).reverse()); // ai: 1->j-1, R:j->2
				K = K / e;
				ai(j - 1) = K;
				
				ai.head(j - 1) = (ai.head(j - 1) - K * ai.head(j - 1).reverse()).eval(); // use eval() to avoid aliasing
				e = (1 - K * K) * e;
			}
		}

		// ai 变成 [1 -ai],  az1 = [1, -ai, 0]
		az1 << 1, -ai, 0;
		auto az2 = az1.reverse();
		auto lsf = angle(concat<Eigen::TVectorXc>(roots(az1 + az2), roots(az1 - az2), Eigen::Vertical)).eval();
		// lsf升序排序
		std::sort(lsf.data(), lsf.data() + lsf.size());
		LSF.col(k - 1) = lsf.segment(lsf.size() - p - 1, p);
	}
	return LSF;
}
