#pragma once

typedef struct
{
	int blackEditor;
	int whiteEditor;

	const char* nodraw;
	const char* backstage;

	const char* blackFloor;
	const char* blackWall;
	const char* blackCeiling;

	const char* whiteFloor;
	const char* whiteWall;
	const char* whiteCeiling;

	const char* blackFloorMini;
	const char* blackWallMini;
	const char* blackCeilingMini;

	const char* whiteFloorMini;
	const char* whiteWallMini;
	const char* whiteCeilingMini;
} VoxelConfig;

extern VoxelConfig voxelConfig;

void loadVoxelConfig();
