#pragma once

#include "../CoreSystems/CoreStd.h"
#include "ModelLoader.h"

#include "SVO_IO.h"

class SVO_Loader
	: public ModelLoader
{
private:
protected:
public:
	int mX, mY, mZ;
	SVO_Header m_header;
	std::vector<GPU_Voxel<NC>>	m_voxels;
	std::vector<GPU_Node>			m_nodes;
	std::vector<TNode> testNodes;
private:
protected:
public:
	bool VLoad(const std::string& _file) final;
};