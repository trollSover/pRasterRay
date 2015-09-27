# pRasterRay

CONTROLS
*move using WASD
*look using mouse
*ESC to exit application

NOTES
*compile x64 for the moment since a IO mismatch reading x32 has not been fixed yet

*change SOURCE in MASTER64.txt to change model, however there's a bug that mess up
the color data for some of the models (not sure if the bug occurs in this application
or in the voxelizing application)

FILES OF INTEREST
*Assets/RCSinglePassVoxel.hlsl
*Application/VoxelApp .h/.cpp
*Application/SVO.h

RAYCAST BUG
*Traversal not working, the current traversal method (found in RCSinglePassVoxel)
is not used nor correct - disregard for now

*There's some issue with the spaces, it seems like the rasterized part
and the raycasted part resides in different spaces. The raycasted AABB of the
root node (the only raycasted node at the moment) tends to follow the camera
around. I believe this is partly due to threadId = pixelposition when I write the 
result into the colour buffer/texture.

*The raycasted box disappears if you move inside it, this is not a bug, it's simply
done to avoid the result of negative raydirections