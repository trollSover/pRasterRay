#include "SVO_Loader.h"

static unsigned int counter;

void Traverse(std::vector<GPU_Voxel<NC>> _voxels, std::vector<GPU_Node> _nodes, GPU_Node& _currentNode)
{
	counter++;

	if (_currentNode.NChildren() <= 0)
		return;

	//printf("%i: I have %i children\n", counter, _currentNode.NChildren());
	

	for (int i = 0; i < 8; ++i)
	{
		if (_currentNode.HasChild(i))
		{
			Traverse(_voxels, _nodes, _nodes[_currentNode.GetChildAddress(i)]);
		}
	}
}

bool SVO_Loader::VLoad(const std::string& _file)
{
	mX = mY = mZ = 0;

	if (!FileExist(_file + ".oct"))
	{
		PrintError(AT, _file + " not found");
		return false;
	}

	if (!ReadHeader(_file, m_header))
	{
		PrintError(AT, _file + " is corrupted");
		return false;
	}

	FILE* dataIn = nullptr;
	fopen_s(&dataIn, std::string(m_header.filename + ".doct").c_str(), "rb");

	if (!dataIn)
	{
		PrintError(AT, _file + ".doct not found");
		return false;
	}

	VoxelData<NC> dIn;
	for (size_t i = 0; i < m_header.numData; ++i)
	{
		ReadData(dataIn, dIn);
		//dIn.m_data.c = Normalize(dIn.m_data.c);
		FVEC3 c = FVEC3(dIn.m_data.c.x, dIn.m_data.c.y, dIn.m_data.c.z);
		c = Normalize(c);
		dIn.m_data.c = FVEC4(c.x, c.y, c.z, 1);

		m_voxels.push_back(dIn);


		uint32_t x, y, z;
		MATH::mortonDecode(dIn.m_morton, x, y, z);
		mX = max(mX, x);
		mY = max(mY, y);
		mZ = max(mZ, z);
	}

	if (m_voxels.size() != m_header.numData)
	{
		printf("amount of data (%zf) read does not match expected amount (%zf)\n", m_voxels.size(), m_header.numData);
	}

	fclose(dataIn);

	FILE* nodeIn = nullptr;
	fopen_s(&nodeIn, std::string(m_header.filename + ".noct").c_str(), "rb");

	if (!nodeIn)
	{
		PrintError(AT, _file + ".noct not found");
		
		return false;
	}
	int nLevels = m_header.numLevels + 1;

	std::vector<int> lCount;
	lCount.resize(nLevels);
	for (int i = 0; i < nLevels; ++i)
		lCount[i] = 0;

	Node<NC> nIn;
	GPU_Node gpn;
	for (size_t i = 0; i < m_header.numNodes; ++i)
	{

		ReadNode(nodeIn, nIn);
		nIn.level = (nLevels-1) - nIn.level;

		m_nodes.push_back(nIn);

		assert(nIn.level >= 0 && nIn.level < nLevels);
		lCount[nIn.level]++;

		//testNodes.push_back(nIn);
		if (i == (m_header.numNodes - 1))
		{
			int a = 42;
			gpn = GPU_Node(nIn);
		}
	}

	TNode* rt = &m_nodes[m_header.numNodes - 1];

	//for (int i = 0; i < 8; ++i)
	//{
	//	if (rt->HasChildAtIndex(i))
	//	{
	//		printf("---TNode---\n");
	//		TNode* n = &m_nodes[rt->GetChildAddress(i)];
	//		printf("parent address to %i == %i\n", i, rt->GetChildAddress(i));
	//		printf("child[%i] address: %i = %s\n", i, n->basePtr, n->IsLeaf() == true ? "is leaf" : "not leaf");
	//	}
	//	else printf("---no TNode---\n");
	//	if (gpn.HasChild(i))
	//	{
	//		printf("---GPUNode---\n");
	//		TNode tn = m_nodes[gpn.GetChildAddress(i)];
	//		printf("parent address to %i == %i\n", i, gpn.GetChildAddress(i));
	//		printf("child[%i] address: %i = %s\n", i, tn.basePtr, tn.IsLeaf() == true ? "is leaf" : "not leaf");
	//	}
	//	else printf("---no GPUNode---\n");

	//}

	for (int i = 0; i < nLevels; ++i)
	{
		printf("level: %i = %i nodes\n", i, lCount[i]);
	}

	int nsize = sizeof(TNode);

	printf("nodes: %i\nvoxels: %i\n", m_nodes.size(), m_voxels.size());

	if (m_nodes.size() != m_header.numNodes)
	{
		printf("amount of nodes (%zf) read does not match expected amount (%zf)\n", m_voxels.size(), m_header.numData);
	}

	int x, y, z;
	x = 2;
	y = 0;
	z = 0;

	int ox, oy, oz;
	ox = 1;
	oy = 1;
	oz = 1;

	uint32_t mort = MATH::mortonEncode_LUT(x, y, z);

	TNode currentNode = m_nodes[m_nodes.size() - 1];
	int child = 0;
	//while (true)
	{
		if (x >= ox)
		{
			// 1,3,5,7
			child |= 170;
		} //  0,2,4,6
		else child |= 85;

		if (y >= oy)
		{
			// 4,5,6,7
			child |= 15;
		}  //  0,1,2,3
		else child |= 240;
		if (z >= oz)
		{
			// 2,3,6,7
			child |= 51;
		}  //  0,1,4,5
		else child |= 204;
	}
	
	fclose(nodeIn);

	return true;
}