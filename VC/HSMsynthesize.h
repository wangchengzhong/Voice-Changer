#pragma once
#include"StructDefinitions.h"
class HSMsynthesize
{
public:
	HSMsynthesize();
	~HSMsynthesize();
	void processSynthesize(PicosStructArray& picos, int bufferLength);

};