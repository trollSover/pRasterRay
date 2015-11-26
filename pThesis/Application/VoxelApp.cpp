#include "VoxelApp.h"
#include "../Global/SimpleInput.h"
//#include "../CoreSystems/Core.h"
//extern Core* g_pCoreSystem;
#include "DebugRenderer.h"
#include "MarchingCubes.h"
#include "../Render/ShaderInputFormats.h"
#include "../Global/TimerQPC.h"

#include <sstream>
#include <bitset>

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

	//if (perfHandle != nullptr)
	//{
	//	bool freeLibOk = FreeLibrary(perfHandle);
	//	if (!freeLibOk)
	//	{
	//		printf("failed to unload GPUPerfAPI\n");
	//	}
	//}
}

bool VoxelApp::VInit()
{
	/* Performance counter init */

	drawRasterized	= true;
	drawRayCast		= true;
	drawUIHelp		= false;

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

#ifdef _PERFTRACK
	m_renderloopPerfContext.Init(m_pDriver->GetPerfLibrary(), m_pDriver->GetDevice());
#endif
	//gpa_uint32 counter;
	//m_renderloopPerfContext.m_pLibTable->getCounterIndex("D3DGPUTime", &counter);

	m_eRenderType = RENDER_TYPE::RT_Debug;
	SetRenderType(m_eRenderType);
	m_cullMode = D3D11_CULL_BACK;// D3D11_CULL_NONE;
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

	printf("(Octree) ");
	m_pOctreeMatrices = VNEW D3DBuffer();
	if (!m_pOctreeMatrices->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_CONSTANT, 1, sizeof(CBOctreeMatrices)))
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
			break;
		}
		printf("%s\n", line.c_str());
	}

	if (!m_svo.CreateSVO(svoPath))
		return false;
	else printf("Sparse Voxel Octree ok\n");

	std::vector<GPU_Voxel<NC>> voxs;

	TNode* root = &m_svo.GetRoot();
	int rtptr = m_svo.GetNodeList().size();
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

		//for (int i = 0; i < N; ++i)
		//{
		//	FVEC4 color = (c.c[0] + c.c[1] + c.c[2]) / 3.f;
 	//		vertices.push_back(Vertex(tri[i].p[0], (c.n[0]), c.c[0]));
		//	vertices.push_back(Vertex(tri[i].p[1], (c.n[1]), c.c[1]));
		//	vertices.push_back(Vertex(tri[i].p[2], (c.n[5]), c.c[5]));
		//}
		if (N == 2)
		{
			vertices.push_back(Vertex(tri[0].p[0], (c.n[0]), c.c[0]));
			vertices.push_back(Vertex(tri[0].p[1], (c.n[1]), c.c[1]));
			vertices.push_back(Vertex(tri[0].p[2], (c.n[4]), c.c[4]));

			vertices.push_back(Vertex(tri[1].p[0], (c.n[1]), c.c[1]));
			vertices.push_back(Vertex(tri[1].p[1], (c.n[4]), c.c[4]));
			vertices.push_back(Vertex(tri[1].p[2], (c.n[5]), c.c[5]));
		}
		else if (N == 0)
		{

		}
		else
			printf("what?");
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
	m_camera.SetMovementSpeed(1);
	m_camera.Update(Time());
	m_pCuller = VNEW FrustumCulling();

	//m_pCuller->CheckVoxels(m_svo.m_svoLoader.m_nodes, rootPtr, ninds, FVEC3(gridLength, gridLength, gridLength) / 2);	// unused at the moment

	m_pTerrainVBuffer = VNEW D3DBuffer();

	printf("Points: %i RAM estimate: %i MB\n", vertices.size(), sizeof(GPU_Voxel<NC>)* vertices.size() / 1024 / 1024);
	printf("Voxels: %i RAM estimate: %i MB\n", m_svo.GetVoxelList().size(), (sizeof(GPU_Voxel<NC>) /* Other data? */)* m_svo.GetVoxelList().size() / 1024 / 1024);
	printf("Nodes: %i RAM estimate: %i MB\n", m_svo.GetNodeList().size(), sizeof(TNode)* m_svo.GetNodeList().size() / 1024 / 1024);

	printf("(Terrain) ");
	if (!m_pTerrainVBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_VERTEX, m_svo.GetNodeList().size(), sizeof(TNode), &m_svo.GetNodeList()[0]))
	{
		PrintError(AT, "failed to create vertex buffer");
		return false;
	}
	m_pTerrainIBuffer = nullptr;
	//m_pTerrainIBuffer = VNEW D3DBuffer();
	//if (!m_pTerrainIBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_INDEX, ninds.size(), sizeof(uint32_t), &ninds[0]))
	//{
	//	PrintError(AT, "failed to create index buffer");
	//	return false;
	//}

	size_t nElements = m_svo.GetNodeList().size();
	uint32_t sElement = sizeof(TNode);

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
	std::vector<Voxel> voxels;
	voxels.reserve(m_svo.m_svoLoader.m_voxels.size());
	voxels.insert(voxels.end(), m_svo.m_svoLoader.m_voxels.begin(), m_svo.m_svoLoader.m_voxels.end());

	if (!m_pVoxelBuffer->Init(m_pDriver->GetDevice(), BT_STRUCTURED, BB_SRV_UAV, voxels.size(), sizeof(Voxel), &voxels[0]))
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

	std::ofstream stream;
	stream.open("root.txt");
	TNode* pNode = root;
	stream << "root has " << pNode->CountChildren() << " children\naddress\t\t\t= " << rootPtr << "\n";
	stream << "child mask " << pNode->children << "\t= " << std::bitset<32>(pNode->children).to_string() << "\n";
	stream << "children\t\t= " << std::bitset<8>(pNode->children).to_string() << "\n\n";
	for (int i = 0; i < 8; ++i)
	{
		if (pNode->HasChildAtIndex(i))
		{
			TNode* pChild = &m_svo.m_svoLoader.m_nodes[pNode->GetChildAddress(i)];
			stream << "child: " << i << " has " << pChild->CountChildren() << " children\naddress\t\t\t= " << pNode->GetChildAddress(i) << "\n";
			stream << "child mask " << pChild->children << "\t= " << std::bitset<32>(pChild->children).to_string() << "\n";
			stream << "children\t\t= " << std::bitset<8>(pChild->children).to_string() << "\n\n";
		}
	}
	stream.close();

	return true;
}

