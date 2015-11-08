#pragma once

#include "../CoreSystems/CoreStd.h"
#include "SVO_Loader.h"
#include "Node.h"
#include "VoxelData.h"

#include "MarchingCubes.h"
#include "../Render/ShaderInputFormats.h"

template<typename T>
class SparseVoxelOctree
{
private:
	uint64_t max_coord;
protected:
public:
	SVO_Loader m_svoLoader;
	std::vector<Vertex> vertices;
	std::vector<CELL> cells;

	//std::vector<CELL> grid;

	struct VD { uint32_t y; size_t dataPtr; VD() : y(0), dataPtr(0) {}  };
	//uint32_t mcGrid[128][128];
	VD mcGrid[1024][1024];
	uint32_t mX, mZ, mY;

	uint32_t*** grid;
	uint32_t gridlength;
	std::vector<uint32_t> mcMIndices;
private:
protected:
public:

	SparseVoxelOctree() : mX(0), mZ(0), mY(0)
	{}

	bool CreateSVO(const std::string& _file) 
	{ 
		if (!m_svoLoader.VLoad(_file))
			return false;

		TNode* root =  &m_svoLoader.m_nodes[m_svoLoader.m_nodes.size() - 1];
//		root->parentPtr = -1; // note: unsigned int
		uint32_t nodePtr = m_svoLoader.m_nodes.size() - 1;
		AssignParentPointer(root, nodePtr);

//		grid.resize(m_svoLoader.m_voxels.size());
		mX = m_svoLoader.mX;
		mY = m_svoLoader.mY;
		mZ = m_svoLoader.mZ;
		grid = VNEW uint32_t**[mX];
		for (int i = 0; i < mX; ++i)
		{
			grid[i] = VNEW uint32_t*[mZ];
			
			for (int j = 0; j < mZ; ++j)
			{
				grid[i][j] = VNEW uint32_t[mY];
				std::fill(grid[i][j], grid[i][j] + mY, 0);
			}
		}
		//for (int i = 0; i < mX; ++i)
		//for (int j = 0; j < mZ; ++j)
		//for (int k = 0; k < mY; ++k)
		//if (grid[i][j][k] != 0)
		//{

		//	printf("i=%i, j=%i, k=%i val = %i!\n", i,j,k, grid[i][j][k]);
		//	break;
		//}

		int nmem = sizeof(GPU_Node) * m_svoLoader.m_nodes.size() / 1024 / 1024;
		int vmem = sizeof(VoxelData<NC>) * m_svoLoader.m_voxels.size() / 1024 / 1024;

		return true;
	}

	void AssignParentPointer(TNode*& _node, uint32_t _nodePtr)
	{
		if (_node->CountChildren() > 0)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (_node->HasChildAtIndex(i))
				{
					uint32_t childPtr = _node->GetChildAddress(i);
//					m_svoLoader.m_nodes[childPtr].parentPtr = _nodePtr;
					TNode* child = &m_svoLoader.m_nodes[childPtr];
					AssignParentPointer(child, childPtr);
				}
			}
		}
	}

	std::vector<TNode>& GetNodeList() { return m_svoLoader.m_nodes; }


	std::vector<GPU_Voxel<T>>& GetVoxelList() { return m_svoLoader.m_voxels; }


	TNode& GetRoot() 
	{ 
		//return m_svoLoader.m_nodes[0];
//		m_svoLoader.m_nodes[m_svoLoader.m_nodes.size() - 1].parentPtr = -1;	// note: unsigned int

		return m_svoLoader.m_nodes[m_svoLoader.m_nodes.size()-1]; 


	}

	void GetSVO(TNode& _parent, std::vector<GPU_Voxel<T>>& _voxels, int _level = 0)
	{
		uint32_t x, y, z;

		uint64_t mort = m_svoLoader.m_voxels[_parent.GetDataAddress()].morton;

		if (mort != 0)
		{
			MATH::mortonDecode(mort, x, y, z);

			mcGrid[x][z].y =  max(mcGrid[x][z].y, y );
			mcGrid[x][z].dataPtr = _parent.dataPtr; 
		}

		if (_parent.CountChildren() == 0)
		{
			_voxels.push_back(m_svoLoader.m_voxels[_parent.GetDataAddress()]);
		}

		if (_parent.CountChildren() > 0)
		{
			for (int i = 0; i < 8; ++i)
			{
				int index = MortonOrder[i];
				if (_parent.HasChildAtIndex(index))
				{
					GetSVO(m_svoLoader.m_nodes[_parent.GetChildAddress(index)], _voxels, _level + 1);
				}
			}
		}
	}

	void GetCell(const TNode& _parent, CELL& _cell, int _index)
	{
		int length = m_svoLoader.m_header.gridlength;

		for (int x = 0; x < mX; x += 1)
		{
			for (int z = 0; z < mZ; z += 1)
			{
				try
				{
					CELL c1;
					c1.p[0] = FVEC3(x, mcGrid[x][z].y, z);
					c1.p[1] = FVEC3(x + 1, mcGrid[x + 1][z].y, z);
					c1.p[4] = FVEC3(x, mcGrid[x][z + 1].y, z + 1);
					c1.p[5] = FVEC3(x + 1, mcGrid[x + 1][z + 1].y, z + 1);

					c1.val[0] = c1.val[1] = c1.val[4] = c1.val[5] = 1;

					c1.n[0] = m_svoLoader.m_voxels[mcGrid[x][z].dataPtr].data.n;
					c1.n[1] = m_svoLoader.m_voxels[mcGrid[x + 1][z].dataPtr].data.n;
					c1.n[4] = m_svoLoader.m_voxels[mcGrid[x][z + 1].dataPtr].data.n;
					c1.n[5] = m_svoLoader.m_voxels[mcGrid[x + 1][z + 1].dataPtr].data.n;

					c1.c[0] = m_svoLoader.m_voxels[mcGrid[x][z].dataPtr].data.c;
					c1.c[1] = m_svoLoader.m_voxels[mcGrid[x + 1][z].dataPtr].data.c;
					c1.c[4] = m_svoLoader.m_voxels[mcGrid[x][z + 1].dataPtr].data.c;
					c1.c[5] = m_svoLoader.m_voxels[mcGrid[x + 1][z + 1].dataPtr].data.c;

					cells.push_back(c1);

				}
				catch (std::exception& e)
				{
					printf("%i:%i\ = %sn", x, z,e.what());
					return;	// make proper fail catch
				}
			}
		}
	}
};