#include "RayTracer.h"

#include "../CoreSystems/IApplication.h"

extern IApplication* g_pApplication;

RayTracer::RayTracer() : m_pPrimRayStage(nullptr), m_pIntersectionStage(nullptr), m_pColorStage(nullptr)
{ }

RayTracer::~RayTracer()
{
	SAFE_DELETE(m_pPrimRayStage);
	SAFE_DELETE(m_pIntersectionStage);
	SAFE_DELETE(m_pColorStage);
}

bool RayTracer::VInit(DXDriver* _driver)
{
	if (m_pPrimRayStage || m_pIntersectionStage || m_pColorStage)
	{
		PrintError(AT, "shader stages already created");
		return false;
	}

	m_pPrimRayStage = VNEW ComputeShader();

	if (!m_pPrimRayStage->Init(L"Assets/Shaders/RTPrimaryRays.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create compute shader stage");
		return false;
	}

	m_pIntersectionStage = VNEW ComputeShader();

	if (!m_pIntersectionStage->Init(L"Assets/Shaders/RTIntersect.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create compute shader stage");
		return false;
	}

	m_pColorStage = VNEW ComputeShader();

	//if (!m_pColorStage->Init(L"Assets/Shaders/RCSinglePass.hlsl", _driver->GetDevice()))
	if (!m_pColorStage->Init(L"Assets/Shaders/RTComputeColor.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create compute shader stage");
		return false;
	}
	Resolution res = g_pApplication->VGetResolution();

	/* UAVS */
	m_pRays = VNEW D3DBuffer();
	if (!m_pRays->Init(_driver->GetDevice(), BT_STRUCTURED, BB_UAV, res.height * res.width, sizeof(Ray)))
		return false;
	m_pIntersections = VNEW D3DBuffer();
	if (!m_pIntersections->Init(_driver->GetDevice(), BT_STRUCTURED, BB_UAV, res.height *res.width, sizeof(Intersection)))
		return false;
	m_pColorAccumulation = VNEW D3DBuffer();
	if (!m_pColorAccumulation->Init(_driver->GetDevice(), BT_STRUCTURED, BB_UAV, res.height * res.width, sizeof(FVEC4)))
		return false;

	return true;
}

void RayTracer::VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices)
{
	
	//ID3D11UnorderedAccessView* uavs[] = { _driver->GetRenderTargetUAV(), m_pRays->GetUAV(), m_pIntersections->GetUAV(), m_pColorAccumulation->GetUAV() };

	//_driver->GetContext()->CSSetUnorderedAccessViews(0, 4, uavs, NULL);

	//ID3D11ShaderResourceView* srvs[] = { _indices->GetSRV() };
	//_driver->GetContext()->CSSetShaderResources(2, 1, srvs);


	//m_pPrimRayStage->VStage(_driver->GetContext());
	//_driver->GetContext()->Dispatch(32, 32, 1);
	//_driver->GetContext()->CSSetShader(nullptr, nullptr, 0);

	//m_pIntersectionStage->VStage(_driver->GetContext());
	//_driver->GetContext()->Dispatch(32, 32, 1);
	//_driver->GetContext()->CSSetShader(nullptr, nullptr, 0);

	//m_pColorStage->VStage(_driver->GetContext());
	//_driver->GetContext()->Dispatch(32, 32, 1);
	//_driver->GetContext()->CSSetShader(nullptr, nullptr, 0);


	//ID3D11UnorderedAccessView* uavsU[] { nullptr, nullptr, nullptr, nullptr };
	//ID3D11ShaderResourceView* srvsU[] = { nullptr };
	//_driver->GetContext()->CSSetUnorderedAccessViews(0, 4, uavsU, NULL);
	//_driver->GetContext()->CSSetShaderResources(2, 1, srvsU);
}