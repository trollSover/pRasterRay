#pragma once

#include "../CoreSystems/CoreStd.h"
#include "../Application/Node.h"
#include "../Application/SVO_IO.h"
#include "ConstantBufferDefines.h"

namespace RasterRay
{
	class Frustum
	{
		/* Members */
	private:
		FVEC4 m_planes[6];
		FMAT4X4	m_wvpMatrix, m_viewMatrix;
		OctreeConstantBuffer	m_sceneCB;
		SVO_Header				m_svoHeader;
		/* Methods */
	private:
		unsigned EnclosedNode(const float, const float, const float, const float);
		bool IntersectNode(const float, const float, const float, const float);

		bool GetSmallestEnclosingNode(const std::vector<VOXEL::TNode>&, const unsigned, const float, const unsigned, FVEC3, OctreeConstantBuffer&);

	public:
		Frustum(void);
		~Frustum(void);

		void Build(const float, FMAT4X4, FMAT4X4);

		OctreeConstantBuffer SetRaycastScene(const std::vector<VOXEL::TNode>&, const SVO_Header);
		
	};
}