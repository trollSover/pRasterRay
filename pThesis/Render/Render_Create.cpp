#include "Render_Create.h"
#include "../CoreSystems/CoreStd.h"

#include <d3d11shadertracing.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>


HRESULT CreateInputLayout(ID3D11Device*& _device, ID3D10Blob* _shader, ID3D11InputLayout*& _layout)
{
	HRESULT hr = S_OK;

	ID3D11ShaderReflection* reflection;

	hr = D3DReflect(_shader->GetBufferPointer(), _shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
	if (FAILED(hr))
	{
		PrintError(AT, "failed to reflect shader layout");
		return hr;
	}

	D3D11_SHADER_DESC shaderDesc;
	hr = reflection->GetDesc(&shaderDesc);

	if (FAILED(hr))
	{
		reflection->Release();

		PrintError(AT, "failed to get reflected desc");
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	byte animSlot = 255;

	for (UINT32 i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		hr = reflection->GetInputParameterDesc(i, &paramDesc);
		if (FAILED(hr))
			MessageBox(NULL, "FAIL", "Shader Error", MB_OK);

		std::string semantic = paramDesc.SemanticName;
		if (animSlot == 255)
		{
			if (semantic == "PERINST")
				animSlot = 2;
			else if (semantic == "ANIM")
				animSlot = 1;
		}
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		if (semantic == "PERINST")
		{
			if (animSlot != 1)
				elementDesc.InputSlot = 1;
			else
				elementDesc.InputSlot = 2;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;
		}
		else if (semantic == "ANIM")
		{
			elementDesc.InputSlot = animSlot;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
		}
		else
		{
			elementDesc.InputSlot = 0;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
		}

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		inputLayoutDesc.push_back(elementDesc);
	}

	hr = _device->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), _shader->GetBufferPointer(), _shader->GetBufferSize(), &_layout);

	if (FAILED(hr))
	{
		PrintError(AT, "failed to create input layout from reflection");
	}

	reflection->Release();

	return hr;
}

HRESULT CreateConstantBuffer(ID3D11Device*& _device, ID3D11Buffer*& _cbuffer, unsigned int _byteSize)
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC dBuffer;
	ZeroMemory(&dBuffer, sizeof(D3D11_BUFFER_DESC));

	dBuffer.Usage			= D3D11_USAGE_DEFAULT;
	dBuffer.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	dBuffer.CPUAccessFlags	= 0;
	dBuffer.ByteWidth		= _byteSize;

	hr = _device->CreateBuffer(&dBuffer, NULL, &_cbuffer);

	if (FAILED(hr))
	{
		PrintError(AT, "failed to create constant buffer");
	}

	return hr;
}