static bool toggleUI = true;
static bool toggleRasterize = true;
static bool toggleRayCast = true;
static bool toggleFill = true;
static bool toggleCull = true;
static bool toggleMove = true;
static int count;
static int fps;
static float cMS;
static float oMS;
static float movespeed = 1;


static gpa_uint32 currentWaitSessionID = 1;

bool VoxelApp::VFrame(Time time)
{
	HRESULT hr = S_OK;

	if (KeyPress(VK_ESCAPE))
	{
		PostQuitMessage(0);
		return true;
	}

	/* Update UI */
	UpdateUI(time);

	/* Update Camera */
	UpdateCamera(time);

	/* Update Render Settings */
	// render state
	UpdateRenderState();
	// vertex culling
	//m_pCuller->Construct(100, TMatrixTranspose(m_camera.GetProjectionMatrix()), TMatrixTranspose(m_camera.GetViewMatrix()));	// unused at the moment
	// buffer mapping
	UpdateBuffers();
	
	/* Draw */
	m_pDriver->BeginScene();

	// draw terrain
	RenderGeometry();

	// finalize deferred by merging the different targets
	SetFillMode(D3D11_FILL_SOLID); // fill mode
	m_pBackBufferDeferred->VDraw(m_pDriver, nullptr);
	
	// draw UI
	RenderUI(time);

	// reset rasterizer state
	SetFillMode(m_fillMode);
	SetCullMode(m_cullMode);

	///* End Draw */
	return m_pDriver->EndScene();
	//return frameOk;
}

