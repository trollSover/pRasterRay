#include "SparseOctree.h"
#include "../Application/MarchingCubes.h"

using namespace RasterRay;

SparseOctree::~SparseOctree(void)
{
	m_nodes.clear();
	m_voxels.clear();
}

bool SparseOctree::LoadFile(const std::string& _filename)
{
	bool res = true;

	res = FileExist(_filename + ".oct");
	if (!res)
	{
		PrintError(AT, _filename + " not found!");
		return false;
	}

	res = ReadHeader(_filename, m_header);
	if (!res)
	{
		PrintError(AT, _filename + " header not found!");
		return false;
	}

	m_rootPtr = m_header.numNodes - 1;

	FILE* pStream = nullptr;
	fopen_s(&pStream, std::string(m_header.filename + ".doct").c_str(), "rb");

	if (!pStream)
	{
		PrintError(AT, m_header.filename + ".doct not found!");
		return false;
	}

	int voxelByteSize = sizeof(VoxelData<NC>);
	int dataByteSize = sizeof(NC);

	m_voxels.resize(m_header.numData);

	VoxelData<NC> voxel;

	for (size_t i = 0; i < m_header.numData; ++i)
	{
		ReadData(pStream, voxel);
		m_voxels[i].color	= voxel.m_data.c;
		m_voxels[i].normal	= voxel.m_data.n;	
	}

	fclose(pStream);

	fopen_s(&pStream, std::string(m_header.filename + ".noct").c_str(), "rb");

	if (!pStream)
	{
		PrintError(AT, m_header.filename + ".noct not found!");
		return false;
	}

	m_nodes.resize(m_header.numNodes);
	m_nodesPerDepth.resize(m_header.numLevels + 1);

	Node<NC> node;
	unsigned childMask;

	for (size_t i = 0; i < m_header.numNodes; ++i)
	{
		ReadNode(pStream, node);

		m_nodes[i] = (node);

		assert(node.level >= 0 && node.level <= m_header.numLevels);
		m_nodesPerDepth[node.level]++;
	}

	m_lodMesh.resize(m_header.numLevels);
	m_boundingGrid.resize(m_header.numLevels+1);
	m_boundingGridIndices.resize(m_header.numLevels+1);
	BuildMesh(0, m_header.gridlength, m_rootPtr, FVEC3(0,0,0));
	DualContour();
	for (std::vector<Vertex>& subMesh : m_lodMesh)
	{
		m_verticesPerDepth.push_back(subMesh.size());
	}

	return true;
}

static TNode parent;

void SparseOctree::BuildMesh(const unsigned _lodLevels)
{
	const unsigned levels = m_header.numLevels + 1;
	m_mesh.resize(levels);
	
	BuildMesh(0, m_header.gridlength, m_rootPtr, FVEC3());

	// construct node indices
	m_nodeIndices.resize(levels);
	for (unsigned i = 0; i < levels; ++i)
	{
		m_nodeIndices[i].resize(m_mesh[i].size());

	}

	parent = m_nodes[m_rootPtr];
	signed n = parent.CountChildren();

	int a = 42;
}

void SparseOctree::IndexMesh(void)
{

}

void SparseOctree::BuildMesh(const unsigned _depth, const unsigned _dim, const unsigned _ptr, FVEC3 _origin)
{
	static FVEC3 offset[8] =
	{
		FVEC3(0, 0, 0),
		FVEC3(1, 0, 0),
		FVEC3(0, 1, 0),
		FVEC3(1, 1, 0),
		FVEC3(0, 0, 1),
		FVEC3(1, 0, 1),
		FVEC3(0, 1, 1),
		FVEC3(1, 1, 1),
	};

	TNode node = m_nodes[_ptr];
	VOXEL::Voxel voxel = m_voxels[node.GetDataAddress()];

	float mod = 1;

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[0] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[1] * _dim * mod));

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[0] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[2] * _dim * mod));

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[0] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[4] * _dim * mod));

	//																  	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[1] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[3] * _dim * mod));

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[1] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[5] * _dim * mod));

	//																  	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[2] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[3] * _dim * mod));

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[2] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[6] * _dim * mod));

	//																  	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[3] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[7] * _dim * mod));

	//																  	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[4] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[5] * _dim * mod));

	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[4] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[6] * _dim * mod));

	//																  	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[5] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[7] * _dim * mod));

	//																	
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[6] * _dim * mod));
	m_boundingGrid[_depth].push_back(FVEC3(_origin + offset[7] * _dim * mod));

	unsigned id = m_boundingGridIndices[_depth].size();
	for (unsigned k = 0; k < 24; ++k)
	{
		m_boundingGridIndices[_depth].push_back(id + k);
	}

	if (node.IsLeaf()) return;



	static FVEC3 ms[8] =
	{
		FVEC3(0, 0, 0),
		FVEC3(0, 1, 0),
		FVEC3(1, 1, 0),
		FVEC3(1, 0, 0),
		
		
		FVEC3(0, 0, 1),
		FVEC3(0, 1, 1),
		FVEC3(1, 1, 1),
		FVEC3(1, 0, 1),
	
			
	};
	const unsigned mcIndex[] = { 0, 1, 3, 2, 4, 5, 7, 6 };

	// build part of mesh
	CELL c;
	unsigned index = 0;
	for (unsigned i = 0; i < 8; ++i)
	{
		if (node.HasChildAtIndex(i))
		{
			index			= mcIndex[i];
			c.p[index]		= _origin + offset[i] * _dim;
			c.val[index]	= 1;
		}
	}

	TRIANGLE tris[5];
	unsigned N = Polygonize(c, 1, tris);
	std::vector<Vertex> nodeVertices;
	for (unsigned i = 0; i < N; ++i)
	{
		nodeVertices.push_back(Vertex(tris[i].p[0], voxel.normal, voxel.color));
		nodeVertices.push_back(Vertex(tris[i].p[1], voxel.normal, voxel.color));
		nodeVertices.push_back(Vertex(tris[i].p[2], voxel.normal, voxel.color));
	}

	m_lodMesh[_depth].insert(m_lodMesh[_depth].end(), nodeVertices.begin(), nodeVertices.end());



	// recurse
	for (unsigned i = 0; i < 8; ++i)
	{
		if (node.HasChildAtIndex(i))
		{
			FVEC3	 pos = offset[i] * (_dim * 0.5);
			unsigned ptr = node.GetChildAddress(i);

			BuildMesh(_depth + 1, _dim * 0.5, ptr, _origin + pos);

			
		}
	}
}

