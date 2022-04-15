#pragma once
#include "StructDefinitions.h"
class HSMwfwConvert
{
public:
	HSMwfwConvert(HSMModel& model);
	~HSMwfwConvert();

	void processWfwConvert(PicosStructArray& picos);
private:
	HSMModel& model;
};