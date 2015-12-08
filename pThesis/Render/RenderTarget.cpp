#include "RenderTarget.h"
#include "../CoreSystems/CoreStd.h"

RenderTarget::RenderTarget(const char* _id) :	m_pTexture(nullptr), 
												m_pView(nullptr), 
												m_pShaderResource(nullptr), 
												m_pUAV(nullptr), 
												m_formatSupport(RTFS_NONE),
												m_boundFormat(BF_NONE),
												m_pId(_id) 												
{ }

RenderTarget::~RenderTarget()
{
	//SAFE_DELETE(m_pId);
	SAFE_RELEASE(m_pUAV);
	SAFE_RELEASE(m_pShaderResource);
	SAFE_RELEASE(m_pView);
	SAFE_RELEASE(m_pTexture);
}

bool RenderTarget::Init(const RenderTargetInit& _rtInit, ID3D11Device* _device)
{
	HRESULT hr		= S_OK;
	m_formatSupport = (RT_FORMATSUPPORT)(_rtInit.formats);

	// we might already have an existing texture we want to bind
	// so check if the ptr is already assigned
	if (nullptr == m_pTexture)
	{
		// create rendertarget texture
		hr = _device->CreateTexture2D(&_rtInit.TexDesc, NULL, &m_pTexture);

		if (FAILED(hr))
		{
			PrintError(AT, hr);
			return false;
		}
	}
	else
	{
		PrintError(AT, "texture already assigned!\n");
	}

	// create rendertarget view
	hr = _device->CreateRenderTargetView(m_pTexture, &_rtInit.RTVDesc, &m_pView);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return false;
	}

	if (m_formatSupport & RTFS_SHADER_RESOURCE)
	{
		// make rendertarget texture accessible as a shader resource
		hr = _device->CreateShaderResourceView(m_pTexture, &_rtInit.SRVDesc, &m_pShaderResource);

		if (FAILED(hr))
		{
			PrintError(AT, hr);
			return false;
		}
	}

	if (m_formatSupport & RTFS_UNORDERED_ACCESS)
	{
		// make rendertarget texture accessible as an unordered access view (computeshader access)
		hr = _device->CreateUnorderedAccessView(m_pTexture, &_rtInit.UAVDesc, &m_pUAV);

		if (FAILED(hr))
		{
			PrintError(AT, hr);
			return false;
		}
	}

	if (false == (m_formatSupport & RTFS_TEXTURE))
	{
		// release texture ptr since no texture support defined
		SAFE_RELEASE(m_pTexture);
	}

	return true;
}

void RenderTarget::GetView(ID3D11RenderTargetView*& _renderTarget)
{ 
	m_boundFormat = (BOUND_FORMAT)(m_boundFormat | BF_VIEW);
	_renderTarget = m_pView; 
}

void RenderTarget::GetTexture(ID3D11Texture2D*& _texture)
{ 
	m_boundFormat = (BOUND_FORMAT)(m_boundFormat | BF_TEXTURE);
	_texture = m_pTexture; 
}

void RenderTarget::GetShaderResource(ID3D11ShaderResourceView*& _shaderResource)
{
	m_boundFormat = (BOUND_FORMAT)(m_boundFormat | BF_SHADER_RESOURCE);
	_shaderResource = m_pShaderResource;
}

void RenderTarget::GetUAV(ID3D11UnorderedAccessView*& _uav)
{
	m_boundFormat = (BOUND_FORMAT)(m_boundFormat | BF_UNORDERED_ACCESS);
	_uav = m_pUAV;
}

void RenderTarget::ClearView(ID3D11DeviceContext* _context, const FVEC4 _color)
{
	const float color[4] = { _color.x, _color.y, _color.z, _color.w };

	_context->ClearRenderTargetView(m_pView, color);
}