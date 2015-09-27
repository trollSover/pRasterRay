#pragma once

#include "../CoreSystems/CoreStd.h"
#include "ShaderInputFormats.h"
#include "Buffer.h"

class DXFont
{
private:
	struct FontType
	{
		float	left, right;
		int		height;
	};

	FontType*	m_paFont;
protected:
public:

private:
	bool LoadFont(const char* _fontFile);
	void BuildVertexArray(void* _vertices, const char* _text, float _dX, float _dY);
protected:
public:
	DXFont(void);
	~DXFont(void);
		
	bool Init(ID3D11Device* _device, const char* _fontFile);
	bool UpdateText(ID3D11Device* _device, const char* _text, int _x, int _y);
};