void VoxelApp::RenderUI(const Time& _time)
{
	FVEC3 cpos = m_camera.GetPosition();
	FVEC3 cdir = m_camera.GetEyeDir();

	const float fontSize = 16.0f;
	const float indx = 10.0f;
	float indy = 10.0f;
	
	std::wostringstream stream;
	stream.precision(5);
	std::wstring str;
	stream << "Camera::Position\nx: " << cpos.x << "\ny: " << cpos.y << "\nz: " << cpos.z << "\n" <<
			"Camera::Direction\nx: " << cdir.x << "\ny: " << cdir.y << "\nz: " << cdir.z <<
			"\nCamera::Speed: " << m_camera.GetMovementSpeed();

	str = stream.str();

	m_pFontWrapper->DrawString(m_pDriver->GetContext(), str.c_str(), fontSize, indx, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	stream << "FPS = " << fps << "\ndt(ms) = " << _time.dtMS;
	str = stream.str();

	m_pFontWrapper->DrawString(m_pDriver->GetContext(), str.c_str(), fontSize, indx, 180, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	stream <<	"Cull Mode: " << (m_cullMode == D3D11_CULL_BACK ? "back" : "none") << 
				"\nFill Mode: " << (m_fillMode == D3D11_FILL_SOLID ? "solid" : "wireframe") <<
				"\nRasterizer: " << (drawRasterized ? "on" : "off") <<
				"\nRayCaster: " << (drawRayCast ? "on" : "off");

	str = stream.str();

	m_pFontWrapper->DrawString(m_pDriver->GetContext(), str.c_str(), fontSize, 200, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	//cbOctreeMatrices.pixelInOctree = TVectorLength(pixel);
	//cbOctreeMatrices.viewportToCamera = viewportToCamera;
	////cbOctreeMatrices.cameraToOctree		= XMMatrixInverse(&XMMatrixDeterminant(octreeToWorld), octreeToWorld);
	//cbOctreeMatrices.cameraToOctree = XMMatrixInverse(&XMMatrixDeterminant(cameraToOctree), cameraToOctree);
	FMAT4X4 cto = cbOctreeMatrices.cameraToOctree;
	FMAT4X4 vtc = cbOctreeMatrices.viewportToCamera;
	stream << "CameraToOctree\n";//
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			// fixed float size + add extra empty space if positive value
			stream << std::fixed << cto.m[i][j] << (cto.m[i][j] < 0 ? "| " : " | ");
		}
		stream << "\n";
	}
	str = stream.str();
	m_pFontWrapper->DrawString(m_pDriver->GetContext(), str.c_str(), fontSize, 400, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	stream << "ViewportToCamera\n";//
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			stream << std::fixed << vtc.m[i][j] << " | ";
		}
		stream << "\n";
	}
	str = stream.str();
	m_pFontWrapper->DrawString(m_pDriver->GetContext(), str.c_str(), fontSize, 700, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	const WCHAR* help = L"Press 'h' for control mappings";
	m_pFontWrapper->DrawString(m_pDriver->GetContext(), help, fontSize, VGetResolution().width - 256, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);

	if (drawUIHelp)
	{
		const WCHAR* help = L"Move: WASD\nLook: Mouse\nToggle Cull: F1\nToggle Fill: F2\nCamera Speed: Numpad +/-\nToggle Rasterize: 1\nToggle RayCast: 2\nCamera Reset: Home";
		m_pFontWrapper->DrawString(m_pDriver->GetContext(), help, fontSize, VGetResolution().width - 256, 10 + fontSize + 3, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	}
}

//#define GPUPERF
void VoxelApp::RenderGeometry(void)
{
#ifdef _PERFTRACK
	bool frameOk = true;

	gpa_uint32 sessionID;

	GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->beginSession(&sessionID));

	gpa_uint32 numRequiredPasses;
	GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->getPassCount(&numRequiredPasses));

	for (gpa_uint32 i = 0; i < numRequiredPasses; ++i)
	{
		m_pDriver->BeginScene();

		GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->beginPass());

		GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->beginSample(0));

		if (drawRasterized)
		{
			m_pDebugRenderer->VDraw(m_pDriver, m_pDebugVBuffer, m_pTerrainIBuffer);
		}
		else
		{
			// clear deferred rendertargets
			RenderTarget* renderTargets[3];
			m_pDriver->GetRenderTarget(RT_NORMAL, renderTargets[0]);
			m_pDriver->GetRenderTarget(RT_DEPTH, renderTargets[1]);
			m_pDriver->GetRenderTarget(RT_COLOR, renderTargets[2]);
			ID3D11RenderTargetView* rtvs[3];
			renderTargets[0]->GetView(*&rtvs[0]);
			renderTargets[1]->GetView(*&rtvs[1]);
			renderTargets[2]->GetView(*&rtvs[2]);
			m_pDriver->ClearRenderTargets(rtvs, 3);
		}

		// raycast deferred
		if (drawRayCast)
		{
			unsigned int initialCounts = -1;
			ID3D11UnorderedAccessView * uavs[] = { m_pVoxelBuffer->GetUAV(), m_pNodeBuffer->GetUAV() };
			m_pDriver->GetContext()->CSSetUnorderedAccessViews(0, 2, uavs, NULL);

			m_pRayCaster->VDraw(m_pDriver, nullptr);
		}

		GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->endSample());

		GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->endPass());
	}

	GPA_OPEN_SAFE(m_renderloopPerfContext.m_pLibTable->endSession());

	bool ready = false;
	if (sessionID != currentWaitSessionID)
	{
		GPA_Status status = m_renderloopPerfContext.m_pLibTable->isSessionReady(&ready, currentWaitSessionID);
		while (status == GPA_STATUS_ERROR_SESSION_NOT_FOUND)
		{
			currentWaitSessionID++;
			status = m_renderloopPerfContext.m_pLibTable->isSessionReady(&ready, currentWaitSessionID);
		}

	}
	if (ready)
	{
		m_renderloopPerfContext.WriteSession(currentWaitSessionID, "counterResults.csv");
		currentWaitSessionID++;
	}

