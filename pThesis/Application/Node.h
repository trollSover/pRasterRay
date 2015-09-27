#pragma once

#include "VoxelData.h"

namespace VOXEL
{
	static const FVEC3 NodeOrigin[] = { FVEC3(0, 0, 0),		// Front Bottom Left
		FVEC3(1, 0, 0),		// Front Bottom Right
		FVEC3(0, 1, 0),		// Front Top	Left
		FVEC3(1, 1, 0),		// Front Top	Right
		FVEC3(0, 0, 1),		// Back  Bottom Left
		FVEC3(1, 0, 1), 	// Back  Bottom Right							
		FVEC3(0, 1, 1),		// Back  Top	Left
		FVEC3(1, 1, 1)		// Back  Top	Right
	};

	/* Natural layout of morton encoded octree */
	static const int MortonOrder[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	/* Quadrant traversal layout */
	static const int QuadrantOrder[]	= { 0, 1, 4, 5, 2, 3, 6, 7 };
	/* Marching cube traversal layout for cell construction */
	static const int MCOrder[]			= { 0, 1, 3, 2, 4, 5, 7, 6 };

	//static const std::string ChildOrder[] = { "front_bottom_left",
	//	"front_bottom_right",
	//	"front_top_left",
	//	"front_top_right",
	//	"back_bottom_left",
	//	"back_bottom_right",
	//	"back_top_left",
	//	"back_top_right" };

	static const char	EMPTY = -1;	// empty space voxel
	static const size_t NODATA = 0;
	static const char	LEAF[8] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY };
	static const int	NOCHILD = 8;
	static const int	LEAFMASK = -2004318072;	/* (binary) 10001000100010001000100010001000 : each nibble = NOCHILD 
												   note: nibble > NOCHILD is undefined, should fix that */

	template<typename T>
	class Node
	{
		/* Members */
	private:

		friend class SVO;
	protected:
	public:
		uint32_t	dataPtr;
		uint32_t	basePtr;
		int8_t		level;
		char		children[8];

		//T cache;
		/* Methods */
	private:
	protected:
	public:
		Node()
			: basePtr(0), dataPtr(0), level(0)//, cache(T())
		{
			memset(&children, (char)EMPTY, 8);
			//memset(&m_voxelData, NODATA, 5);
		}

		T	   GetCache()						const { return cache; }
		size_t GetDataAddress()					const { return dataPtr; }
		size_t GetBaseAddress()					const { return basePtr; }
		size_t GetChildAddress(uint32_t _index)	const
		{
			assert(_index < 8);
			assert(children[_index] != EMPTY);
			return basePtr + children[_index];
		}



		/* Check if Node has a child at the index position */
		bool HasChild(uint32_t _index)	const { return children[_index] != EMPTY; }

		/* Check if node is pointing to any data */
		bool HasData() const { return !(dataPtr == NODATA); }

		/* Check if Node is leaf (no children activated) */
		bool IsLeaf() const
		{
			//return memcmp(m_children, LEAF, 8 * sizeof(char)) == 0 ? true : false; 
			if (memcmp(children, LEAF, 8 * sizeof(char)) == 0)
				return true;
			return false;
		}

		/* Check if leaf and no data */
		bool IsNull() const { return IsLeaf() && !HasData(); }

		int NChildren() const
		{
			int n = 0;
			for (int i = 0; i < 8; ++i)
			if (children[i] != EMPTY)
				n++;
			return n;
		}
	};

	struct TNode
	{
	private:


	protected:
	public:
		/*  1*8 bits: child boolean
		3*8 bits: child offsets	*/
		

		uint32_t	dataPtr;
		uint32_t	basePtr;
		uint32_t	parentPtr;
		uint32_t	children;
		int32_t		level;

	private:
		bool HasData() const { return NODATA != dataPtr; }
		
	protected:
	public:
		uint32_t GetDataAddress()					const { return dataPtr; }
		uint32_t GetBaseAddress()					const { return basePtr; }

		uint32_t GetChildAddress(uint32_t _index)	const
		{
			assert(_index < 8);
			return basePtr + (children >> (8 + 3 * _index) & ((1 << 3) - 1));
		}

		bool IsNull() const { return !HasData() && IsLeaf(); }

		/* compare first 8 bits to zero, i.e. 'no children' */
		bool IsLeaf() const 
		{ 
			return  0 == (children & ((1 << 8) - 1)); 
		}

		bool HasChildAtIndex (const uint32_t _index) const 
		{ 
			return 1 == ((children >> _index) & ((1 << 1) - 1)); 
		}

		/* count set '1' of first 8 bits (__popcnt) */
		unsigned int CountChildren() const 
		{ 
			return __popcnt(children & ((1 << 8) - 1)); 
		}

		TNode() 
			: children(0), dataPtr(0), basePtr(0), level(-1)
		{ }

