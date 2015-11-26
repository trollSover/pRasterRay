#pragma once

#include <stdint.h>
#include "../Utils/Math_Util.h"

namespace VOXEL
{
	struct NC
	{
		FVEC3 n;
		FVEC4 c;

		NC(FVEC3 _n = FVEC3(), FVEC4 _c = FVEC4())
			: n(_n), c(_c)
		{}
	};

	//struct GPU_INT64
	//{
	//	uint32_t lo;
	//	uint32_t hi;
	//
	//	GPU_INT64()
	//		: hi(0), lo(0)
	//	{}
	//};

	template<typename T>
	class VoxelData
	{
		/* Members */
	private:
			

		friend class SVO;
	protected:
	public:
		uint64_t	m_morton;
		T			m_data;
		/* Methods */
	private:
	protected:
	public:
		VoxelData(uint64_t _morton, T _data)
			: m_morton(_morton), m_data(_data)
		{	}

		VoxelData(void)
			: m_morton(0)
		{	}

		uint64_t Morton() const { return m_morton; }

		void PrintStat(bool _printCoords = false)
		{
			printf("morton: %" PRIu64, m_morton);
			if (_printCoords)
			{
				uint32_t x, y, z;
				MATH::mortonDecode(m_morton, x, y, z);
				printf(" = {%i,%i,%i}", x, y, z);
			}
			printf("\n");
		}

		/* Operators */
	public:
		bool operator > (const VoxelData& _vd) const { return m_morton > _vd.m_morton; }
		bool operator < (const VoxelData& _vd) const { return m_morton < _vd.m_morton; }
	};

	template<typename T>
	struct GPU_Voxel
	{
		//GPU_INT64 morton;
		uint64_t morton;
		T data;

		GPU_Voxel(VoxelData<T> _voxel)
		{
			assert(_voxel.m_morton <= MAXUINT32);
			morton = _voxel.m_morton;

			//morton.lo	= (_voxel.m_morton);
			//morton.hi	= (_voxel.m_morton >> 32);
			data		= _voxel.m_data;
		}
	};

	struct Voxel
	{
		FVEC4 color;
		FVEC3 normal;

		Voxel(void)
		{	}

		Voxel(const GPU_Voxel<NC>& _voxel)
			: color(_voxel.data.c), normal(_voxel.data.n)
		{	}
	};
};