#else

	// rasterize deferred
	if (drawRasterized)
	{
		m_pDebugRenderer->VDraw(m_pDriver, m_pDebugVBuffer, m_pTerrainIBuffer);
	}
	else
	{
		// clear deferred rendertargets
		RenderTarget* renderTargets[3];
		m_pDriver->GetRenderTarget(RT_NORMAL, renderTargets[0]);
		m_pDriver->GetRenderTarget(RT_DEPTH, renderTargets[1]);
		m_pDriver->GetRenderTarget(RT_COLOR, renderTargets[2]);
		ID3D11RenderTargetView* rtvs[3];
		renderTargets[0]->GetView(*&rtvs[0]);
		renderTargets[1]->GetView(*&rtvs[1]);
		renderTargets[2]->GetView(*&rtvs[2]);
		m_pDriver->ClearRenderTargets(rtvs, 3);
	}

	// raycast deferred
	if (drawRayCast)
	{
		unsigned int initialCounts = -1;
		ID3D11UnorderedAccessView * uavs[] = { m_pVoxelBuffer->GetUAV(), m_pNodeBuffer->GetUAV() };
		m_pDriver->GetContext()->CSSetUnorderedAccessViews(0, 2, uavs, NULL);

		m_pRayCaster->VDraw(m_pDriver, nullptr);
	}
#endif

}

void VoxelApp::UpdateUI(const Time& _time)
{
	// Simple FPS counter
	count++;
	cMS += (_time.dtMS);


	if (cMS >= (oMS + 1000.f))
	{
		fps = count;
		count = 0;
		oMS = cMS;
	}

	// UI State Update
	if (ToggleOnKey(UI_HELP, toggleUI))
	{
		drawUIHelp = !drawUIHelp;
	}
}

