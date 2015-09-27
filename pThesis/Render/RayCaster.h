#pragma once

#include "Renderer.h"

class RayCaster
	: public IRenderer
{
private:
	Shader* m_pSinglePassVoxelCS;

	ID3D11SamplerState* m_pSampler;
	
	ID3D11UnorderedAccessView*	m_pSourceBuffer;
	ID3D11UnorderedAccessView*	m_pTargetBuffer;
protected:
public:

private:
	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	final;

	

protected:
public:
	RayCaster();
	~RayCaster();
};