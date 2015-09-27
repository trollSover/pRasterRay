#include "Buffer.h"
#include "D3DBufferUtility.h"

D3DBuffer::D3DBuffer() :	m_pResource(nullptr),
							//m_pBuffer(nullptr),
							m_pSRV(nullptr),
							m_pUAV(nullptr),
							m_nElements(0),
							m_sElement(0)
{

}

D3DBuffer::~D3DBuffer()
{
	Clear();
}

void D3DBuffer::Clear()
{
	SAFE_RELEASE(m_pUAV);
	SAFE_RELEASE(m_pSRV);
	SAFE_RELEASE(m_pResource);
}

HRESULT D3DBuffer::OnBufferType(ID3D11Device* _device, const BufferBind _bBind, size_t _nElements, uint32_t _byteWidth, void* _data)
{
	HRESULT hr = S_OK;

	switch (_bBind)
	{
		case BB_CONSTANT:
		{
			printf(" : constant buffer");
			D3D11_BUFFER_DESC dConst = GetConstBufferDesc(_byteWidth);

			hr = _device->CreateBuffer(&dConst, nullptr, (ID3D11Buffer**)(&m_pResource));

			if (FAILED(hr))
			{
				PrintError(AT, "failed to create constant buffer");
			}
		}
		break;
		default:
		{
			D3D11_BUFFER_DESC dDynamic = GetDynamicBufferDesc(_byteWidth, _nElements, _bBind);

			if (_data)
			{

				D3D11_SUBRESOURCE_DATA srd;
				ZeroMemory(&srd, sizeof(D3D11_SUBRESOURCE_DATA));
				srd.pSysMem = _data;
				srd.SysMemPitch = 0;
				srd.SysMemSlicePitch = 0;
				hr = _device->CreateBuffer(&dDynamic, &srd, (ID3D11Buffer**)(&m_pResource));
			}
			else
			{
				hr = _device->CreateBuffer(&dDynamic, nullptr, (ID3D11Buffer**)(&m_pResource));
			}

			if (FAILED(hr))
			{
				PrintError(AT, "Failed to initialize dynamic buffer");
				return hr;
			}
			
			if (_bBind == BB_UAV || _bBind == BB_SRV_UAV)
			{
				printf(" : UAV buffer");
				D3D11_UNORDERED_ACCESS_VIEW_DESC dUAV = GetDynamicUAVDesc(m_pResource);
				hr = _device->CreateUnorderedAccessView(m_pResource, &dUAV, &m_pUAV);

				if (FAILED(hr))
				{
					PrintError(AT, "Failed to initialize UAV");
					return hr;
				}
			}
			if (_bBind == BB_SRV || _bBind == BB_SRV_UAV)
			{
				printf(" : SRV buffer");
				D3D11_SHADER_RESOURCE_VIEW_DESC sdesc = GetDynamicSRVDesc(m_pResource);
				hr = _device->CreateShaderResourceView(m_pResource, &sdesc, &m_pSRV);
				
				if (FAILED(hr))
				{
					PrintError(AT, "Failed to initialize SRV");
					return hr;
				}			
			}
		}
		break;
	}
	return hr;
}

bool D3DBuffer::Init(ID3D11Device* _device, const BufferType _bType, const BufferBind _bBind, size_t _nElements, uint32_t _byteWidth, void* _data)
{
	printf("D3DBuffer");
	if (m_pResource || m_pSRV || m_pUAV ) //|| m_pBuffer)
	{
		PrintError(AT, "Buffer already initialized, use Clear() to release resources before reuse");
		return false;
	}

	HRESULT hr = S_OK;
	switch (_bType)
	{
		case BT_TEXTURE2D:
		{
			PrintError(AT, "texture resource loading not implemented yet");
			return false;
		}
		break;
		default:
		{
			hr = OnBufferType(_device, _bBind, _nElements, _byteWidth, _data);
		}
		break;
	}

	if (FAILED(hr))
	{
		_com_error err(hr);
		PrintError(AT, err.ErrorMessage());
	}
	else
	{
		m_nElements = _nElements;
		m_sElement	= _byteWidth;
	}
	printf(" ok\n");
	return true;
}