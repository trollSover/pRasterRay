#include "Font.h"

#include <fstream>

DXFont::DXFont(void) : m_paFont(nullptr)
{ }

DXFont::~DXFont(void)
{
	SAFE_DELETE_ARRAY(m_paFont);
}

bool DXFont::LoadFont(const char* _fontFile)
{
	m_paFont = VNEW FontType[95];
	if (!m_paFont)
	{
		PrintError(AT, "failed to allocate font");
		return false;
	}

	FILE* in = nullptr;
	fopen_s(&in, _fontFile, "r");

	if (!in)
	{
		PrintError(AT, "failed to open: " + std::string(_fontFile));
		return false;
	}
	char cCode;
	char slask;
	int ascii;
	float left, right;
	int pSize;
	for (int i = 0; i < 95; ++i)
	{
		fscanf_s(in, "%i", &ascii);
		fscanf_s(in, "%c", &slask);
		fscanf_s(in, "%c", &cCode);
		fscanf_s(in, "%c", &slask);
		fscanf_s(in, "%f", &left);
		fscanf_s(in, "%f", &right);
		fscanf_s(in, "%i", &pSize);

		m_paFont[i].left	= left;
		m_paFont[i].right	= right;
		m_paFont[i].height	= pSize;
	}

	fclose(in);

	return true;
}

bool DXFont::Init(ID3D11Device* _device, const char* _fontFile)
{
	bool b = LoadFont(_fontFile);
	return false;
}

bool DXFont::UpdateText(ID3D11Device* _device, const char* _text, int _x, int _y)
{
	int numLetters;
	Vertex* vertices;
	float dx, dy;
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Vertex* verticesPtr;
	return false;
}

void DXFont::BuildVertexArray(void* _vertices, const char* _text, float _dX, float _dY)
{
	Vertex* vPtr;
	int N, index, letter;

	vPtr = (Vertex*)_vertices;
	N = (int)strlen(_text);
	index = 0;

	for (int i = 0; i < N; ++i)
	{
		letter = ((int)_text[i]) - 32;
		if (letter == 0)
		{
			_dX = _dX + 3.0f;
		}
		else
		{
			// First triangle in quad.
			vPtr[index].p	= FVEC3(_dX, _dY, 0.0f);  // Top left.
			vPtr[index].uv	= FVEC2(m_paFont[letter].left, 0.0f);
			index++;

			vPtr[index].p = FVEC3(_dX + m_paFont[letter].height, _dY - 16, 0.0f);  // Bottom right.
			vPtr[index].uv = FVEC2(m_paFont[letter].right, 1.0f);
			index++;

			vPtr[index].p = FVEC3(_dX, _dY - 16, 0.0f);  // Bottom left.
			vPtr[index].uv = FVEC2(m_paFont[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vPtr[index].p = FVEC3(_dX, _dY, 0.0f);  // Top left.
			vPtr[index].uv = FVEC2(m_paFont[letter].left, 0.0f);
			index++;

			vPtr[index].p = FVEC3(_dX + m_paFont[letter].height, _dY, 0.0f);  // Top right.
			vPtr[index].uv = FVEC2(m_paFont[letter].right, 0.0f);
			index++;

			vPtr[index].p = FVEC3(_dX + m_paFont[letter].height, _dY - 16, 0.0);  // Bottom right.
			vPtr[index].uv = FVEC2(m_paFont[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			_dX = _dX + m_paFont[letter].height + 1.0f;
		}

	}
}