float SparseOctree::ApproximateSurface(FVEC3 _sample, const float _radius)
{
	return VECTOR::TVectorLength(FVEC3() - _sample) - _radius;
}

void SparseOctree::Traverse(unsigned _node, unsigned _depth, unsigned _dim, FVEC3 _origin, std::vector<VOXEL::TNode> _nodes)
{
	if (_depth > 1) return;
	TNode node = _nodes[_node];
	Voxel voxel = m_voxels[node.GetDataAddress()];
	if (node.IsLeaf()) return;

	static FVEC3 offset[8] =
	{
		FVEC3(0, 0, 0),
		FVEC3(1, 0, 0),
		FVEC3(0, 1, 0),
		FVEC3(1, 1, 0),
		FVEC3(0, 0, 1),
		FVEC3(1, 0, 1),
		FVEC3(0, 1, 1),
		FVEC3(1, 1, 1),
	};

	CELL c[4];
	if (node.HasChildAtIndex(0)) { c[0].p[0] = _origin + offset[0] * _dim; c[0].val[0] = 1;}
	if (node.HasChildAtIndex(1)) { c[0].p[1] = _origin + offset[1] * _dim; c[0].val[1] = 1;}
	if (node.HasChildAtIndex(4)) { c[0].p[4] = _origin + offset[4] * _dim; c[0].val[4] = 1;}
	if (node.HasChildAtIndex(5)) { c[0].p[5] = _origin + offset[5] * _dim; c[0].val[5] = 1;}

	if (node.HasChildAtIndex(1)) { c[1].p[0] = _origin + offset[0] * _dim; c[1].val[0] = 1; }
	if (node.HasChildAtIndex(5)) { c[1].p[1] = _origin + offset[1] * _dim; c[1].val[1] = 1; }
	{ c[1].p[4] = _origin + offset[4] * _dim; c[1].val[4] = 1; }
	{ c[1].p[5] = _origin + offset[5] * _dim; c[1].val[5] = 1; }

	if (node.HasChildAtIndex(4)) { c[2].p[0] = _origin + offset[0] * _dim; c[1].val[0] = 1; }
	if (node.HasChildAtIndex(5)) { c[2].p[1] = _origin + offset[1] * _dim; c[1].val[1] = 1; }
	{ c[2].p[4] = _origin + offset[4] * _dim; c[1].val[4] = 1; }
	{ c[2].p[5] = _origin + offset[5] * _dim; c[1].val[5] = 1; }

	if (node.HasChildAtIndex(5)) { c[3].p[0] = _origin + offset[0] * _dim; c[2].val[0] = 1; }
	{ c[3].p[1] = _origin + offset[1] * _dim; c[3].val[1] = 1; }
	{ c[3].p[4] = _origin + offset[4] * _dim; c[3].val[4] = 1; }
	{ c[3].p[5] = _origin + offset[5] * _dim; c[3].val[5] = 1; }

	unsigned N = 0;
	TRIANGLE tris[5];

	m_lodMesh[0].clear();
	std::vector<Vertex> nodeVertices;
	for (unsigned i = 0; i < 4; ++i)
	{
		N = Polygonize(c[i], 1, tris);
		nodeVertices.push_back(Vertex(tris[i].p[0], voxel.normal, voxel.color));
		nodeVertices.push_back(Vertex(tris[i].p[1], voxel.normal, voxel.color));
		nodeVertices.push_back(Vertex(tris[i].p[2], voxel.normal, voxel.color));
	}
	m_lodMesh[_depth].insert(m_lodMesh[_depth].end(), nodeVertices.begin(), nodeVertices.end());

	for (unsigned i = 0; i < 8; ++i)
	{
		if (node.HasChildAtIndex(i))
		{
			FVEC3	 pos = offset[i] * (_dim * 0.5);
			unsigned ptr = node.GetChildAddress(i);
			Traverse(ptr, _dim * 0.5, _depth + 1, _origin + pos, _nodes);
		}
	}
}

void SparseOctree::DualContour(void)
{
	unsigned root = 0;
	//Traverse(root, 0, m_header.gridlength, FVEC3(), gridNodes);
	int a = 42;
}