void VoxelApp::UpdateBuffers(void)
{
	HRESULT hr = S_OK;

	/* Update cbuffers */

	/* Camera */
	FMAT4X4 world;
	TMatrixIdentity(world);

	//cbCamera.mWVP				= TMatrixTranspose(m_camera.GetVPMatrix());
	cbCamera.mWVP				= TMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix());
	cbCamera.mView				= TMatrixTranspose(m_camera.GetViewMatrix());
	cbCamera.mProjection		= TMatrixTranspose(m_camera.GetProjectionMatrix());
	cbCamera.mWorld				= world;
	cbCamera.cameraPos			= m_camera.GetPosition();
	cbCamera.cameraDir			= m_camera.GetEyeDir();
	cbCamera.right				= m_camera.GetRight();
	cbCamera.up					= m_camera.GetUp();
	cbCamera.mViewInverse		= XMMatrixInverse(&XMMatrixDeterminant(cbCamera.mView), cbCamera.mView);
	cbCamera.mProjectionInverse = XMMatrixInverse(&XMMatrixDeterminant(cbCamera.mProjection), cbCamera.mProjection);
	cbCamera.mWVPInverse		= XMMatrixInverse(&XMMatrixDeterminant(cbCamera.mWVP), cbCamera.mWVP);
	cbCamera.mRotation			= m_camera.m_rotationMatrix;

	hr = m_pDriver->MapSubResource(m_pCameraCB->GetResource(), cbCamera);
	if (FAILED(hr))
	{
		PrintError(AT, "failed to map subresource");
	}
	int vsize = sizeof(FVEC3);
	/* Octree Matrices */
	//FVEC4 aabbMin = FVEC4(128, 128, 128, 0) * -1; // FVEC4(0, 0, 0, 0);// 
	//FVEC4 aabbMax = FVEC4(128, 128, 128, 1);
	float hsize = m_svo.m_svoLoader.m_header.gridlength / 2;
	FVEC4 aabbMin = FVEC4(0, 0, 0, 0); 
	//FVEC4 aabbMin = FVEC4(-hsize, -hsize, -hsize, 1);
	FVEC4 aabbMax = FVEC4(hsize, hsize, hsize, 1);
	

	//FVEC4 aabbMax = FVEC4(1,1,1,1);
	FVEC4 offset = aabbMin + aabbMax;

	FMAT4X4 octreeToWorld = TTranslateMatrix(offset);

	FVEC3 scale					= FVEC3( FVEC2(2.f) / FVEC2(VGetResolution().width, VGetResolution().height), 1.f);	//X

	FMAT4X4 worldToCamera = m_camera.GetViewMatrix();
	FMAT4X4 toWorld			= worldToCamera * octreeToWorld;


	FVEC3 pixel = Cross(FVEC3(toWorld.m[0][0], toWorld.m[1][0], toWorld.m[2][0]), FVEC3(toWorld.m[0][1], toWorld.m[1][1], toWorld.m[2][1]));

	float dim = 256;
	FVEC3 min, max;
	max = FVEC3(dim, dim, dim);
	
	FVEC3 pos[] =
	{
		FVEC3(0, 0, 0),
		FVEC3(1, 0, 0),
		FVEC3(0, 1, 0),
		FVEC3(1, 1, 0),
		FVEC3(0, 0, 1),
		FVEC3(1, 0, 1),
		FVEC3(0, 1, 1),
		FVEC3(1, 1, 1),
	};
	//printf("DEPTH: 0\n");
	for (int i = 0; i < 8; ++i)
	{
		FVEC3 cmin, cmax;
		cmin = min + (pos[i] * (dim * 0.5f));
		cmax = cmin + (pos[7] * (dim * 0.5f));
		//printf("child: %i\nMin{%.2f:%.2f:%.2f} \nMax{%.2f:%.2f:%.2f}\n", i, cmin.x, cmin.y, cmin.z, cmax.x, cmax.y, cmax.z);
		//printf("DEPTH: 1\n");
		for (int j = 0; j < 8; ++j)
		{
			FVEC3 dmin, dmax;
			dmin = cmin + (pos[j] * (dim * 0.25f));
			dmax = dmin + (pos[7] * (dim * 0.25f));
			//printf("\tchild: %i\n\tMin{%.2f:%.2f:%.2f} \n\tMax{%.2f:%.2f:%.2f}\n", j, dmin.x, dmin.y, dmin.z, dmax.x, dmax.y, dmax.z);
		}
		
	}
	int a = 42;
	/*
	// Non-transposed matrices
	/*cbOctreeMatrices.pixelInOctree		= TVectorLength(pixel);
	cbOctreeMatrices.viewportToCamera	= TMatrixInverse(m_camera.GetProjectionMatrix()) * TMatrixTranspose(TTranslateMatrix(FVEC3(-1.f, -1.f, 0.f))) * TScaleMatrix(FVEC4(scale.x, scale.y, scale.z, 0));
	cbOctreeMatrices.cameraToOctree		= TMatrixTranspose(TTranslateMatrix(FVEC3(1,1,1))) * TMatrixInverse( worldToCamera * octreeToWorld);
*/
	// AABB box
	// Vec3 hi, lo
	// maxDisp = 0.f + ?
	// octreeToObject = Translate( Vec3( lo + hi ) * 0.5f ) * Scale( Vec3( hi - lo).max() + maxDisp * 2.f ) ) * Translate( Vec3(-0.5f) )
	// octreeToWorld = 
	// worldToCamera =
	// -----------------------
	// Matrix3 orient = 
	// Matrix3 r =
	// r.column[2] = Normalize( Camera::Forward )
	// r.column[0] = Normalize( Cross( Camera::Up, r.column[2] ) )
	// r.column[1] = Normalize( Cross( r.column[2], r.column[0] ) )
	// -----------------------
	// Vec3 pos = Transpose( orient ) * Camera::Position
	// Matrix4 r =
	// r.row[0] = { Vec4( orient.column[0], -pos.x ) }
	// r.row[1] = { Vec4( orient.column[1], -pos.y ) }
	// r.row[2] = { Vec4( orient.column[2], -pos.z ) }
	// r.row[3] = { 0, 0, 0, 1 }
	// ------------------------

	// scale = Vec3( Vec2(2.f) / Vec2(Resolution), 1.f)
	// viewportToCamera = Invert(Projection) * Translate( Vec3(-1,-1,0) ) * Scale(scale)
	// cameraToOctree = Translate( Vec3(1.f) ) * Invert( (worldToCamera * octreeToWorld) )
	// pixelInOctree =
	cbOctreeMatrices.pixelInOctree = TVectorLength(pixel);
	cbOctreeMatrices.viewportToCamera = TMatrixInverse(m_camera.GetProjectionMatrix()) * TTranslateMatrix(FVEC3(-1.f, -1.f, 0.f)) * TScaleMatrix(FVEC4(scale.x, scale.y, scale.z, 0));
	cbOctreeMatrices.cameraToOctree = TTranslateMatrix(FVEC3(1, 1, 1)) * TMatrixInverse(worldToCamera * octreeToWorld);


	//cbOctreeMatrices.viewportToCamera = TMatrixTranspose(cbOctreeMatrices.viewportToCamera);
	//cbOctreeMatrices.cameraToOctree = TMatrixTranspose(cbOctreeMatrices.cameraToOctree);
	
	hr = m_pDriver->MapSubResource(m_pOctreeMatrices->GetResource(), cbOctreeMatrices);
	if (FAILED(hr))
	{
		PrintError(AT, "failed to map subresource");
	}

	ID3D11Buffer* cbuffers[] = {	(ID3D11Buffer*)m_pCameraCB->GetResource(),
									(ID3D11Buffer*)m_pWindowCB->GetResource(),
									(ID3D11Buffer*)m_pVoxelCB->GetResource(),
									(ID3D11Buffer*)m_pOctreeMatrices->GetResource() };

	/* Set Cbuffers */
	m_pDriver->GetContext()->VSSetConstantBuffers(0, 4, cbuffers);
	m_pDriver->GetContext()->GSSetConstantBuffers(0, 4, cbuffers);
	m_pDriver->GetContext()->PSSetConstantBuffers(0, 4, cbuffers);
	m_pDriver->GetContext()->CSSetConstantBuffers(0, 4, cbuffers);
}

