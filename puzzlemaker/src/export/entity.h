#pragma once

#include "cglm/types.h"
#include "export/brush.h"
#include "item/item.h"

typedef struct
{
	int id;
  const char* className;
  const char* name;

  vec3 pos;
  vec3 rotation;

	ItemOutput* outputs;

  const char** kvs;
  const char** rawOutputs;
  int* brushes;

	char script;
} Entity;

void exportStartEntities();
void exportEntitiesProcessOutputs();
void exportEndEntities(FILE* file);
Entity* getEntityList();

Entity* exportCreateEntity();
void exportEntityAddKv(Entity* ent, ItemKv* kv);
void exportEntityAddKvs(Entity* ent, const char* kv);
void exportEntityAddKvss(Entity* ent, const char* key, const char* value);
void exportEntityAddBrush(Entity* ent, Brush* brush);
void exportEntityAddRawOutput(Entity* ent, const char* output, const char* name, const char* input, const char* arg, float delay);
