#pragma once

/***************
--- INCLUDES ---
****************/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#define FDEBUG
//#define _PERFTRACK

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

#ifdef _WIN64
#pragma comment(lib, "lib/FW1FontWrapper_1_1/x64/FW1FontWrapper.lib")
#elif _WIN32
#pragma comment(lib, "FW1FontWrapper_1_1/x86/FW1FontWrapper.lib")
#endif

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

	FMAT4X4 mViewInverse;			// 64
	FMAT4X4 mProjectionInverse;		// 64
	FMAT4X4 mWVPInverse;			// 64
	FMAT4X4 mRotation;				// 64

	FVEC3	cameraPos;		// 12
	float a;
	FVEC3	cameraDir;		// 12
	float aa;
	FVEC3	right;			// 12
	float aaa;
	FVEC3	up;				// 12
	float aaaa;
};							// = 560	

struct CBDVoxel
{
	uint32_t numNodes;		// total amount of nodes
	int32_t gridLength;		// uniform length of cube
	int32_t rootDepth;		// depth N at root level where: { 0, 1, ..., N }
	int32_t rootIndex;		// root pointer for Node buffer
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

struct CBOctreeMatrices
{
	FMAT4X4 viewportToCamera;		// 64
	FMAT4X4 cameraToOctree;			// 64
	//FMAT4X4 octreeToWorld;			// 64
	//
	//FMAT4X4 worldToOctree;			// 64
	//FMAT4X4 OctreeToWorld;			// 64
	//
	//FMAT4X4	octreeToViewPort;		// 64
	//FMAT4X4	viewportToOctree;		// 64

	//FVEC3	cameraPosition;			// 12
	float	pixelInOctree;			// 4
	FVEC3	padding;
};									// = 464 / 16 = 29

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

#include "../Revised/Log.h"

static void CreateAppLog()
{
	FILE* file;

	fopen_s(&file, "log.txt", "w");

	if (file)
	{
		time_t t = time(NULL);
		tm tstruct;
		char buf[80];
		localtime_s(&tstruct, &t);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X\n", &tstruct);
		fputs(buf, file);
		fclose(file);
	}

}

static void Log(const char* _msg)
{
	FILE* file;

	fopen_s(&file, "log.txt", "a");

	if (file == nullptr)
	{
		return;
	}

	fputs(_msg, file);

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

	Verdandi::Debug::Log(Verdandi::Debug::LogNotification::Custom) << _msg;

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

	Verdandi::Debug::Log(Verdandi::Debug::LogNotification::Custom) << std::string(msg);

#if defined (FORCE_LOG)
	Log(msg);
#endif
}

//#include <WinUser.h>

/* User Input Defines */
// Camera Input
#define CAMERA_UP				VK_CONTROL
#define CAMERA_DOWN				VK_SPACE
#define CAMERA_FORWARD			'W'
#define CAMERA_BACK				'S'
#define CAMERA_LEFT				'A'
#define CAMERA_RIGHT			'D'
#define CAMERA_ROTATE_LEFT		'Q'
#define CAMERA_ROTATE_RIGHT		'E'
#define CAMERA_ROTATE_UP		'R'
#define CAMERA_ROTATE_DOWN		'F'
#define CAMERA_SPEED_INCREMENT	VK_ADD
#define CAMERA_SPEED_DECREMENT	VK_SUBTRACT
#define CAMERA_SPEED_RESET		VK_MULTIPLY
#define CAMERA_RESET_ALL		VK_HOME
#define CAMERA_LOCK				'N'
#define CAMERA_YAW_LEFT			'T'
#define CAMERA_YAW_RIGHT		'Y'
#define CAMERA_ROLL_LEFT		'G'
#define CAMERA_ROLL_RIGHT		'H'
#define CAMERA_PITCH_LEFT		'B'
#define CAMERA_PITCH_RIGHT		'N'

// General UI Input
#define UI_HELP					'P'

// Draw State Input
#define TOGGLE_RASTERIZE		'1'
#define TOGGLE_RAYCAST			'2'
#define TOGGLE_DEBUG			'3'
#define TOGGLE_CULL_MODE		VK_F1
#define TOGGLE_FILL_MODE		VK_F2
#define RASTERIZE_LOD_INCREMENT	VK_F5
#define RASTERIZE_LOD_DECREMENT VK_F6
#define RELOAD_SHADER			VK_F8


/* General Limit Defines */
#define CAMERA_SPEED_MAX		10
#define CAMERA_SPEED_MIN		0.01