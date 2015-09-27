#pragma once

#include "../CoreSystems/CoreStd.h"




class D3DBuffer
{
private:
	ID3D11Resource*				m_pResource;
	//ID3D11Buffer*				m_pBuffer;
	ID3D11UnorderedAccessView*	m_pUAV;
	ID3D11ShaderResourceView*	m_pSRV;

	uint32_t	m_nElements;
	uint32_t	m_sElement;

protected:
public:

private:
	D3DBuffer(const D3DBuffer&) = delete;

	HRESULT OnBufferType(ID3D11Device* _device, const BufferBind _bBind, size_t _nElements, uint32_t _byteWidth, void* _data = nullptr);
protected:
public:
	uint32_t	GetNElements()	const { return m_nElements; }
	uint32_t	GetSElement()	const { return m_sElement; }
	uint32_t	GetByteWidth()	const { return m_nElements * m_sElement; }

	D3DBuffer();
	~D3DBuffer();

	/* Initialize ID3D11Buffer 
		#_device	: ID3D11Device
		#_bType		: type of buffer to be created
		#_bBind		: bindings to be used on the gpu
		#_nElements : number of elements in buffer
		#_byteWidth	: size of one element
		#_data		: data to be initialized on creation (optional) 
	return:	true - on success
			false if else */
	bool Init(ID3D11Device* _device, const BufferType _bType, const BufferBind _bBind, size_t _nElements, uint32_t _byteWidth, void* _data = nullptr);

	/* Null all internal resources */
	void Clear();

	/* Get raw resource pointer */
	ID3D11Resource*				GetResource()		{ return m_pResource; }
	ID3D11Resource**			GetResourceRef()	{ return &m_pResource; }
	/* Get raw resource pointer */
	//ID3D11Resource*&			GetResourceRef()	{ return m_pResource; }

	/* Get unordered access view pointer */
	ID3D11UnorderedAccessView*	GetUAV()		{ return m_pUAV; }

	/* Get shader resource view pointer */
	ID3D11ShaderResourceView*	GetSRV()		{ return m_pSRV; }
	ID3D11ShaderResourceView**	GetSRVRef()		{ return &m_pSRV; }
};