#pragma once

#include <d3d11.h>
#include "Buffer.h"

HRESULT CreateInputLayout(ID3D11Device*& _device, ID3D10Blob* _shader, ID3D11InputLayout*& _layout);

HRESULT CreateConstantBuffer(ID3D11Device*& _device, ID3D11Buffer*& _cbuffer, unsigned int _byteSize);