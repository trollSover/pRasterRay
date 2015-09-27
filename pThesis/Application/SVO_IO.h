#pragma once

#include <fstream>

#include "../CoreSystems/CoreStd.h"
#include "Node.h"

#include <algorithm>
#include <list>
#include <math.h>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

using namespace VOXEL;

inline bool FileExist(const std::string& _file)
{
	FILE* f;
	fopen_s(&f, _file.c_str(), "r");
	if (!f)
		return false;
	fclose(f);
	return true;
}

struct SVO_Header
{
	int version;
	std::string filename;
	//size_t gridlength;
	//size_t numNodes;
	//size_t numData;
	//int numLevels;

	int32_t gridlength;
	int32_t numNodes;
	int32_t numData;
	int32_t numLevels;

	SVO_Header()
		: version(-1), filename(""), gridlength(-1), numNodes(-1), numData(-1), numLevels(-1)
	{}

	SVO_Header(int _version, std::string _filename, size_t _gridLength, size_t _nodes, size_t _data, int _levels)
		: version(_version), filename(_filename), gridlength(_gridLength), numNodes(_nodes), numData(_data), numLevels(_levels)
	{}

	void Print() const
	{
		printf("version: %i\nfilename: %s\ngridlength: %zu\nnum nodes: %zu\nnum data: %zu\nnum levels: %i\n", version, filename, gridlength, numNodes, numData, numLevels);
	}

	bool FilesExist() const { return FileExist(filename + "oct") && FileExist(filename + "noct") && FileExist(filename + "doct"); }
};

inline void WriteHeader(const SVO_Header& _header)
{
	FILE* f;
	fopen_s(&f, std::string(_header.filename + ".oct").c_str(), "wb");

	if (!f)
	{
		PrintError(AT, "failed to create SVO header");
		return;
	}

	//fwrite(&_header, sizeof(SVO_Header), 1, f);

	fwrite(&_header.version, sizeof(int), 1, f);
	//fwrite(&_header.filename, sizeof(char) * _header.filename.length(), 1, f);
	fwrite(&_header.gridlength, sizeof(size_t), 1, f);
	fwrite(&_header.numNodes, sizeof(size_t), 1, f);
	fwrite(&_header.numData, sizeof(size_t), 1, f);
	fwrite(&_header.numLevels, sizeof(int), 1, f);

	fclose(f);
}

inline bool ReadHeader(const std::string& _filename, SVO_Header& _header)
{
	FILE* f;
	fopen_s(&f, std::string(_filename + ".oct").c_str(), "rb");

	if (!f)
	{
		PrintError(AT, "failed to open SVO header");
		return false;
	}

	//fread(&_header.version, sizeof(int), 1, f);
	//fread(&_header.gridlength, sizeof(size_t), 1, f);
	//fread(&_header.numNodes, sizeof(size_t), 1, f);
	//fread(&_header.numData, sizeof(size_t), 1, f);
	//fread(&_header.numLevels, sizeof(int), 1, f);
	//_header.filename = _filename;
;
	fread(&_header.version, sizeof(int32_t), 1, f);
	fread(&_header.gridlength, sizeof(int32_t), 1, f);
	fread(&_header.numNodes, sizeof(int32_t), 1, f);
	fread(&_header.numData, sizeof(int32_t), 1, f);
	fread(&_header.numLevels, sizeof(int32_t), 1, f);
	_header.filename = _filename;

	fclose(f);
	return true;
}



template<typename T>
inline size_t WriteData(FILE* _dOut, const VoxelData<T>& _vd, size_t& _dPos)
{
	fwrite(&_vd, sizeof(VoxelData<T>), 1, _dOut);
	_dPos++;
	return _dPos - 1;
}

template<typename T>
inline size_t WriteNode(FILE* _nOut, const Node<T>& _vn, size_t& _nPos)
{
	fwrite(&_vn, sizeof(Node<T>), 1, _nOut);
	_nPos++;
	return _nPos - 1;
}

template<typename T>
inline void ReadData(FILE* _f, VoxelData<T>& _vd)
{
	fread(&_vd, sizeof(VoxelData<T>), 1, _f);
}

template<typename T>
inline void ReadNode(FILE* _f, Node<T>& _vn)
{
	//fread(&_vn, sizeof(Node<T>), 1, _f);
	fread(&_vn, 17, 1, _f);
}