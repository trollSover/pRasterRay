#pragma once

#include <d3d11.h>
#include "../CoreSystems/CoreStd.h"

enum RT_FORMATSUPPORT
{
	RTFS_NONE				= 0,
	RTFS_TEXTURE			= 0x1,
	RTFS_SHADER_RESOURCE	= 0x2,
	RTFS_UNORDERED_ACCESS	= 0x4,
};

enum BOUND_FORMAT
{
	BF_NONE				= 0,
	BF_VIEW				= 0x1,
	BF_TEXTURE			= 0x2,
	BF_SHADER_RESOURCE	= 0x4,
	BF_UNORDERED_ACCESS	= 0x8
};

struct RenderTargetInit
{

	unsigned int						formats;
	D3D11_TEXTURE2D_DESC				TexDesc;
	D3D11_RENDER_TARGET_VIEW_DESC		RTVDesc;	
	D3D11_SHADER_RESOURCE_VIEW_DESC		SRVDesc;
	D3D11_UNORDERED_ACCESS_VIEW_DESC	UAVDesc;

	RenderTargetInit()
	{
		formats = RTFS_NONE;
		ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
		ZeroMemory(&RTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));		
		ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		ZeroMemory(&UAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	}
};


class RenderTarget
{
private:
	ID3D11RenderTargetView		*m_pView;
	ID3D11Texture2D				*m_pTexture;
	ID3D11ShaderResourceView	*m_pShaderResource;
	ID3D11UnorderedAccessView	*m_pUAV;

	const char					*m_pId;
	RT_FORMATSUPPORT			 m_formatSupport;
	BOUND_FORMAT				 m_boundFormat;

	friend class DXDriver;
protected:
public:

private:
protected:
public:
	void GetView(ID3D11RenderTargetView*& _renderTargetView);
	void GetTexture(ID3D11Texture2D*& _texture);
	void GetShaderResource(ID3D11ShaderResourceView*& _shaderResource);
	void GetUAV(ID3D11UnorderedAccessView*& _uav);

	bool FormatSupportFor(RT_FORMATSUPPORT _fs) { return 0 != (_fs & m_formatSupport); }

	RenderTarget(const char* _id);
	~RenderTarget();

	bool Init(const RenderTargetInit& _rtInit, ID3D11Device* _device);
	void ClearView(ID3D11DeviceContext* _context, const FVEC4 _color);

	const char* GetId() { return m_pId; }
};