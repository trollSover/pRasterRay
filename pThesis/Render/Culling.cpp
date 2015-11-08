#include "Culling.h"

#include "../CoreSystems/IApplication.h"

extern IApplication* g_pApplication;

FrustumCulling::FrustumCulling() : depthMap(nullptr)
{
	assert(g_pApplication);
	
	depthRes = g_pApplication->VGetResolution();
	depthMap = new float[depthRes.width * depthRes.height];
	//memset(depthMap, -1.0f, (depthRes.height * depthRes.width) * sizeof(*depthMap));
	size_t size;
	std::tie(depthMap, size) = std::get_temporary_buffer<float>(depthRes.width * depthRes.height);
	std::uninitialized_fill(depthMap, depthMap + size, 10000.0f);
	assert(depthMap);
}

FrustumCulling::~FrustumCulling()
{
	SAFE_DELETE_ARRAY(depthMap);
}

void FrustumCulling::Construct(const float _depth, FMAT4X4 _projectionMatrix, FMAT4X4 _viewMatrix)
{
	float zMin, r;
	FMAT4X4 matrix;
	VPMatrix = TMatrixIdentity<float>() * _viewMatrix * _projectionMatrix;
	VMatrix = TMatrixIdentity<float>() * _viewMatrix;
	zMin = -_projectionMatrix.m[3][2] / _projectionMatrix.m[2][2];		// change for different starting depths
	r = _depth / (_depth - zMin);
	_projectionMatrix.m[2][2] = r;
	_projectionMatrix.m[3][2] = -r * zMin;
	
	matrix = _viewMatrix * _projectionMatrix;
	//VPMatrix = matrix;
	// near plane
	m_aPlanes[0].x = matrix.m[0][3] + matrix.m[0][2];
	m_aPlanes[0].y = matrix.m[1][3] + matrix.m[1][2];
	m_aPlanes[0].z = matrix.m[2][3] + matrix.m[2][2];
	m_aPlanes[0].w = matrix.m[3][3] + matrix.m[3][2];
	m_aPlanes[0]   = Normalize(m_aPlanes[0]);

	// far plane
	m_aPlanes[1].x = matrix.m[0][3] - matrix.m[0][2];
	m_aPlanes[1].y = matrix.m[1][3] - matrix.m[1][2];
	m_aPlanes[1].z = matrix.m[2][3] - matrix.m[2][2];
	m_aPlanes[1].w = matrix.m[3][3] - matrix.m[3][2];
	m_aPlanes[1]   = Normalize(m_aPlanes[1]);

	// left plane
	m_aPlanes[2].x = matrix.m[0][3] + matrix.m[0][0];
	m_aPlanes[2].y = matrix.m[1][3] + matrix.m[1][0];
	m_aPlanes[2].z = matrix.m[2][3] + matrix.m[2][0];
	m_aPlanes[2].w = matrix.m[3][3] + matrix.m[3][0];
	m_aPlanes[2]   = Normalize(m_aPlanes[2]);

	// right plane
	m_aPlanes[3].x = matrix.m[0][3] - matrix.m[0][0];
	m_aPlanes[3].y = matrix.m[1][3] - matrix.m[1][0];
	m_aPlanes[3].z = matrix.m[2][3] - matrix.m[2][0];
	m_aPlanes[3].w = matrix.m[3][3] - matrix.m[3][0];
	m_aPlanes[3]   = Normalize(m_aPlanes[3]);

	// top
	m_aPlanes[4].x = matrix.m[0][3] - matrix.m[0][1];
	m_aPlanes[4].y = matrix.m[1][3] - matrix.m[1][1];
	m_aPlanes[4].z = matrix.m[2][3] - matrix.m[2][1];
	m_aPlanes[4].w = matrix.m[3][3] - matrix.m[3][1];
	m_aPlanes[4]   = Normalize(m_aPlanes[4]);

	// bottom
	m_aPlanes[5].x = matrix.m[0][3] + matrix.m[0][1];
	m_aPlanes[5].y = matrix.m[1][3] + matrix.m[1][1];
	m_aPlanes[5].z = matrix.m[2][3] + matrix.m[2][1];
	m_aPlanes[5].w = matrix.m[3][3] + matrix.m[3][1];
	m_aPlanes[5]   = Normalize(m_aPlanes[5]);

	//depthRes = g_pApplication->VGetResolution();
	//size_t size;
	//std::tie(depthMap, size) = std::get_temporary_buffer<float>(depthRes.width * depthRes.height);
	//std::uninitialized_fill(depthMap, depthMap + size, -100000.0f);
}

bool FrustumCulling::CheckPoint(float _x, float _y, float _z)
{
	for (int i = 0; i < 6; ++i)
	{
		if (PlaneDotCoord(FVEC3(_x, _y, _z), m_aPlanes[i]) < 0.0f)
			return false;
	}
	return true;
}

