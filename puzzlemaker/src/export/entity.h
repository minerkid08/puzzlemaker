#pragma once

#include "cglm/types.h"
#include "export/brush.h"
#include "item/item.h"

typedef struct
{
  const char* className;
  const char* name;

  vec3 pos;
  vec3 rotation;

	ItemOutput* outputs;

  const char** kvs;
  Brush** brushes;
} Entity;

void exportStartEntities();
void exportEndEntities(FILE* file);

Entity* exportCreateEntity();
void exportEntityAddKv(Entity* ent, ItemKv* kv);
void exportEntityAddKvs(Entity* ent, const char* kv);
void exportEntityAddKvss(Entity* ent, const char* key, const char* value);
void exportEntityAddBrush(Entity* ent, Brush* brush);
