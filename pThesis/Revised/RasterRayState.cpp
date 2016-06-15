#include "RasterRayApp.h"

using namespace RasterRay;

static bool toggleUI				= true;
static bool toggleRasterize			= true;
static bool toggleRayCast			= true;
static bool toggleBoundingVolume	= true;
static bool toggleFill				= true;
static bool toggleCull				= true;
static bool toggleMove				= true;
static bool toggleCameraReset		= true;
static bool toggleDebug				= true;
static bool toogleRasterLoDIncr		= true;
static bool toogleRasterLoDDecr		= true;

unsigned RasterRayApp::GetRasterStateId(const D3D11_FILL_MODE _fillMode, const D3D11_CULL_MODE _cullMode)
{
	unsigned index;

	switch (_cullMode)
	{
	case D3D11_CULL_BACK:
		index = 1;
		break;
	case D3D11_CULL_FRONT:
		index = 2;
		break;
	case D3D11_CULL_NONE:
	default:
		index = 0;
		break;
	}
	
	if (_fillMode == D3D11_FILL_WIREFRAME)
		index = index + 1;

	return index;
}

unsigned RasterRayApp::SetFillMode(const D3D11_FILL_MODE& _mode)
{
	m_currentRasterStateIndex = (m_currentRasterStateIndex + 2) % RSI_COUNT;
	m_pContext->RSSetState(m_pRasterizerStates[m_currentRasterStateIndex].m_pState);
	return m_currentRasterStateIndex;
}

unsigned RasterRayApp::SetCullMode(const D3D11_CULL_MODE& _mode)
{
	m_currentRasterStateIndex += (m_currentRasterStateIndex % 2 == 0) ? 1 : -1;
	//if (m_currentRasterStateIndex % 2 != 0)
	//	m_currentRasterStateIndex -= 1;
	//else
	//	m_currentRasterStateIndex += 1;

	m_pContext->RSSetState(m_pRasterizerStates[m_currentRasterStateIndex].m_pState);
	return m_currentRasterStateIndex;
}

void RasterRayApp::ResetRasterState(void)
{
	m_pContext->RSSetState(m_pRasterizerStates[m_currentRasterStateIndex].m_pState);
}

unsigned RasterRayApp::SetDepthMode(const unsigned _index)
{
	unsigned index = _index % DSSI_COUNT;
	m_pContext->OMSetDepthStencilState(m_pDepthStencilStates[index], 1);
	return index;
}

void RasterRayApp::ResetDepthMode(void)
{
	m_pContext->OMSetDepthStencilState(m_pDepthStencilStates[m_currentDepthStateIndex], 1);
	//m_currentDepthStateIndex = (_index % DSSI_COUNT);
}

bool RasterRayApp::IsKeyPress(unsigned _key)
{
	return m_pInput->VIsKeyDown(_key);
}

bool RasterRayApp::ToggleOnKey(unsigned _key, bool& _toggle)
{
	// key pressed and we can toggle
	if (IsKeyPress(_key) && _toggle)
	{
		_toggle = false;
		return true;
	}
	// key not pressed - reset toggle
	if (!IsKeyPress(_key))
		_toggle = true;
	// toggle is false -either no OR continous keypress - do not toggle
	return false;
}

void RasterRayApp::UpdateRenderState(void)
{
	static int cullIndex;

	// Rasterizer State Updates
	if (ToggleOnKey(TOGGLE_CULL_MODE, toggleCull))
	{
		static const D3D11_CULL_MODE modes[3] = { D3D11_CULL_BACK, D3D11_CULL_FRONT, D3D11_CULL_NONE };
		cullIndex = (cullIndex + 1);
		if (cullIndex > 2) cullIndex = 0;
		m_cullMode = modes[cullIndex];
		SetCullMode(m_cullMode);
	}

	if (ToggleOnKey(TOGGLE_FILL_MODE, toggleFill))
	{
		m_fillMode == D3D11_FILL_SOLID ? m_fillMode = D3D11_FILL_WIREFRAME : m_fillMode = D3D11_FILL_SOLID;
		SetFillMode(m_fillMode);
	}

	// Render State Update
	if (ToggleOnKey(TOGGLE_RASTERIZE, toggleRasterize))
	{
		m_drawRasterized = !m_drawRasterized;
	}

	if (ToggleOnKey(TOGGLE_RAYCAST, toggleRayCast))
	{
		m_drawRayCast = !m_drawRayCast;
	}

	if (ToggleOnKey(TOGGLE_DEBUG, toggleDebug))
	{
		m_drawDebug = !m_drawDebug;
	}

	if (ToggleOnKey(RASTERIZE_LOD_INCREMENT, toogleRasterLoDIncr))
	{
		m_lodLevel = (m_lodLevel + 1) % m_sparseOctree.GetHeader().numLevels;
	}
	else if (ToggleOnKey(RASTERIZE_LOD_DECREMENT, toogleRasterLoDDecr))
	{
		m_lodLevel = (m_lodLevel - 1) % m_sparseOctree.GetHeader().numLevels;
	}

	//if (ToggleOnKey(TOGGLE_BOUNDINGVOLUME, toggleBoundingVolume))
	//{
	//	m_drawBoundingVolume = !m_drawBoundingVolume;
	//}
}

