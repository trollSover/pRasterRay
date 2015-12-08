#pragma once

#include "Application\SVO_Loader.h"
#include "CoreSystems\CoreStd.h"

struct Int2
{
	int x, y;
};

class CPURaycaster
{
private:
	SVO_Loader* m_pSvoLoader;
	Int2 m_numThreads;
	Int2 m_resolution;

	CBDCamera camera;
protected:
public:

private:
	void EmulateThread(const Int2);
protected:
public:

	void Init(const Int2, const Int2, SVO_Loader*);
	
	void Emulate(const CBDCamera&);
};