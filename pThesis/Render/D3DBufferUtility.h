#pragma once

#include "../CoreSystems/CoreStd.h"

D3D11_TEXTURE2D_DESC GetTextureDesc(uint nElements, uint32_t _width, uint32_t _height)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width				= _width;
	desc.Height				= _height;
	desc.MipLevels			= 1;
	desc.ArraySize			= nElements;
	desc.SampleDesc.Count	= 1;
	desc.SampleDesc.Quality = 0;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags		= 0;
	desc.MiscFlags			= 0;

	return desc;
}

D3D11_SHADER_RESOURCE_VIEW_DESC GetSRVDesc(size_t nElements)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	desc.ViewDimension					= D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	desc.Format							= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Texture2DArray.MostDetailedMip = 0;
	desc.Texture2DArray.MipLevels		= 1;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize		= nElements;

	return desc;
}

D3D11_BUFFER_DESC GetConstBufferDesc(uint32_t sInBytes)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.Usage					= D3D11_USAGE_DYNAMIC;
	desc.ByteWidth				= sInBytes;
	desc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags				= 0;
	desc.StructureByteStride	= 0;

	return desc;

	//HRESULT hr = S_OK;
	//D3D11_BUFFER_DESC dBuffer;
	//ZeroMemory(&dBuffer, sizeof(D3D11_BUFFER_DESC));

	//dBuffer.Usage			= D3D11_USAGE_DEFAULT;
	//dBuffer.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	//dBuffer.CPUAccessFlags	= 0;
	//dBuffer.ByteWidth		= sInBytes;

	//return dBuffer;
}

D3D11_BUFFER_DESC GetDynamicBufferDesc(uint32_t sInBytes, size_t nElements, BufferBind bBind)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.ByteWidth				= sInBytes * nElements;
	desc.MiscFlags				= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride	= sInBytes;

	switch (bBind)
	{
	case BB_SRV_UAV:
		desc.Usage		= D3D11_USAGE_DEFAULT;
		desc.BindFlags	= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		break;
	case BB_UAV:
		desc.BindFlags	= D3D11_BIND_UNORDERED_ACCESS;
		break;
	case BB_SRV:
		desc.BindFlags	= D3D11_BIND_SHADER_RESOURCE;
		break;
	case BB_VERTEX:
		desc.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		desc.Usage			= D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags		= 0;
		break;
	case BB_INDEX:
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		break;
	default: break;	// watch me ignore your error
	}

	return desc;
}

D3D11_SHADER_RESOURCE_VIEW_DESC GetDynamicSRVDesc(ID3D11Resource* resource)
{
	D3D11_BUFFER_DESC bdesc;
	ZeroMemory(&bdesc, sizeof(D3D11_BUFFER_DESC));

	((ID3D11Buffer*)resource)->GetDesc(&bdesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC udesc;
	ZeroMemory(&udesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	if (bdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		udesc.Format				= DXGI_FORMAT_R32_TYPELESS;
		udesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFEREX;
		udesc.BufferEx.FirstElement = 0;
		udesc.BufferEx.Flags		= D3D11_BUFFEREX_SRV_FLAG_RAW;
		udesc.BufferEx.NumElements	= bdesc.ByteWidth / 4;
	}
	else if (bdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		udesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFER;
		udesc.Format				= DXGI_FORMAT_UNKNOWN;
		udesc.Buffer.NumElements	= bdesc.ByteWidth / bdesc.StructureByteStride;
		udesc.Buffer.FirstElement	= 0;
	}

	return udesc;
}

D3D11_UNORDERED_ACCESS_VIEW_DESC GetDynamicUAVDesc(ID3D11Resource* resource)
{
	D3D11_BUFFER_DESC bdesc;
	ZeroMemory(&bdesc, sizeof(D3D11_BUFFER_DESC));
	((ID3D11Buffer*)resource)->GetDesc(&bdesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC udesc;
	ZeroMemory(&udesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	udesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
	udesc.Buffer.FirstElement	= 0;

	if (bdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer
		udesc.Format				= DXGI_FORMAT_R32_TYPELESS;
		udesc.Buffer.Flags			= D3D11_BUFFER_UAV_FLAG_RAW;
		udesc.Buffer.NumElements	= bdesc.ByteWidth / 4;
	}
	else if (bdesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		// This is a Structured Buffer
		udesc.Format				= DXGI_FORMAT_UNKNOWN;
		udesc.Buffer.NumElements	= bdesc.ByteWidth / bdesc.StructureByteStride;
	}

	return udesc;
}