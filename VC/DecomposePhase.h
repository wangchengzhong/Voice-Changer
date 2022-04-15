#pragma once
#include"StructDefinitions.h"
class DecomposePhase
{
public:
	DecomposePhase(PicosStructArray& picos);
	~DecomposePhase();

	void processDecompose(PicosStructArray& picos);
private:
	PicosStructArray& picos;

};