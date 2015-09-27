#pragma once

/***************
--- INCLUDES ---
****************/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#define FDEBUG

/* External */

#include <Windows.h>
#include <comdef.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>

#include <memory>
#include <malloc.h>
#include <string>

#include <vector>
#include <map>
#include <inttypes.h>
#include <algorithm>

#include <time.h>

/* Internal */
#define DXVECTOR

#include "../Utils/Vector_Util.h"
#include "../Utils/Math_Util.h"

using namespace VECTOR;

/********************
--- EXTERNAL LIBS ---
*********************/

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX;

/**************
--- DEFINES ---
***************/

#define SAFE_RELEASE(x)			if( x ) { (x)->Release();	(x) = nullptr; }
#define SAFE_DELETE(x)			if( x ) { delete(x);		(x) = nullptr; }
#define SAFE_DELETE_ARRAY(x)	if( x ) { delete[](x);		(x) = nullptr; }

/***************
--- TYPEDEFS ---
****************/

typedef unsigned int uint;

typedef VECTOR::VEC2<float> FVEC2;
typedef VECTOR::VEC3<float> FVEC3;
typedef VECTOR::VEC4<float> FVEC4;
typedef VECTOR::VEC4<float> COLOR;

typedef VECTOR::VEC3<uint32_t>	U32VEC3;
typedef VECTOR::VEC3<int32_t>	I32VEC3;

typedef VECTOR::VEC2<float>		FPOINT;
typedef VECTOR::VEC2<int32_t>	I32POINT;

typedef VECTOR::MATRIX4X4<float> FMAT4X4;

/* Constant Buffer Data */
struct CBDCamera
{
	FMAT4X4	mWVP;			// 64
	FMAT4X4 mView;			// 64
	FMAT4X4 mProjection;	// 64
	FMAT4X4 mWorld;			// 64
	FVEC3	cameraPos;		// 12
	FVEC3	cameraDir;		// 12
	FVEC3	right;			// 12
	FVEC3	up;				// 12
};							// = 304

struct CBDVoxel
{
	uint32_t numNodes;		// total amount of nodes
	uint32_t gridLength;	// uniform length of cube
	uint32_t rootDepth;		// depth N at root level where: { 0, 1, ..., N }
	uint32_t rootIndex;		// root pointer for Node buffer
};

struct CBDWindow
{
	uint32_t width;
	uint32_t height;

	VEC2<uint32_t>	padding0;
};

struct CBDEval
{
	float		depthDivider;
	uint32_t	NLoD;

	VEC2<uint32_t>	padding0;
};

struct Intersection
{
	int id;
	float u, v, t;

	Intersection()
		: id(-1), t(0), u(0), v(0)
	{}
};

class Ray
{
private:
	FVEC3 m_origin;
	FVEC3 m_delta;
	FVEC4 m_color;
	int	  m_triangleId;

protected:
public:

private:
	Ray(const Ray&) = delete;
protected:
public:
	Ray()
		: m_triangleId(-1), m_origin(0, 0, 0), m_delta(0, 0, 0), m_color(0, 0, 0, 0)
	{}

	~Ray() {}
};

/*******************
--- COMMON TYPES ---
********************/
enum BufferType { BT_RAW, BT_STRUCTURED, BT_TEXTURE2D };
enum BufferBind { BB_SRV, BB_UAV, BB_SRV_UAV, BB_CONSTANT, BB_VERTEX, BB_INDEX };

struct Resolution
{
	uint width, height;
	Resolution(uint _w = 0, uint _h = 0)
		: width(_w), height(_h)
	{}
};

struct Time
{
	const float dtMS;
	const float totalS;

	Time(float _dt = 0, float _total = 0)
		: dtMS(_dt), totalS(_total)
	{}

	Time(const Time& other)
		: dtMS(other.dtMS), totalS(other.totalS)
	{}
};

/************
--- Debug ---
*************/

/* debug helper macros */
#define FORCE_LOG

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT "File: " __FILE__  "\nFunc: (" __FUNCTION__ ")\nLine: " TOSTRING(__LINE__)

#if defined (_DEBUG)
#define VNEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#define VNEW new
#endif

static void Log(const char* _msg)
{
	static bool LogCreated;
	FILE* file;

	if (!LogCreated)
	{
		fopen_s(&file, "log.txt", "w");
		time_t t = time(NULL);
		tm tstruct;
		char buf[80];
		localtime_s(&tstruct, &t);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X\n", &tstruct);
		fputs(buf, file);
		LogCreated = true;
	}
	else
		fopen_s(&file, "log.txt", "a");

	if (file == nullptr)
	{
		if (LogCreated)
			LogCreated = false;
		return;
	}
	else
	{
		fputs(_msg, file);
	}

	if (file)
		fclose(file);
}

static void PrintError(const char* _location, const std::string _msg)
{
	const char* no_path = strrchr(_location, '\\');

	// remove full path
	if (no_path)
		++no_path;
	else
		no_path = _location;


	printf("-----\nError! %s\n%s\n-----\n", _msg.c_str(), no_path);

#if defined (FORCE_LOG)
	Log(_msg.c_str());
#endif
}

static void PrintError(const char* _location, const HRESULT _hr)
{
	const char* no_path = strrchr(_location, '\\');
	_com_error err(_hr);
	LPCTSTR msg = err.ErrorMessage();

	// remove full path
	if (no_path)
		++no_path;
	else
		no_path = _location;


	printf("-----\nError! %s\n%s\n-----\n", msg, no_path);

#if defined (FORCE_LOG)
	Log(msg);
#endif
}



//	XMFLOAT4 operator=(const XMFLOAT4& d2) { this->d = d2; }
//
//	operator const XMFLOAT4(){ return this->d; }
//	operator const XMVECTOR(){ return XMLoadFloat4(&d); }

//
//	//XMFLOAT4X4 operator=(const XMFLOAT4X4& d2)	{ XMStoreFloat4x4(&this->d, XMLoadFloat4x4(&d2)); }
//	operator const XMFLOAT4*()	{ return (XMFLOAT4*)this; }
//	operator XMMATRIX()			{ return XMLoadFloat4x4(&d); }
//};
//
//struct Vertex
//{
//	VECTOR3 position;
//	VECTOR3 normal;
//	VECTOR2 uv;
//
//	Vertex(float size = 1.0f, VECTOR3 _p = VECTOR3(0, 0, 0), VECTOR3 _n = VECTOR3(0, 0, 0), VECTOR2 _uv = VECTOR2(0, 0))
//		: position(_p), normal(_n), uv(_uv)
//	{
//		position.d.x *= size;
//		position.d.y *= size;
//		position.d.z *= size;
//	}
//};