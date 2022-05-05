#pragma once

#include<cmath>
// 使用线性插值将信号重新采样到新的大小
// 'originalSize' 是原始信号的最大尺寸
// 'newSignalSize' 是要重新采样到的大小。 'newSignal' 必须至少一样大。
static void linearResample(const float* const originalSignal, const int originalSize,
	float* const newSignal, const int newSignalSize)
{
	const auto lerp = [&](double v0, double v1, double t)
	{
		return (1.f - t) * v0 + t * v1;
	};

	// 如果原始信号大于新大小，则压缩信号以适应新缓冲区
	// 否则扩展信号以适应新缓冲区
	const auto scale = originalSize / (double)newSignalSize;
	double index = 0.f;

	for (int i = 0; i < newSignalSize; ++i)
	{
		const auto wholeIndex = (int)floor(index);
		const auto fractionIndex = index - wholeIndex;
		const auto sampleA = originalSignal[wholeIndex];
		const auto sampleB = originalSignal[wholeIndex + 1];
		newSignal[i] = lerp(sampleA, sampleB, fractionIndex);
		index += scale;
	}
}