static bool toggleReload = true;
HRESULT RasterRayApp::ReloadShader(const std::string _shaderPath)
{
	HRESULT hr = S_OK;

	if (ToggleOnKey(RELOAD_SHADER, toggleReload))
	{
		if (m_pRayCastComputeShader)
		{
			m_pRayCastComputeShader->Release();
			m_pRayCastComputeShader = nullptr;
		}
		m_drawRayCast = false;

		/* create compute shader */
		D3D_SHADER_MACRO defines[] =
		{
			{ "STACK_LIMIT", "15" },
			{ "ITR_LIMIT", "1000" },
			{ "THREAD_COUNT_X", "32" },
			{ "THREAD_COUNT_Y", "16" },
			{ "WORK_SIZE_X", "5" },
			{ "WORK_SIZE_Y", "3" },
			NULL, NULL
		};
		ID3D10Blob* pShaderByteCode = nullptr;
		hr = CompileShader(L"Revised/Shaders/RCSinglePassRaycast.hlsl", "cs_5_0", pShaderByteCode, "MainCS", &defines[0]);
		if (hr != S_OK) { PrintError(AT, hr); return hr; }

		hr = m_pDevice->CreateComputeShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pRayCastComputeShader);
		if (FAILED(hr)) { PrintError(AT, hr); return hr; }

		m_drawRayCast = true;
		static int reloadCounter;
		reloadCounter++;
		printf("%i: Shader Reloaded Successfully\n", reloadCounter);
	}

	

	return hr;
}


static float movespeed = 1.f;

void RasterRayApp::UpdateCamera(const Time& _time)
{
	const float lookBase = 1.f / 1000.f;
	const float lookSpeed = lookBase * _time.dtMS;

	// Camera Reset
	if (ToggleOnKey(CAMERA_RESET_ALL, toggleCameraReset))
	{
		m_pCamera->SetMovementSpeed(1);
		m_pCamera->SetPositionAndView(0, 0, 0, 0, 0);
	}
	
	if (m_pInput->VIsMouseKeyDown())
	{
		if (m_pInput->MouseMoved())
		{
			m_pCamera->AdjustHeadingPitch(m_pInput->m_mousePoint.x * lookSpeed, m_pInput->m_mousePoint.y * lookSpeed);
		}
	}

	// yaw roll pitch
	if (IsKeyPress(CAMERA_YAW_LEFT))	m_pCamera->AdjustYaw(1 * lookSpeed);
	if (IsKeyPress(CAMERA_YAW_RIGHT))	m_pCamera->AdjustYaw(-1 * lookSpeed);
	if (IsKeyPress(CAMERA_ROLL_LEFT))	m_pCamera->AdjustRoll(1 * lookSpeed);
	if (IsKeyPress(CAMERA_ROLL_RIGHT))	m_pCamera->AdjustRoll(-1 * lookSpeed);
	if (IsKeyPress(CAMERA_PITCH_LEFT))	m_pCamera->AdjustPitch(1 * lookSpeed);
	if (IsKeyPress(CAMERA_PITCH_RIGHT)) m_pCamera->AdjustPitch(-1 * lookSpeed);

	// WASD movement
	IsKeyPress(CAMERA_FORWARD) ? m_pCamera->SetMovementToggle(0, 1) : m_pCamera->SetMovementToggle(0, 0);
	IsKeyPress(CAMERA_LEFT) ? m_pCamera->SetMovementToggle(2, -1) : m_pCamera->SetMovementToggle(2, 0);
	IsKeyPress(CAMERA_BACK) ? m_pCamera->SetMovementToggle(1, -1) : m_pCamera->SetMovementToggle(1, 0);

	IsKeyPress(CAMERA_RIGHT) ? m_pCamera->SetMovementToggle(3, 1) : m_pCamera->SetMovementToggle(3, 0);
	// Set Camera Move Speed
	if (ToggleOnKey(CAMERA_SPEED_INCREMENT, toggleMove))
	{
		if (m_pCamera->GetMovementSpeed() >= CAMERA_SPEED_MAX)
		{
			m_pCamera->SetMovementSpeed(CAMERA_SPEED_MAX);
		}
		else
		{
			float incr = movespeed * 0.01;
			m_pCamera->SetMovementSpeed(movespeed += incr);
		}

	}
	if (ToggleOnKey(CAMERA_SPEED_DECREMENT, toggleMove))
	{
		if (m_pCamera->GetMovementSpeed() <= CAMERA_SPEED_MIN)
		{
			m_pCamera->SetMovementSpeed(CAMERA_SPEED_MIN);
		}
		else
		{
			float decr = -movespeed * 0.01;
			m_pCamera->SetMovementSpeed(movespeed += decr);
		}
	}
	// Reset Camera Move Speed
	if (ToggleOnKey(CAMERA_SPEED_RESET, toggleMove))
	{
		m_pCamera->SetMovementSpeed(1);
	}

	if (IsKeyPress(CAMERA_ROTATE_LEFT))
	{
		m_pCamera->AdjustHeadingPitch(-lookSpeed, 0);
	}
	if (IsKeyPress(CAMERA_ROTATE_RIGHT))
	{
		m_pCamera->AdjustHeadingPitch(lookSpeed, 0);
	}
	if (IsKeyPress(CAMERA_ROTATE_UP))
	{
		m_pCamera->AdjustHeadingPitch(0, -lookSpeed);
	}
	if (IsKeyPress(CAMERA_ROTATE_DOWN))
	{
		m_pCamera->AdjustHeadingPitch(0, lookSpeed);
	}
	if (IsKeyPress(CAMERA_UP))
	{
		FVEC3 p = m_pCamera->GetPosition();
		p.y += 1 * _time.dtMS * m_pCamera->GetMovementSpeed();
		m_pCamera->SetPositionAndView(p.x, p.y, p.z, 0, 0);
	}
	if (IsKeyPress(CAMERA_DOWN))
	{
		FVEC3 p = m_pCamera->GetPosition();
		p.y -= 1 * _time.dtMS * m_pCamera->GetMovementSpeed();
		m_pCamera->SetPositionAndView(p.x, p.y, p.z, 0, 0);
	}
	// update

	m_pCamera->Update(_time);
}


