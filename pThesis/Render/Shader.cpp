#include "Shader.h"

#include "Render_Create.h"

#pragma comment(lib, "D3DCompiler.lib")

bool Shader::Init(LPCWSTR _file, ID3D11Device* _device)
{
	printf("[Compiling] %ls ...", _file);

	HRESULT hr = S_OK;

	ID3DBlob* compiledShader = nullptr;
	ID3DBlob* error = nullptr;
	FILE* shaderBlob = nullptr;

	DWORD dwsf = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL;
#ifdef _DEBUG
	dwsf |= D3DCOMPILE_DEBUG;
	dwsf |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	dwsf |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	hr = D3DCompileFromFile(_file, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", VType(), dwsf, NULL, &compiledShader, &error);
	
	if (error)
	{
		PrintError(AT, (const char*)error->GetBufferPointer());
		return false;
	}

	if (FAILED(hr))
	{
		PrintError(AT, "unidentified error occured while trying to create shader");
		return false;
	}

	hr = VCompile(_device, compiledShader);

	if (FAILED(hr))
	{
		PrintError(AT, "failed to create shader");
		return false;
	}

	SAFE_RELEASE(error);
	SAFE_RELEASE(compiledShader);

	printf("ok\n");
	return true;
}

/********************
--- Vertex Shader ---
*********************/

VertexShader::VertexShader() :	m_pVertexShader(nullptr),
								m_pInputLayout(nullptr)
{}
// : m_pVertexShader(nullptr)	{ }

VertexShader::~VertexShader()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
}

HRESULT VertexShader::VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader)
{
	HRESULT hr = S_OK;
	hr =_device->CreateVertexShader(_compiledShader->GetBufferPointer(), _compiledShader->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
		return hr;

	return CreateInputLayout(_device, _compiledShader, m_pInputLayout);
}

void VertexShader::VStage(ID3D11DeviceContext* _context)
{
	_context->VSSetShader(m_pVertexShader, NULL, 0);
	_context->IASetInputLayout(m_pInputLayout);
}


/*******************
--- Pixel Shader ---
********************/

HRESULT PixelShader::VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader)
{
	return _device->CreatePixelShader(_compiledShader->GetBufferPointer(), _compiledShader->GetBufferSize(), NULL, &m_pPixelShader);
}

void PixelShader::VStage(ID3D11DeviceContext* _context)
{
	_context->PSSetShader(m_pPixelShader, NULL, 0);
}

/**********************
--- Geometry Shader ---
***********************/

HRESULT GeometryShader::VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader)
{
	return _device->CreateGeometryShader(_compiledShader->GetBufferPointer(), _compiledShader->GetBufferSize(), NULL, &m_pGeometryShader);
}

void GeometryShader::VStage(ID3D11DeviceContext* _context)
{
	_context->GSSetShader(m_pGeometryShader, NULL, 0);
}

/*********************
--- Compute Shader ---
**********************/

HRESULT ComputeShader::VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader)
{
	return _device->CreateComputeShader(_compiledShader->GetBufferPointer(), _compiledShader->GetBufferSize(), NULL, &m_pComputeShader);
}

void ComputeShader::VStage(ID3D11DeviceContext* _context)
{
	_context->CSSetShader(m_pComputeShader, NULL, 0);
}