void VoxelApp::UpdateRenderState(void)
{
	// Rasterizer State Updates
	if (ToggleOnKey(VK_F1, toggleCull))
	{
		m_cullMode == D3D11_CULL_BACK ? m_cullMode = D3D11_CULL_NONE : m_cullMode = D3D11_CULL_BACK;
		SetCullMode(m_cullMode);
	}

	if (ToggleOnKey(VK_F2, toggleFill))
	{
		m_fillMode == D3D11_FILL_SOLID ? m_fillMode = D3D11_FILL_WIREFRAME : m_fillMode = D3D11_FILL_SOLID;
		SetFillMode(m_fillMode);
	}

	// Render State Update
	if (ToggleOnKey(TOGGLE_RASTERIZE, toggleRasterize))
	{
		drawRasterized = !drawRasterized;
	}

	if (ToggleOnKey(TOGGLE_RAYCAST, toggleRayCast))
	{
		drawRayCast = !drawRayCast;
	}
}

static bool toggleCameraReset = true;

void VoxelApp::UpdateCamera(const Time& _time)
{
	// Camera Reset
	if (ToggleOnKey(CAMERA_RESET_ALL, toggleCameraReset))
	{
		m_camera.SetMovementSpeed(1);
		m_camera.SetPositionAndView(0, 0, 0, 0, 0);
	}

	// WASD movement
	KeyPress(CAMERA_FORWARD)? m_camera.SetMovementToggle(0, 1) : m_camera.SetMovementToggle(0, 0);
	KeyPress(CAMERA_LEFT)	? m_camera.SetMovementToggle(2, -1) : m_camera.SetMovementToggle(2, 0);
	KeyPress(CAMERA_BACK)	? m_camera.SetMovementToggle(1, -1) : m_camera.SetMovementToggle(1, 0);
	KeyPress(CAMERA_RIGHT)	? m_camera.SetMovementToggle(3, 1) : m_camera.SetMovementToggle(3, 0);

	// Set Camera Move Speed
	if (ToggleOnKey(CAMERA_SPEED_INCREMENT, toggleMove))
	{
		if (m_camera.GetMovementSpeed() >= CAMERA_SPEED_MAX)
		{
			m_camera.SetMovementSpeed(CAMERA_SPEED_MAX);
		}
		else
		{
			float incr = movespeed * 0.01;
			m_camera.SetMovementSpeed(movespeed += incr);
		}
		
	}
	if (ToggleOnKey(CAMERA_SPEED_DECREMENT, toggleMove))
	{
		if (m_camera.GetMovementSpeed() <= CAMERA_SPEED_MIN)
		{
			m_camera.SetMovementSpeed(CAMERA_SPEED_MIN);
		}
		else
		{
			float decr = -movespeed * 0.01;
			m_camera.SetMovementSpeed(movespeed += decr);
		}
	}
	// Reset Camera Move Speed
	if (ToggleOnKey(CAMERA_SPEED_RESET, toggleMove))
	{
		m_camera.SetMovementSpeed(1);
	}

	/* Camera Rotate */
	const float lookBase = 1.f / 1000.f;
	const float lookSpeed = lookBase * _time.dtMS;

	if (m_pInput->MouseMoved())
	{
		m_camera.AdjustHeadingPitch(m_pInput->m_mousePoint.x * lookSpeed, m_pInput->m_mousePoint.y * lookSpeed);
	}
	if (KeyPress(CAMERA_ROTATE_LEFT))
	{
		m_camera.AdjustHeadingPitch(-lookSpeed, 0);
	}
	if (KeyPress(CAMERA_ROTATE_RIGHT))
	{
		m_camera.AdjustHeadingPitch(lookSpeed, 0);
	}
	if (KeyPress(CAMERA_ROTATE_UP))
	{
		m_camera.AdjustHeadingPitch(0, -lookSpeed);
	}
	if (KeyPress(CAMERA_ROTATE_DOWN))
	{
		m_camera.AdjustHeadingPitch(0, lookSpeed);
	}
	if (KeyPress(CAMERA_UP))
	{
		FVEC3 p = m_camera.GetPosition();
		p.y += 1 * _time.dtMS * m_camera.GetMovementSpeed();
		m_camera.SetPositionAndView(p.x, p.y, p.z, 0, 0);
	}
	if (KeyPress(CAMERA_DOWN))
	{
		FVEC3 p = m_camera.GetPosition();
		p.y -= 1 * _time.dtMS * m_camera.GetMovementSpeed();
		m_camera.SetPositionAndView(p.x, p.y, p.z, 0, 0);
	}
	// update

	m_camera.Update(_time);
}

bool VoxelApp::KeyPress(unsigned int _key)
{
	return m_pInput->VIsKeyDown(_key);
}

bool VoxelApp::ToggleOnKey(unsigned int _key, bool& _toggle)
{
	// key pressed and we can toggle
	if (KeyPress(_key) && _toggle)
	{
		_toggle = false;
		return true;
	}
	// key not pressed - reset toggle
	if (!KeyPress(_key))
	{
		_toggle = true;		
	}

	// toggle is false - ie keypress is continous - do not toggle in this case
	return false;
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

// TODO: remove
void VoxelApp::SetRenderType(RENDER_TYPE _renderType)
{
	assert(_renderType < RENDER_TYPE::RT_Unknown);

	m_eRenderType = _renderType;
}

void VoxelApp::SetCullMode(const D3D11_CULL_MODE& _mode)
{
	m_pDriver->SetRasterizerState(_mode, m_fillMode);
}

void VoxelApp::SetFillMode(const D3D11_FILL_MODE& _mode)
{
	m_pDriver->SetRasterizerState(m_cullMode, _mode);
}