bool FrustumCulling::CheckCube(float _x, float _y, float _z, float _dim)
{
	for (int i = 0; i < 6; ++i)
	{
		if (PlaneDotCoord(FVEC3(_x - _dim, _y - _dim, _z - _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y - _dim, _z - _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y + _dim, _z - _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z - _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y - _dim, _z + _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y - _dim, _z + _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y + _dim, _z + _dim), m_aPlanes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z + _dim), m_aPlanes[i]) >= 0)
			continue;
		return false;
	}
	return true;
}

bool FrustumCulling::CheckSphere(float _x, float _y, float _z, float _radius)
{
	for (int i = 0; i < 6; ++i)
	{
		if (PlaneDotCoord(FVEC3(_x, _y, _z), m_aPlanes[i]) < -_radius)
			return false;
	}
	return true;
}

bool FrustumCulling::CheckRectangle(float _x, float _y, float _z, float _xDim, float _yDim, float _zDim)
{
	for (int i = 0; i < 6; ++i)
	{
		if (PlaneDotCoord(FVEC3(_x - _xDim, _y - _yDim, _z - _zDim), m_aPlanes[i]) >= 0.0f)
			continue;							 			
		if (PlaneDotCoord(FVEC3(_x + _xDim, _y - _yDim, _z - _zDim), m_aPlanes[i]) >= 0.0f)
			continue;							 			
		if (PlaneDotCoord(FVEC3(_x - _xDim, _y + _yDim, _z - _zDim), m_aPlanes[i]) >= 0.0f)
			continue;							 			 
		if (PlaneDotCoord(FVEC3(_x - _xDim, _y - _yDim, _z + _zDim), m_aPlanes[i]) >= 0.0f)
			continue;				 						
		if (PlaneDotCoord(FVEC3(_x + _xDim, _y + _yDim, _z - _zDim), m_aPlanes[i]) >= 0.0f)
			continue;							 			 
		if (PlaneDotCoord(FVEC3(_x + _xDim, _y - _yDim, _z + _zDim), m_aPlanes[i]) >= 0.0f)
			continue;				  			 			 
		if (PlaneDotCoord(FVEC3(_x - _xDim, _y + _yDim, _z + _zDim), m_aPlanes[i]) >= 0.0f)
			continue;				 			 			  
		if (PlaneDotCoord(FVEC3(_x + _xDim, _y + _yDim, _z + _zDim), m_aPlanes[i]) >= 0.0f)
			continue;
		return false;
	}
	return true;
}

/* By morton Order */
static const FVEC3 offset[] = {	FVEC3(-0.5,	-0.5,	-0.5),
								FVEC3( 0.5,	-0.5,	-0.5),
								FVEC3(-0.5,	 0.5,	-0.5),
								FVEC3( 0.5,	 0.5,	-0.5),
								FVEC3(-0.5,	-0.5,	 0.5),
								FVEC3( 0.5,	-0.5,	 0.5),
								FVEC3(-0.5,	 0.5,	 0.5),
								FVEC3( 0.5,	 0.5,	 0.5)
};

inline float convertPixelsToClipSpace(const int pixelDimension, const int pixels)
{
	return (float)pixels / pixelDimension * 2 - 1;
}

inline float convertPixelsToClipSpaceDistance(const int pixelDimension, const int pixels)
{
	return (float)pixels / pixelDimension * 2;
}

void FrustumCulling::CheckVoxels(const std::vector<GPU_Node>& _nodes, uint32_t _nodePtr, std::vector<uint32_t>& _visibleNodes, FVEC3 _origin)
{
	GPU_Node node = _nodes[_nodePtr];
	uint32_t dim = std::pow(2, node.level);

	if (!CheckCube(_origin.x, _origin.y, _origin.z, dim))
		return;

	if (node.IsLeaf())	// leaf
	{
		if (node.HasData())
			_visibleNodes.push_back(_nodePtr);
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			int index = MortonOrder[i];

			if (node.HasChild(index))
			{
				FVEC3 nodepos = offset[index];
				nodepos *= (dim * 0.5);

				CheckVoxels(_nodes, node.GetChildAddress(index), _visibleNodes, _origin + nodepos);
			}
		}
	}
}

void FrustumCulling::CheckVoxels(const std::vector<TNode>& _nodes, uint32_t _nodePtr, std::vector<uint32_t>& _visibleNodes, FVEC3 _origin)
{
	TNode node = _nodes[_nodePtr];
	uint32_t dim = std::pow(2, 42);

	if (!CheckCube(_origin.x, _origin.y, _origin.z, dim))
		return;

	if (node.IsLeaf())	// leaf
	{
		if (!node.IsNull())
			_visibleNodes.push_back(_nodePtr);
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			int index = MortonOrder[i];

			if (node.HasChildAtIndex(index))
			{
				FVEC3 nodepos = offset[index];
				nodepos *= ((float)dim / 2.0f);

				CheckVoxels(_nodes, node.GetChildAddress(index), _visibleNodes, _origin + nodepos);
			}
		}
	}
}