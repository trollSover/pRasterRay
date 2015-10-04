#include "VoxelApp.h"
#include "../Global/SimpleInput.h"
//#include "../CoreSystems/Core.h"
//extern Core* g_pCoreSystem;
#include "DebugRenderer.h"
#include "MarchingCubes.h"
#include "../Render/ShaderInputFormats.h"
#include "../Global/TimerQPC.h"

#include <sstream>

#define RES_WIDTH	1280
#define RES_HEIGHT	720

VoxelApp::~VoxelApp()
{
	SAFE_DELETE(m_pCuller);
	SAFE_DELETE(m_pVoxelCB);
	SAFE_DELETE(m_pWindowCB);
	SAFE_DELETE(m_pEvalCB);
	SAFE_DELETE(m_pCameraCB);
	SAFE_DELETE(m_pTerrainVBuffer);
	SAFE_DELETE(m_pNodeBuffer);
	SAFE_DELETE(m_pVoxelBuffer);
	SAFE_DELETE(m_pRasterizer);
	SAFE_DELETE(m_pRayCaster);
	SAFE_DELETE(m_pBackBufferDeferred);
	SAFE_DELETE(m_pInput);
	SAFE_DELETE(m_pDriver);
}

bool VoxelApp::VInit()
{
	printf("Input...");
	m_pInput = new SimpleInput();
	if (!m_pInput || !m_pInput->VInit())
		return false;
	else printf("ok\n");

	printf("--- DirectX ---\n");
	m_pDriver = new DXDriver();
	if (!m_pDriver || !m_pDriver->Init(VGetResolution()))
		return false;
	else printf("DirectX driver ok/DirectX\n");

	m_eRenderType = RENDER_TYPE::RT_Debug;
	SetRenderType(m_eRenderType);
	m_cullMode = D3D11_CULL_NONE;
	m_fillMode = D3D11_FILL_SOLID;
	m_pDriver->SetRasterizerState(m_cullMode, m_fillMode);

	for (unsigned int i = 0; i < RENDERTARGET_COUNT; ++i)
		m_pRenderTargets[i] = nullptr;

	/* Create deferred render target and resources */
	RenderTargetInit rti;
	rti.formats = RTFS_SHADER_RESOURCE | RTFS_TEXTURE | RTFS_UNORDERED_ACCESS;

	// texture
	rti.TexDesc.Width = VGetResolution().width;
	rti.TexDesc.Height = VGetResolution().height;
	rti.TexDesc.MipLevels = 1;
	rti.TexDesc.ArraySize = 1;
	rti.TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rti.TexDesc.SampleDesc.Count = 1;
	rti.TexDesc.Usage = D3D11_USAGE_DEFAULT;
	rti.TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	rti.TexDesc.CPUAccessFlags = 0;
	rti.TexDesc.MiscFlags = 0;
	// view
	rti.RTVDesc.Format = rti.TexDesc.Format;
	rti.RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rti.RTVDesc.Texture2D.MipSlice = 0;
	// resource
	rti.SRVDesc.Format = rti.TexDesc.Format;
	rti.SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rti.SRVDesc.Texture2D.MostDetailedMip = 0;
	rti.SRVDesc.Texture2D.MipLevels = 1;
	// unordered access
	rti.UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	rti.UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	rti.UAVDesc.Texture2D.MipSlice = 0;

	// rendertarget: Color = {view, shader resource, unordered access}
	if (false == m_pDriver->CreateRenderTarget(RT_COLOR, rti, m_pRenderTargets[0]))
	{
		PrintError(AT, "failed to create rendertarget");
	}
	// rendertarget: Normal = {view, shader resource, unordered access}
	if (false == m_pDriver->CreateRenderTarget(RT_NORMAL, rti, m_pRenderTargets[1]))
	{
		PrintError(AT, "failed to create rendertarget");
	}

	rti.TexDesc.Format = DXGI_FORMAT_R32_FLOAT;	// single color for depth
	rti.RTVDesc.Format = rti.TexDesc.Format;
	rti.SRVDesc.Format = rti.TexDesc.Format;

	if (false == m_pDriver->CreateRenderTarget(RT_DEPTH, rti, m_pRenderTargets[2]))
	{
		PrintError(AT, "failed to create rendertarget");
		return false;
	}

	m_pBackBufferDeferred = VNEW BackBufferRender();
	if (!m_pBackBufferDeferred->VInit(m_pDriver))
	{
		PrintError(AT, "failed to create deferred backbuffer render stage");
		return false;
	}

	printf("--- Render Interfaces ---\n");
	//m_pRasterizer = VNEW  Rasterizer();
	//if (!m_pRasterizer->VInit(m_pDriver))
	//	return false;
	//else printf("Rasterizer ok\n");

	m_pRayCaster = VNEW RayCaster();
	if (!m_pRayCaster->VInit(m_pDriver))
		return false;
	else printf("Ray tracer ok\n");

	m_pDebugRenderer = VNEW DebugRenderer();
	if (!m_pDebugRenderer->VInit(m_pDriver))
		return false;
	else printf("Debug renderer ok\n");

	printf("--- User ---\n");
	if (!m_camera.Init(VGetResolution().width, VGetResolution().height))
		return false;
	else printf("Camera ok\n");

	printf("(Camera) ");
	m_pCameraCB = VNEW D3DBuffer();
	if (!m_pCameraCB->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_CONSTANT, 1, sizeof(CBDCamera)))
		return false;

	printf("(Window) ");
	m_pWindowCB = VNEW D3DBuffer();
	if (!m_pWindowCB->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_CONSTANT, 1, sizeof(CBDWindow)))
		return false;

	printf("(Voxel) ");
	m_pVoxelCB = VNEW D3DBuffer();
	if (!m_pVoxelCB->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_CONSTANT, 1, sizeof(CBDVoxel)))
		return false;

	printf("(Eval) ");
	m_pEvalCB = VNEW D3DBuffer();
	if (!m_pEvalCB->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_CONSTANT, 1, sizeof(CBDEval)))
		return false;

	printf("/User\n");

	printf("--- SVO ---\n");

	std::ifstream svoStream("master64.txt");

	if (!svoStream)
	{
		PrintError(AT, "failed to open master config");
		return false;
	}

	/* get the path of the voxelized model */
	std::string line;
	std::string svoPath;
	while (std::getline(svoStream, line))
	{
		if (line.find_first_of('#') == 0)	// comment line
			continue;

		if (line.find("source") != std::string::npos)
		{
			int end = line.find_first_of(' ') + 1;
			svoPath = line.erase(0, end);
		}
		printf("%s\n", line.c_str());
	}

	if (!m_svo.CreateSVO(svoPath))
		return false;
	else printf("Sparse Voxel Octree ok\n");

	std::vector<GPU_Voxel<NC>> voxs;

	GPU_Node* root = &m_svo.GetRoot();

	m_svo.GetSVO(*root, voxs);

	double iso = 0.5;
	std::vector<Vertex> vertices;
	CELL cs;
	m_svo.GetCell(*root, cs, 0);
	int N = 0;

	for (CELL& c : m_svo.cells)
	{
		TRIANGLE tri[5];
		N = Polygonize(c, iso, tri);

		for (int i = 0; i < N; ++i)
		{
			FVEC4 color = (c.c[0] + c.c[1] + c.c[2]) / 3.f;
			vertices.push_back(Vertex(tri[i].p[0], (c.n[i]), (color)));
			vertices.push_back(Vertex(tri[i].p[1], (c.n[i]), (color)));
			vertices.push_back(Vertex(tri[i].p[2], (c.n[i]), (color)));
		}
	}

	m_svo.cells.clear();

	if (vertices.size() == 0)
	{
		PrintError(AT, "failed to generate vertices");
		return false;
	}



	m_pDebugVBuffer = VNEW D3DBuffer();
	if (!m_pDebugVBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_VERTEX, vertices.size(), sizeof(Vertex), &vertices[0]))
	{
		PrintError(AT, "failed to create debug vertex buffer");
		return false;
	}

	std::vector<uint32_t> ninds;

	uint32_t gridLength = m_svo.m_svoLoader.m_header.gridlength;
	uint32_t rootPtr = m_svo.m_svoLoader.m_nodes.size() - 1;

	//m_camera.SetPositionAndView(maxX * 0.5f, minY + 256.f, maxZ * 0.5f, 0, 90);
	m_camera.SetPositionAndView(128, 256, -128, 0, 45);
	m_camera.Update(Time());
	m_pCuller = VNEW FrustumCulling();

	m_pCuller->CheckVoxels(m_svo.m_svoLoader.testNodes, rootPtr, ninds, FVEC3(gridLength, gridLength, gridLength) / 2);	// unused at the moment

	m_pTerrainVBuffer = VNEW D3DBuffer();

	printf("Points: %i RAM estimate: %i MB\n", vertices.size(), sizeof(GPU_Voxel<NC>)* vertices.size() / 1024 / 1024);
	printf("Voxels: %i RAM estimate: %i MB\n", m_svo.GetVoxelList().size(), (sizeof(GPU_Voxel<NC>) /* Other data? */)* m_svo.GetVoxelList().size() / 1024 / 1024);
	printf("Nodes: %i RAM estimate: %i MB\n", m_svo.GetNodeList().size(), sizeof(GPU_Node)* m_svo.GetNodeList().size() / 1024 / 1024);

	printf("(Terrain) ");
	if (!m_pTerrainVBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_VERTEX, m_svo.GetNodeList().size(), sizeof(GPU_Node), &m_svo.GetNodeList()[0]))
	{
		PrintError(AT, "failed to create vertex buffer");
		return false;
	}

	m_pTerrainIBuffer = VNEW D3DBuffer();
	if (!m_pTerrainIBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_INDEX, ninds.size(), sizeof(uint32_t), &ninds[0]))
	{
		PrintError(AT, "failed to create index buffer");
		return false;
	}

	size_t nElements = m_svo.GetNodeList().size();
	uint32_t sElement = sizeof(GPU_Node);

	m_pNodeBuffer = VNEW D3DBuffer();

	printf("(Node) ");
	if (!m_pNodeBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_SRV_UAV, nElements, sElement, &m_svo.m_svoLoader.m_nodes[0]))
	{
		PrintError(AT, "failed to create node buffer");
		return false;
	}

	nElements = m_svo.GetVoxelList().size();
	sElement = sizeof(GPU_Voxel<NC>);

	m_pVoxelBuffer = VNEW D3DBuffer();

	printf("(Voxel) ");
	if (!m_pVoxelBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_SRV_UAV, nElements, sElement, &m_svo.m_svoLoader.m_voxels[0]))
	{
		PrintError(AT, "failed to create voxel buffer");
		return false;
	}
	printf("/SVO\n");

	/* Volume */
	cbVoxel.numNodes = m_svo.m_svoLoader.m_header.numNodes;
	cbVoxel.rootIndex = m_svo.m_svoLoader.m_nodes.size() - 1;
	cbVoxel.gridLength = m_svo.m_svoLoader.m_header.gridlength;
	cbVoxel.rootDepth = m_svo.m_svoLoader.m_header.numLevels;

	if (FAILED(m_pDriver->MapSubResource(m_pVoxelCB->GetResource(), cbVoxel)))
	{
		PrintError(AT, "failed to map subresource");
		return false;
	}

	/* Window */
	cbWindow.width = VGetResolution().width;
	cbWindow.height = VGetResolution().height;

	if (FAILED(m_pDriver->MapSubResource(m_pWindowCB->GetResource(), cbWindow)))
	{
		PrintError(AT, "failed to map subresource");
		return false;
	}

	/* Eval */
	cbEval.depthDivider = 1000;
	cbEval.NLoD = 1;

	if (FAILED(m_pDriver->MapSubResource(m_pEvalCB->GetResource(), cbEval)))
	{
		PrintError(AT, "failed to map subresource");
		return false;
	}

	// push svo on gpu
	unsigned int initialCounts = -1;
	ID3D11UnorderedAccessView * uavs[] = { m_pVoxelBuffer->GetUAV(), m_pNodeBuffer->GetUAV() };
	m_pDriver->GetContext()->CSSetUnorderedAccessViews(0, 2, uavs, &initialCounts);

	// create font renderer
	IFW1Factory* fw1Factory;

	if (FAILED(FW1CreateFactory(FW1_VERSION, &fw1Factory)))
	{
		PrintError(AT, "failed to create font factory");
		return false;
	}

	if (FAILED(fw1Factory->CreateFontWrapper(m_pDriver->GetDevice(), L"Times New Roman", &m_pFontWrapper)))
	{
		PrintError(AT, "failed to create font wrapper");
		return false;
	}

	fw1Factory->Release();

