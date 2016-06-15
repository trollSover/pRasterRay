#include "Frustum.h"

using RasterRay::Frustum;
using RasterRay::OctreeConstantBuffer;

bool Frustum::GetSmallestEnclosingNode(const std::vector<VOXEL::TNode>& _nodeSource, const unsigned _nodePtr, const float _dim, const unsigned _depth, FVEC3 _origin, OctreeConstantBuffer& _scene)
{
	TNode node = _nodeSource[_nodePtr];

	unsigned visibleCorners = EnclosedNode(_origin.x, _origin.y, _origin.z, _dim);

	if (visibleCorners == 0) return false;	// we're outside the frustum


	if (visibleCorners == 2)	// we're entirely inside frustum
	{
		if (_dim > _scene.gridLength) return false;
		_scene.gridLength	= _dim;
		_scene.rootIndex	= _nodePtr;
		_scene.rootOrigin	= _origin;
		_scene.maxDepth		= m_svoHeader.numLevels - _depth;
		return true;
	}
	if (node.IsLeaf()) return false;	// we don't have any children

	if (visibleCorners == 1)	// we're partly inside frustum
	{
		static const FVEC3 offset[] =
		{
			{ 0, 0, 0 },
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 1, 1, 0 },
			{ 0, 0, 1 },
			{ 1, 0, 1 },
			{ 0, 1, 1 },
			{ 1, 1, 1 }
		};


		FVEC3 pos;

		for (unsigned i = 0; i < 8; ++i)
		{
			if (node.HasChildAtIndex(i))
			{
				pos = offset[i];
				pos *= (_dim * 0.5f);
				bool b = GetSmallestEnclosingNode(_nodeSource, node.GetChildAddress(i), _dim * 0.5f, _depth + 1, _origin + pos, _scene);
				//if (b) return true;
			}
		}
	}
}

OctreeConstantBuffer Frustum::SetRaycastScene(const std::vector<VOXEL::TNode>& _nodes, const SVO_Header _header)
{
	OctreeConstantBuffer ocb = { 0 };
	ocb.gridLength = _header.gridlength;
	m_svoHeader = _header;

	GetSmallestEnclosingNode(_nodes, m_svoHeader.numNodes - 1, m_svoHeader.gridlength, 0, FVEC3(0, 0, 0), ocb);

	return ocb;
}