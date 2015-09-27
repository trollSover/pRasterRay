#include "RayCaster.h"

#include "../CoreSystems/IApplication.h"

extern IApplication* g_pApplication;

RayCaster::RayCaster() : m_pSampler(nullptr), m_pSinglePassVoxelCS(nullptr), m_pSourceBuffer(nullptr)
{ }

RayCaster::~RayCaster()
{
	SAFE_DELETE(m_pSinglePassVoxelCS);
	SAFE_RELEASE(m_pSampler);
}

bool RayCaster::VInit(DXDriver* _driver)
{
	RenderTarget* target = nullptr;
	_driver->GetRenderTarget(RT_DEPTH, target);

	if (!target)
		return false;

	target->GetUAV(m_pSourceBuffer);

	_driver->GetRenderTarget(RT_COLOR, target);

	if (!target)
		return false;

	target->GetUAV(m_pTargetBuffer);

	if (!m_pSourceBuffer || !m_pTargetBuffer)
		return false;

	if (m_pSinglePassVoxelCS)
	{
		PrintError(AT, "shader stage already created");
		return false;
	}
	m_pSinglePassVoxelCS = VNEW ComputeShader();
	if (!m_pSinglePassVoxelCS)
	{
		PrintError(AT, "failed to allocate memory for shader");
		return false;
	}
	if (!m_pSinglePassVoxelCS->Init(L"Assets/RCSinglePassVoxel.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create compute shader stage");
		return false;
	}



	return true;
}

void RayCaster::VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices)
{
	ID3D11UnorderedAccessView* uav[2] = { m_pSourceBuffer, m_pTargetBuffer };

	_driver->GetContext()->CSSetUnorderedAccessViews(4, 2, uav, NULL);

	m_pSinglePassVoxelCS->VStage(_driver->GetContext());
	_driver->GetContext()->Dispatch(40, 45, 1);

	_driver->GetContext()->CSSetShader(nullptr, nullptr, 0);

	uav[0] = nullptr;
	uav[1] = nullptr;

	_driver->GetContext()->CSSetUnorderedAccessViews(4, 2, uav, NULL);
}