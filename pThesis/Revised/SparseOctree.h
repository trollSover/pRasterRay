#pragma once


#include "../Application/Node.h"
#include "../Application/VoxelData.h"
#include "../Application/SVO_IO.h"
#include "../Render/ShaderInputFormats.h"

namespace RasterRay
{
	class SparseOctree
	{
		/* Members */
	private:
		SVO_Header					m_header;
		unsigned					m_rootPtr;
		std::vector<VOXEL::TNode>	m_nodes;
		std::vector<VOXEL::Voxel>	m_voxels;
		std::vector<unsigned>		m_nodesPerDepth;

		std::vector<VOXEL::TNode> gridNodes;

		//std::vector<std::vector<VOXEL::TNode>>	m_lodNodes;
		//std::vector<std::vector<VOXEL::Voxel>>	m_lodVoxels;

		struct MeshNode
		{
			unsigned nodeptr;
			std::vector<MeshNode> subMeshes;
			std::vector<Vertex> vertices;
		};

		std::vector<MeshNode> m_meshNodes;

		std::vector<std::vector<Vertex>> m_lodMesh;
		std::vector<unsigned>	m_verticesPerDepth;

		std::vector<std::vector<Vertex>>	m_mesh;

		std::vector<std::vector<unsigned>>	m_nodeIndices;
	public:
		std::vector<std::vector<FVEC3>> m_boundingGrid;
		std::vector<std::vector<unsigned>> m_boundingGridIndices;
		/* Methods */
	private:
		void BuildMesh(const unsigned, const unsigned, const unsigned, FVEC3);

		void IndexMesh(void);

		float ApproximateSurface(FVEC3, const float);
		void Traverse(unsigned _node, unsigned _depth, unsigned _dim, FVEC3 _origin, std::vector<VOXEL::TNode> _nodes);
	public:
		std::vector<Vertex> GetLodVertices(unsigned _lodIndex)	{ return m_lodMesh[_lodIndex]; }
		unsigned GetVertexCountAtLodDepth(unsigned _lodIndex)	{ return m_verticesPerDepth[_lodIndex]; }

		bool LoadFile(const std::string&);
		void BuildMesh(const unsigned);
		void DualContour(void);
		~SparseOctree(void);

		SVO_Header GetHeader(void) const { return m_header; }

		std::vector<VOXEL::TNode> GetNodes(void) const { return m_nodes; }
		std::vector<VOXEL::Voxel> GetVoxels(void) const { return m_voxels; }
	};
}