void RasterRayApp::UpdateUI(const Time& _time)
{
	// Simple FPS counter
	static unsigned count;
	static float cMS;
	static float oMS;

	count++;
	cMS += (_time.dtMS);

	if (cMS >= (oMS + 1000.f))
	{
		m_FPS = count;
		count = 0;
		oMS = cMS;
	}

	// UI State Update
	if (ToggleOnKey(UI_HELP, toggleUI))
	{
		m_drawUIHelp = !m_drawUIHelp;
	}
}

void RasterRayApp::UpdateGlobalShaderBuffers(void)
{
	HRESULT hr = S_OK;

	/* Camera */
	FMAT4X4 world;
	TMatrixIdentity(world);
	CameraConstantBuffer ccb;

	ccb.worldViewProjection			= TMatrixTranspose(m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix());
	ccb.view						= TMatrixTranspose(m_pCamera->GetViewMatrix());
	ccb.projection					= TMatrixTranspose(m_pCamera->GetProjectionMatrix());
	ccb.world						= world;
	ccb.cameraPos					= m_pCamera->GetPosition();
	ccb.cameraDir					= m_pCamera->GetEyeDir();
	ccb.cameraRight					= m_pCamera->GetRight();
	ccb.cameraUp					= m_pCamera->GetUp();
	ccb.viewInverse					= XMMatrixInverse(&XMMatrixDeterminant(ccb.view), ccb.view);
	ccb.projectionInverse			= XMMatrixInverse(&XMMatrixDeterminant(ccb.projection), ccb.projection);
	ccb.worldViewProjectionInverse	= XMMatrixInverse(&XMMatrixDeterminant(ccb.worldViewProjection), ccb.worldViewProjection);
	ccb.rotation					= XMMatrixInverse(&XMMatrixDeterminant(ccb.rotation), ccb.rotation);

	ccb.cameraUp.z					= m_pCamera->GetFocalLength();

	D3D11_MAPPED_SUBRESOURCE msr = { 0 };

	hr = m_pContext->Map(m_pGlobalCameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	if (FAILED(hr))	{ PrintError(AT, hr); return;	}

	errno_t err = memcpy_s(msr.pData, msr.RowPitch, &ccb, sizeof(CameraConstantBuffer));
	m_pContext->Unmap(m_pGlobalCameraConstantBuffer, 0);

	ID3D11Buffer* cbuffers[]	= { m_pGlobalCameraConstantBuffer };
	const unsigned arrayLength	= sizeof(cbuffers) / sizeof(*cbuffers);

	/* Set Cbuffers */
	m_pContext->VSSetConstantBuffers(0, 1, &m_pGlobalCameraConstantBuffer);
	m_pContext->GSSetConstantBuffers(0, 1, &m_pGlobalCameraConstantBuffer);
	m_pContext->PSSetConstantBuffers(0, 1, &m_pGlobalCameraConstantBuffer);
	m_pContext->CSSetConstantBuffers(0, 1, &m_pGlobalCameraConstantBuffer);
}

HRESULT RasterRayApp::TakeScreenShot(ID3D11Texture2D* _pTexture)
{
	
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ee719655%28v=vs.85%29.aspx
	return S_FALSE;
}