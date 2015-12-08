#pragma once

#include "../CoreSystems/CoreStd.h"
#include <d3d11.h>

class Shader
{
private:
protected:
public:

private:
protected:
	virtual HRESULT VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader) { return false; };
public:
	virtual ~Shader() {}

	bool Init(LPCWSTR _file, ID3D11Device* _device, D3D_SHADER_MACRO* = nullptr);
	virtual void VStage(ID3D11DeviceContext* _context) = 0;
	virtual void VClose() = 0;
	virtual const LPCSTR VType() const = 0;
};

class VertexShader
	: public Shader
{
private:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout*	m_pInputLayout;
protected:
public:

private:
	HRESULT VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader) final;
	const LPCSTR VType() const final { return "vs_5_0"; }
protected:
public:
	VertexShader();

	~VertexShader();

	void VStage(ID3D11DeviceContext* _context)  final;

	void VClose() final { return; }
};

class PixelShader
	: public Shader
{
private:
	ID3D11PixelShader* m_pPixelShader;
protected:
public:

private:
	HRESULT VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader) final;
	const LPCSTR VType() const final { return "ps_5_0"; }
protected:
public:
	PixelShader() : m_pPixelShader(nullptr)	{ }

	~PixelShader()	{ SAFE_RELEASE(m_pPixelShader); }

	void VStage(ID3D11DeviceContext* _context)  final;

	void VClose() final	{ return; }
};

class GeometryShader
	: public Shader
{
private:
	ID3D11GeometryShader* m_pGeometryShader;
protected:
public:

private:
	HRESULT VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader) final;
	const LPCSTR VType() const final { return "gs_5_0"; }
protected:
public:
	GeometryShader() : m_pGeometryShader(nullptr)	{ }

	~GeometryShader()	{ SAFE_RELEASE(m_pGeometryShader); }

	void VStage(ID3D11DeviceContext* _context)  final;

	void VClose() final	{ return; }
};

class ComputeShader
	: public Shader
{
private:
	ID3D11ComputeShader* m_pComputeShader;
protected:
public:

private:
	HRESULT VCompile(ID3D11Device* _device, ID3D10Blob*& _compiledShader) final;
	const LPCSTR VType() const final { return "cs_5_0"; }
protected:
public:
	ComputeShader() : m_pComputeShader(nullptr)	{ }

	~ComputeShader()	{ SAFE_RELEASE(m_pComputeShader); }

	void VStage(ID3D11DeviceContext* _context)  final;

	void VClose() final	{ return; }
};