#ifdef FDEBUG
	printf("Appliction: Ok\n");
#endif

	return true;
}

static bool toggleFill = false;
static bool toggleCull = false;
static bool toggleMove = true;
static int count;
static int fps;
static float cMS;
static float oMS;
static float movespeed = 1000;

bool VoxelApp::VFrame(Time time)
{

	count++;
	cMS += (time.dtMS / 1000.f);


	if (cMS >= (oMS + 0.0001f))
	{
		fps = count;
		count = 0;
		oMS = cMS;
	}

	HRESULT hr = S_OK;

	if (m_pInput->VIsKeyDown(VK_ESCAPE))
	{
		PostQuitMessage(0);
		return true;
	}

	/* Update Camera */
	m_pInput->VIsKeyDown('W') ? m_camera.SetMovementToggle(0, 1) : m_camera.SetMovementToggle(0, 0);
	m_pInput->VIsKeyDown('A') ? m_camera.SetMovementToggle(2, -1) : m_camera.SetMovementToggle(2, 0);
	m_pInput->VIsKeyDown('S') ? m_camera.SetMovementToggle(1, -1) : m_camera.SetMovementToggle(1, 0);
	m_pInput->VIsKeyDown('D') ? m_camera.SetMovementToggle(3, 1) : m_camera.SetMovementToggle(3, 0);


	const float movespeedIncr = 100.f;

	if (m_pInput->VIsKeyDown(VK_ADD) && toggleMove)
	{
		m_camera.SetMovementSpeed(movespeed += movespeedIncr);
		toggleMove = false;
	}
	if (m_pInput->VIsKeyDown(VK_SUBTRACT) && toggleMove)
	{
		m_camera.SetMovementSpeed(movespeed -= movespeedIncr);
		toggleMove = false;
	}

	if (!m_pInput->VIsKeyDown(VK_ADD) && !m_pInput->VIsKeyDown(VK_SUBTRACT))
	{
		toggleMove = true;
	}

	/* Mouse Look */
	if (m_pInput->MouseMoved())
	{
		const float lookBase = 10.25f;
		const float lookSpeed = lookBase * time.dtMS;
		m_camera.AdjustHeadingPitch(m_pInput->m_mousePoint.x * lookSpeed, m_pInput->m_mousePoint.y * lookSpeed);
	}
	else m_camera.AdjustHeadingPitch(0, 0);

	m_camera.Update(time);

	//m_pCuller->Construct(100, TMatrixTranspose(m_camera.GetProjectionMatrix()), TMatrixTranspose(m_camera.GetViewMatrix()));	// unused at the moment

	if (FAILED(hr))
	{
		PrintError(AT, "failed to map subresource");
	}

	/**********************
	--- Update cbuffers ---
	***********************/
	/* Camera */
	FMAT4X4 world;
	TMatrixIdentity(world);

	cbCamera.mWVP				= m_camera.GetVPMatrix();
	cbCamera.mView				= m_camera.GetViewMatrix();
	cbCamera.mProjection		= m_camera.GetProjectionMatrix();
	cbCamera.mWorld				= world;
	cbCamera.cameraPos			= m_camera.GetPosition();
	cbCamera.cameraDir			= m_camera.GetEyeDir();
	cbCamera.right				= m_camera.GetRight();
	cbCamera.up					= m_camera.GetUp();
	cbCamera.mViewInverse		= m_camera.m_inverseViewMatrix;
	cbCamera.mProjectionInverse = m_camera.m_inverseProjectionMatrix;
	cbCamera.mWVPInverse		= m_camera.m_inverseWVP;
	cbCamera.mRotation			= m_camera.m_rotationMatrix;

	//	FMAT4X4	m_rotationMatrix;
	//FMAT4X4 m_inverseViewMatrix;
	//FMAT4X4 m_inverseProjectionMatrix;
	//FMAT4X4 m_inverseWVP;
	//FMAT4X4	m_viewProjectionMatrix;

	hr = m_pDriver->MapSubResource(m_pCameraCB->GetResource(), cbCamera);
	if (FAILED(hr))
	{
		PrintError(AT, "failed to map subresource");
	}

	ID3D11Buffer* cbuffers[] = { (ID3D11Buffer*)m_pCameraCB->GetResource(), (ID3D11Buffer*)m_pWindowCB->GetResource(), (ID3D11Buffer*)m_pVoxelCB->GetResource() };

	m_pDriver->GetContext()->VSSetConstantBuffers(0, 2, cbuffers);
	m_pDriver->GetContext()->GSSetConstantBuffers(0, 2, cbuffers);
	m_pDriver->GetContext()->PSSetConstantBuffers(0, 2, cbuffers);
	m_pDriver->GetContext()->CSSetConstantBuffers(0, 3, cbuffers);

	/* Draw */
	m_pDriver->BeginScene();



	// rasterize deferred
	m_pDebugRenderer->VDraw(m_pDriver, m_pDebugVBuffer, m_pTerrainIBuffer);
	// raycast deferred
	m_pRayCaster->VDraw(m_pDriver, nullptr);

	// finalize deferred by merging the different targets
	m_pBackBufferDeferred->VDraw(m_pDriver, nullptr);

	FVEC3 cpos = m_camera.GetPosition();
	FVEC3 cdir = m_camera.GetEyeDir();

	std::wostringstream woss;
	woss << "Camera::Position = " << cpos.x << ":" << cpos.y << ":" << cpos.z << "\n" << "Camera::Direction = " << cdir.x << ":" << cdir.y << ":" << cdir.z << "\nCamera::Speed = " << m_camera.GetMovementSpeed();
	std::wstring ctext = woss.str();

	m_pFontWrapper->DrawString(m_pDriver->GetContext(), ctext.c_str(), 14.f, 10, 10, 0xffff1612, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE);

	std::wostringstream foss;
	foss << "FPS = " << fps;
	std::wstring uitext = foss.str();

	m_pFontWrapper->DrawString(m_pDriver->GetContext(), uitext.c_str(), 14.f, 10, 60, 0xffff1612, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE);

	//ID3D11RenderTargetView* empty = nullptr;
	//m_pDriver->SetRenderTargets(&empty, 1);

	return m_pDriver->EndScene();
}