		template<typename T>
		TNode(const Node<T>& _node) 
			: children(0), dataPtr(_node.dataPtr), basePtr(_node.basePtr), level(_node.level)
		{

			/*
			Node composition as described by:
			"Realtime GPU-Raycasting of Volume Data with Smooth Splines on Tetrahedral Partitions" - Dominik Wodniok
			http://www.gris.informatik.tu-darmstadt.de/~dwodniok/files/wodniok_thesis.pdf

			// first 8 bits denotes child/no-child as true/false (1/0)
			// following 3*8 bits (24 bits in total) denotes child pointer offsets

			obtaining a child status:
			- offset by child index (0-7), bitmask by 1 (value range: 0-1)
			child = (children >> N) & ((N << 1) - 1)

			obtaining a child offset:
			- offset by child index (0-7) and constant bitoffset (8), bitmask by 3 (value range: 0-7)
			offset = (children >> (8 + 3N) & ((1 << 3) - 1)
			*/

			for (int i = 0; i < 8; ++i)
			{
				if (_node.children[i] != EMPTY)
				{
					children |= (1 << i);								// set single bit at index 'i' (index range: 0-1)
					children |= (_node.children[i] << (8 + i * 3));		// set three bits at interval 'i -> i+3' (index range: 8-29, interval range: 8*3, value range: 0-7)
				}
			}
		}
	};

	/*  NOTE:	Ptr data types needs to be changed into 32bits types since hlsl does not carry 64bit integers 
				- larger data sets have to be partitioned to be indexed correctly. Voxeldata morton can retain the
				  INT64 type since it's used to derive spatial position in the grid and NOT as an index 
		  
		FIXED:	4 byte index only guarantees a maximum of 2^10 gridlength, i.e. 1024^3, or a max-coordinate of (1024,1024,1024). */

	struct GPU_Node
	{
		uint32_t	dataPtr;
		uint32_t	basePtr;
		uint32_t	parentPtr;
		int32_t		children;
		int32_t		level;

		GPU_Node() 
			: dataPtr(0), basePtr(0), parentPtr(0), children(0), level(-1)
		{}
		
		template<typename T>
		GPU_Node(const Node<T>& _node)
		{
			assert(_node.dataPtr <= MAXUINT32);
			assert(_node.basePtr <= MAXUINT32);

			dataPtr = _node.dataPtr;
			basePtr = _node.basePtr;
			level	= _node.level;
			children = 0;
			/* Child bitmask order by morton encoding:
				0: front bottom left	(FBL)
				1: front bottom right	(FBR)
				2: front top	left	(FTL)
				3: front top	right	(FTR)
				4: back	 bottom left	(BBL)
				5: back  bottom right	(BBR)
				6: back  top	left	(BTL)
				7: back  top	right	(BTR)	
			
				- empty child nodes are marked with value '8' (NOCHILD) i.e. out of bounds	*/

			_node.children[7] == EMPTY ? children |= (NOCHILD)		 : children |=  _node.children[7];
			_node.children[6] == EMPTY ? children |= (NOCHILD << 4)  : children |= (_node.children[6] << 4);
			_node.children[5] == EMPTY ? children |= (NOCHILD << 8)  : children |= (_node.children[5] << 8);
			_node.children[4] == EMPTY ? children |= (NOCHILD << 12) : children |= (_node.children[4] << 12);
			_node.children[3] == EMPTY ? children |= (NOCHILD << 16) : children |= (_node.children[3] << 16);
			_node.children[2] == EMPTY ? children |= (NOCHILD << 20) : children |= (_node.children[2] << 20);
			_node.children[1] == EMPTY ? children |= (NOCHILD << 24) : children |= (_node.children[1] << 24);
			_node.children[0] == EMPTY ? children |= (NOCHILD << 28) : children |= (_node.children[0] << 28);
		}

		uint32_t GetDataAddress()					const { return dataPtr; }
		uint32_t GetBaseAddress()					const { return basePtr; }

		uint32_t GetChildAddress(uint32_t _index)	const
		{
			assert(_index < 8);
			int offset = MATH::extractNBits(children, _index, 4); 
			return basePtr + offset;
		}


		/* Check if Node has a child at the index position */
		bool HasChild(uint32_t _index)	const 
		{ 
			int child = MATH::extractNBits(children, _index, 4);
			return NOCHILD != child;
		}

		/* Check if node is pointing to any data */
		bool HasData() const { return NODATA != dataPtr; }

		/* Check if Node is leaf (no children activated) */
		bool IsLeaf() const	{ return children == LEAFMASK; } /* children == 10001000100010001000100010001000 (all offsets = 8, ie no children) */
		

		/* Check if leaf and no data */
		bool IsNull() const { return IsLeaf() && !HasData(); }

		int NChildren() const
		{
			int n = 0;
			for (int i = 0; i < 8; ++i)
				if (HasChild(i))
					n++;
			return n;
		}
	};
};