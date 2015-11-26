#pragma once

//#include "../CoreSystems/CoreStd.h"
//#include "../CoreSystems/DXDriver.h"
//
//#include "Shader.h"
//#include "Render_Create.h"
class DXDriver;
class D3DBuffer;

class IRenderer
{
private:
protected:
public:
	virtual ~IRenderer() {}

	virtual bool VInit(DXDriver* _driver) = 0;
	virtual void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	= 0;
};

