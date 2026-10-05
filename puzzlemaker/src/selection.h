#include "antline/antline.h"
#include "cglm/cglm.h"
#include "item/item.h"
#include "voxel/voxel.h"

#define SELECTION_NONE 0
#define SELECTION_VOXEL 1
#define SELECTION_ITEM 2
#define SELECTION_ANTLINE 3
#define SELECTION_ANTLINE_SEG 4

typedef struct
{
	char type;
	ivec3 voxelPos;
	ivec3 voxel2Pos;
	char voxelDir;
	Voxel* voxel;
	
	Item* item;

	Antline* antline;
	AntlineSegment* antlineSeg;
} Selection;

extern Selection selection;

void beginSelection(vec3 mouseDir);
void updateSelection(vec3 mouseDir);
void endSelection();
char isSelecting();
void clearSelection();
