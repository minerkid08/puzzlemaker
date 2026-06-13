#include "export/entity.h"
#include "dynList.h"
#include "export/brush.h"
#include "item/item.h"
#include <stdio.h>
#include <string.h>

static Entity* entities;

void exportStartEntities()
{
	entities = dynList_new(0, sizeof(Entity));
	dynList_reserve((void**)&entities, 32);
}

Entity* exportCreateEntity()
{
	int len = dynList_size(entities);
	dynList_resize((void**)&entities, len + 1);
	Entity* ent = &entities[len];

	ent->brushes = 0;
	ent->kvs = 0;

	return ent;
}

void exportEntityAddKv(Entity* ent, ItemKv* kv)
{
	char buf[50];

	ItemKvDef* def = kv->def;
	if (def->type == TYPE_INT)
		snprintf(buf, 50, "\"%s\" \"%d\"", def->name, kv->value.i);
	if (def->type == TYPE_BOOL)
		snprintf(buf, 50, "\"%s\" \"%d\"", def->name, (int)kv->value.b);
	if (def->type & TYPE_DROPDOWN)
	{
		int type = (def->type & (~TYPE_DROPDOWN));
		if (type == TYPE_STRING)
			snprintf(buf, 50, "\"%s\" \"%s\"", def->name, kv->def->dropValues[kv->value.i].s);
		if (type == TYPE_INT)
			snprintf(buf, 50, "\"%s\" \"%d\"", def->name, kv->def->dropValues[kv->value.i].i);
	}

	if (ent->kvs == 0)
		ent->kvs = dynList_new(0, sizeof(const char*));

	int len = dynList_size(ent->kvs);
	dynList_resize((void**)&ent->kvs, len + 1);
	ent->kvs[len] = strdup(buf);
}

void exportEntityAddKvs(Entity* ent, const char* kv)
{
	if (ent->kvs == 0)
		ent->kvs = dynList_new(0, sizeof(const char*));

	int len = dynList_size(ent->kvs);
	dynList_resize((void**)&ent->kvs, len + 1);
	ent->kvs[len] = strdup(kv);
}

void exportEntityAddKvss(Entity* ent, const char* key, const char* value)
{
	char buf[50];
	snprintf(buf, 50, "\"%s\" \"%s\"", key, value);
	if (ent->kvs == 0)
		ent->kvs = dynList_new(0, sizeof(const char*));

	int len = dynList_size(ent->kvs);
	dynList_resize((void**)&ent->kvs, len + 1);
	ent->kvs[len] = strdup(buf);
}

void exportEntityAddBrush(Entity* ent, Brush* brush)
{
	int i = 0;
	if (ent->brushes == 0)
		ent->brushes = dynList_new(1, sizeof(Brush*));
	else
	{
		int len = dynList_size(ent->brushes);
		dynList_resize((void**)&ent->brushes, len + 1);
		i = len;
	}
	ent->brushes[i] = brush;
}

void exportEndEntities(FILE* file)
{
	int len = dynList_size(entities);
	for (int i = 0; i < len; i++)
	{
		Entity* entity = &entities[i];

		fprintf(file, "entity\n{\n");
		fprintf(file, "\"id\" \"%d\"\n", i + 1);
		fprintf(file, "\"classname\" \"%s\"\n", entity->className);
		fprintf(file, "\"origin\" \"%f %f %f\"\n", -entity->pos[0] * 64, entity->pos[2] * 64, entity->pos[1] * 64);
		fprintf(file, "\"angles\" \"%f %f %f\"\n", entity->rotation[2], entities->rotation[1], entities->rotation[0]);
		fprintf(file, "\"targetname\" \"%s\"\n", entity->name);

    free((char*)entity->name);

		char buf[100];

		if (entity->kvs)
		{
			int len = dynList_size(entity->kvs);
			for (int i = 0; i < len; i++)
			{
				fprintf(file, "%s", entity->kvs[i]);
				free((char*)entity->kvs[i]);
			}
			dynList_free(entity->kvs);
		}

		if (entity->outputs)
		{
			int outputLen = dynList_size(entity->outputs);
			if (outputLen > 0)
			{
				fprintf(file, "\n  connections\n  {\n");
				for (int i = 0; i < outputLen; i++)
				{
					ItemOutput* output = &entity->outputs[i];
					Item* item = getItem(output->entity);
					snprintf(buf, 100, "%s%d", item->def->name, output->entity);
					if (output->inverted)
					{
						if (output->input->falseArg)
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									output->input->falseInput, output->input->falseArg);
						else
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									output->input->falseInput);

						if (output->input->trueArg)
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
									output->input->trueInput, output->input->trueArg);
						else
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
									output->input->trueInput);
					}
					else
					{
						if (output->input->trueArg)
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									output->input->trueInput, output->input->trueArg);
						else
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									output->input->trueInput);

						if (output->input->falseArg)
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
									output->input->falseInput, output->input->falseArg);
						else
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput, buf,
									output->input->falseInput);
					}
				}
				fprintf(file, "  }");
			}
		}

		if (entity->brushes)
		{
			int l = dynList_size(entity->brushes);
			for (int j = 0; j < l; j++)
			{
				Brush* b = entity->brushes[j];
				exportBrush(file, b);
			}
			dynList_free(entity->brushes);
		}

		fprintf(file, "}\n");
	}
}
