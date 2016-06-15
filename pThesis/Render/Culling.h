#pragma once

#include "../CoreSystems/CoreStd.h"
#include "../Application/Node.h"
#include "ShaderInputFormats.h"

using namespace VOXEL;

class FrustumCulling
{
private:
	FVEC4 m_aPlanes[6];

	FMAT4X4		VPMatrix, VMatrix;
	Resolution	depthRes;
	float*		depthMap;

protected:
public:
	FrustumCulling();
	~FrustumCulling();

	bool CheckPoint		(float _x, float _y, float _z);
	bool CheckCube		(float _x, float _y, float _z, float _dim);
	bool CheckSphere	(float _x, float _y, float _z, float _radius);
	bool CheckRectangle (float _x, float _y, float _z, float _xDim, float _yDim, float _zDim);

	void CheckVoxels(const std::vector<GPU_Node>& _nodes, uint32_t _nodePtr, std::vector<uint32_t>& _visibleNodes, FVEC3 _origin);
	void CheckVoxels(const std::vector<TNode>& _nodes, uint32_t _nodePtr, std::vector<uint32_t>& _visibleNodes, FVEC3 _origin, int _depth);
	void Construct(float _depth, FMAT4X4 _projectionMatrix, FMAT4X4 _viewMatrix);

	void GetVisibleNodes(const std::vector<TNode>& _nodeSource, uint32_t _rootPtr, std::vector<uint32_t>& _nodeDest, int depth, FVEC3 _geometryOrigin = FVEC3());

	unsigned CubeInFrustum(const float _x, const float _y, const float _z, const float _dim);
	void GetVisibleNodes(const std::vector<TNode>& _nodeSource, uint32_t _rootPtr, std::vector<BBPoint>& _nodeDest, int depth, FVEC3 _geometryOrigin = FVEC3());

	void GetVisibleBBIndices(const std::vector<TNode>& _nodeSource, uint32_t _rootPtr, std::vector<uint32_t>& _nodeDest, const float _dim, FVEC3 _origin);
};