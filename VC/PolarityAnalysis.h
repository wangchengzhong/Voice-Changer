#pragma once

#include "StructDefinitions.h"

class PolarityAnalysis
{
public:
	PolarityAnalysis(PicosStructArray& picos);
	~PolarityAnalysis();

	void processPolarity(PicosStructArray picos);
private:
	PicosStructArray& picos;
};
