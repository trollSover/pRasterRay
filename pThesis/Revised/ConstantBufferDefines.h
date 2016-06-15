#pragma once

#include "../CoreSystems/CoreStd.h"

namespace RasterRay
{
	struct CameraConstantBuffer
	{
		FMAT4X4	worldViewProjection;		// 64
		FMAT4X4 view;						// 64
		FMAT4X4 projection;					// 64
		FMAT4X4 world;						// 64

		FMAT4X4 viewInverse;				// 64
		FMAT4X4 projectionInverse;			// 64
		FMAT4X4 worldViewProjectionInverse;	// 64
		FMAT4X4 rotation;					// 64

		FVEC3	cameraPos;					// 12
		FVEC3	cameraDir;					// 12
		FVEC3	cameraRight;				// 12
		FVEC3	cameraUp;					// 12
	};

	struct WindowConstantBuffer
	{
		unsigned width;
		unsigned height;
		unsigned padding[2];
	};

	struct OctreeConstantBuffer
	{
		//unsigned	numNodes;
		unsigned	gridLength;		// 4
		unsigned	maxDepth;		// 4
		unsigned	rootIndex;		// 4
		FVEC3		rootOrigin;		// 12
		unsigned	pad0, pad1;		// 8
	};								// = 32

	struct DebugConstantBuffer
	{
		FMAT4X4 rotateOnAxis;
		FVEC4	color;
	};
}