Resolution VoxelApp::VGetResolution()const
{
	return{ RES_WIDTH, RES_HEIGHT };
}

const LPCSTR VoxelApp::VGetAppName()const
{
	return "VoxelApp";
}

LRESULT VoxelApp::VApplicationProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
	switch (_umsg)
	{
	case WM_INPUT:
		m_pInput->Update(_hwnd, (HRAWINPUT&)_lparam);
		break;
	default:
		return DefWindowProc(_hwnd, _umsg, _wparam, _lparam);
	}

	return 0;
}

void VoxelApp::SetRenderType(RENDER_TYPE _renderType)
{
	assert(_renderType < RENDER_TYPE::RT_Unknown);

	m_eRenderType = _renderType;
	static const char* types[] = { "Rasterizer", "Raytracer", "Debug" };

}

// some weird bug that keeps culling until toggled once - fix!
// note: bug seems to be caused by wireframe mode
void VoxelApp::ToggleCullMode()
{
	if (m_cullMode == D3D11_CULL_BACK)
	{
		printf("[Cull Mode] = None\n");
		m_cullMode = D3D11_CULL_NONE;
	}
	else
	{
		printf("[Cull Mode] = Back\n");
		m_cullMode = D3D11_CULL_BACK;
	}

	m_pDriver->SetRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID);	// bug temp solution: just force no cull
}

void VoxelApp::ToggleFillMode()
{
	if (m_fillMode == D3D11_FILL_SOLID)
	{
		printf("[Fill Mode] = Wireframe\n");
		m_fillMode = D3D11_FILL_WIREFRAME;
	}
	else
	{
		printf("[Fill Mode] = Solid\n");
		m_fillMode = D3D11_FILL_SOLID;
	}
	m_pDriver->SetRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID);	// bug temp solution: just force no cull
}