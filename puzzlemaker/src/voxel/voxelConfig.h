#pragma once

typedef struct
{
	int blackEditor;
	int whiteEditor;

	const char* nodraw;

	const char* blackFloor;
	const char* blackWall;
	const char* blackCeiling;

	const char* whiteFloor;
	const char* whiteWall;
	const char* whiteCeiling;
} VoxelConfig;

extern VoxelConfig voxelConfig;

void loadVoxelConfig();
