#include "export/entity.h"
#include "dynList.h"
#include "export/brush.h"
#include "item/item.h"
#include <stdio.h>
#include <string.h>

static Entity* entities;

static __attribute__((constructor)) void init()
{
	entities = dynList_new(0, sizeof(Entity));
	dynList_reserve((void**)&entities, 32);
}

Entity* getEntityList()
{
	return entities;
}

void exportStartEntities()
{
	dynList_resize((void**)&entities, 0);
}

Entity* exportCreateEntity()
{
	int len = dynList_size(entities);
	dynList_resize((void**)&entities, len + 1);
	Entity* ent = &entities[len];

	ent->id = len;

	ent->brushes = 0;
	ent->kvs = 0;
	ent->outputs = 0;
	ent->rawOutputs = 0;
	ent->script = 0;

	return ent;
}

void exportEntityAddKv(Entity* ent, ItemKv* kv)
{
	if (ent->kvs == 0)
		ent->kvs = dynList_new(0, sizeof(const char*));

	int len = dynList_size(ent->kvs);
	char buf[128];

	ItemKvDef* def = kv->def;
	if (def->type & TYPE_INSTANCE)
	{
		int type = def->type & ~(TYPE_INSTANCE);
		if (type == TYPE_INT)
			snprintf(buf, 128, "\"replace%d\" \"$%s %d\"", len, def->name, kv->value.i);
		if (type == TYPE_BOOL)
			snprintf(buf, 128, "\"replace%d\" \"$%s %d\"", len, def->name, (int)kv->value.b);
		if (type == TYPE_FLOAT)
			snprintf(buf, 128, "\"replace%d\" \"$%s %.2f\"", len, def->name, kv->value.f);
		if (type == TYPE_STRING)
			snprintf(buf, 128, "\"replace%d\" \"$%s %s\"", len, def->name, kv->value.s);
		if (type == TYPE_PICKER)
		{
			if (kv->value.i == -1)
				snprintf(buf, 128, "\"replace%d\" \"$%s (none)\"", len, def->name);
			else
			{
				Item* target = getItem(kv->value.i);
				snprintf(buf, 128, "\"replace%d\" \"$%s %s%d\"", len, def->name, target->def->name, target->index);
			}
		}
		if (type & TYPE_DROPDOWN)
		{
			type &= ~(TYPE_DROPDOWN);
			if (type == TYPE_STRING)
				snprintf(buf, 128, "\"replace%d\" \"$%s %s\"", len, def->name, kv->def->dropValues[kv->value.i].s);
			if (type == TYPE_INT)
				snprintf(buf, 128, "\"replace%d\" \"$%s %d\"", len, def->name, kv->def->dropValues[kv->value.i].i);
		}
	}
	else
	{
		if (def->type == TYPE_INT)
			snprintf(buf, 128, "\"%s\" \"%d\"", def->name, kv->value.i);
		if (def->type == TYPE_FLOAT)
			snprintf(buf, 128, "\"%s\" \"%.2f\"", def->name, kv->value.f);
		if (def->type == TYPE_BOOL)
			snprintf(buf, 128, "\"%s\" \"%d\"", def->name, (int)kv->value.b);
		if (def->type == TYPE_STRING)
			snprintf(buf, 128, "\"%s\" \"%s\"", def->name, kv->value.s);
		if (def->type & TYPE_DROPDOWN)
		{
			int type = (def->type & (~TYPE_DROPDOWN));
			if (type == TYPE_STRING)
				snprintf(buf, 128, "\"%s\" \"%s\"", def->name, kv->def->dropValues[kv->value.i].s);
			if (type == TYPE_INT)
				snprintf(buf, 128, "\"%s\" \"%d\"", def->name, kv->def->dropValues[kv->value.i].i);
		}
		if (def->type == TYPE_PICKER)
		{
			if (kv->value.i == -1)
				snprintf(buf, 128, "\"%s\" \"(none)\"", def->name);
			else
			{
				Item* target = getItem(kv->value.i);
				snprintf(buf, 128, "\"%s\" \"%s%d\"", def->name, target->def->name, target->index);
			}
		}
	}

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
	int* arr = ent->brushes;
	if (arr == 0)
		arr = dynList_new(1, sizeof(int));
	else
	{
		int len = dynList_size(arr);
		dynList_resize((void**)&arr, len + 1);
		i = len;
	}
	brush->ent = 1;
	arr[i] = brush->id;
	ent->brushes = arr;
}

void exportEntityAddRawOutput(Entity* ent, const char* output, const char* name, const char* input, const char* arg,
							  float delay)
{
	char buf[256];
	snprintf(buf, 256, "\"%s\" \"%s\x1b%s\x1b%s\x1b%.2f\x1b-1\"", output, name, input, arg, delay);
	int i = 0;
	const char** arr = ent->rawOutputs;
	if (arr == 0)
		arr = dynList_new(1, sizeof(const char*));
	else
	{
		int len = dynList_size(arr);
		dynList_resize((void**)&arr, len + 1);
		i = len;
	}
	arr[i] = strdup(buf);
	ent->rawOutputs = arr;
}

void exportEntitiesProcessOutputs()
{
	int len = dynList_size(entities);
	for (int i = 0; i < len; i++)
	{
		Entity* entity = &entities[i];
		if (entity->outputs == 0)
			continue;

		int outputCount = dynList_size(entity->outputs);
		if (outputCount == 0)
			continue;

		const char* entName = entity->name;

		int uniqueOutputs = 0;
		OutputDef** outputDefs = dynList_new(0, sizeof(OutputDef**));
		for (int j = 0; j < outputCount; j++)
		{
			ItemOutput* output = &entity->outputs[j];
			if (uniqueOutputs == 0)
			{
				dynList_resize((void*)&outputDefs, uniqueOutputs + 1);
				outputDefs[uniqueOutputs] = output->def;
				uniqueOutputs++;
				continue;
			}
			for (int k = 0; k < uniqueOutputs; k++)
			{
				if (outputDefs[k] == output->def)
					continue;
				dynList_resize((void*)&outputDefs, uniqueOutputs + 1);
				outputDefs[uniqueOutputs] = output->def;
				uniqueOutputs++;
			}
		}

		if (uniqueOutputs == 0)
			continue;

		int* relays = malloc(sizeof(int) * uniqueOutputs);

		vec3 entPos;
		entPos[0] = entity->pos[0];
		entPos[1] = entity->pos[1];
		entPos[2] = entity->pos[2];

		for (int j = 0; j < uniqueOutputs; j++)
		{
			OutputDef* output = outputDefs[j];
			char name[128];
			snprintf(name, 128, "%s_%s", entName, output->name);
			exportEntityAddRawOutput(entity, output->trueOutput, name, "FireUser1", "", 0);
			if (output->falseOutput)
				exportEntityAddRawOutput(entity, output->falseOutput, name, "FireUser2", "", 0);

			Entity* relay = exportCreateEntity();
			relay->className = "logic_relay";
			relay->name = strdup(name);
			relay->rotation[0] = 0;
			relay->rotation[1] = 0;
			relay->rotation[2] = 0;
			relay->pos[0] = entPos[0];
			relay->pos[1] = entPos[1];
			relay->pos[2] = entPos[2];
			relays[j] = relay->id;
		}

		ItemOutput* outputArr = entity->outputs;
		entity->outputs = 0;

		for (int j = 0; j < outputCount; j++)
		{
			ItemOutput* output = &outputArr[j];
			char relayName[128];
			snprintf(relayName, 128, "%s_%s", entName, output->def->name);

			Entity* relay = 0;

			for (int k = 0; k < uniqueOutputs; k++)
			{
				Entity* relay2 = &entities[relays[k]];
				if (strcmp(relay2->name, relayName) == 0)
				{
					relay = relay2;
					break;
				}
			}

			char ioEntName[128];
			Item* item = getItem(output->entity);
			InputDef* input = output->input;
			if (item->ioEnt)
				strncpy(ioEntName, item->ioEnt, 128);
			else
				snprintf(ioEntName, 128, "%s%d", item->def->name, output->entity);
			char antlineName[64];
			snprintf(antlineName, 64, "antline%d-tex", output->antline);
			if (output->antline != -1)
				exportEntityAddRawOutput(relay, "OnUser1", antlineName, "SetTextureIndex", "1", 0);
			if (output->inverted)
			{
				if (input->falseInput)
				{
					if (input->falseArg)
						exportEntityAddRawOutput(relay, "OnUser1", ioEntName, input->falseInput, input->falseArg, 0);
					else
						exportEntityAddRawOutput(relay, "OnUser1", ioEntName, input->falseInput, "", 0);
				}

				if (output->def->falseOutput)
				{
					if (input->trueArg)
						exportEntityAddRawOutput(relay, "OnUser2", ioEntName, input->trueInput, input->trueArg, 0);
					else
						exportEntityAddRawOutput(relay, "OnUser2", ioEntName, input->trueInput, "", 0);
					if (output->antline != -1)
						exportEntityAddRawOutput(relay, "OnUser2", antlineName, "SetTextureIndex", "0", 0);
				}
			}
			else
			{
				if (input->trueArg)
					exportEntityAddRawOutput(relay, "OnUser1", ioEntName, input->trueInput, input->trueArg, 0);
				else
					exportEntityAddRawOutput(relay, "OnUser1", ioEntName, input->trueInput, "", 0);

				if (input->falseInput || output->def->falseOutput)
				{
					if (input->falseArg)
						exportEntityAddRawOutput(relay, "OnUser2", ioEntName, input->falseInput, input->falseArg, 0);
					else
						exportEntityAddRawOutput(relay, "OnUser2", ioEntName, input->falseInput, "", 0);
					if (output->antline != -1)
						exportEntityAddRawOutput(relay, "OnUser2", antlineName, "SetTextureIndex", "0", 0);
				}
			}
		}
	}
}

void exportEndEntities(FILE* file)
{
	int len = dynList_size(entities);
	for (int i = 0; i < len; i++)
	{
		Entity* entity = &entities[i];

		fprintf(file, "entity\n{\n");

		fprintf(file, "  \"id\" \"%d\"\n", i + 1);
		fprintf(file, "  \"classname\" \"%s\"\n", entity->className);
		fprintf(file, "  \"origin\" \"%f %f %f\"\n", entity->pos[2] * 64, entity->pos[0] * 64, entity->pos[1] * 64);
		fprintf(file, "  \"angles\" \"%f %f %f\"\n", entity->rotation[0], entity->rotation[1], entity->rotation[2]);
		fprintf(file, "  \"targetname\" \"%s\"\n", entity->name);

		free((char*)entity->name);
		if (entity->script)
			free((char*)entity->className);

		char buf[100];

		if (entity->kvs)
		{
			int len = dynList_size(entity->kvs);
			for (int i = 0; i < len; i++)
			{
				fprintf(file, "  %s\n", entity->kvs[i]);
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
					InputDef* input = output->input;
					if (item->ioEnt)
						strncpy(buf, item->ioEnt, 100);
					else
						snprintf(buf, 100, "%s%d", item->def->name, output->entity);
					if (output->inverted)
					{
						if (input->falseInput)
						{
							if (input->falseArg)
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->trueOutput,
										buf, input->falseInput, input->falseArg);
							else
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
										input->falseInput);
						}

						if (output->def->falseOutput)
						{
							if (input->trueArg)
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->falseOutput,
										buf, input->trueInput, input->trueArg);
							else
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput,
										buf, input->trueInput);
						}
					}
					else
					{
						if (input->trueArg)
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									input->trueInput, input->trueArg);
						else
							fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->trueOutput, buf,
									input->trueInput);

						if (input->falseInput || output->def->falseOutput)
						{
							if (input->falseArg)
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b%s\x1b 0\x1b-1\"\n", output->def->falseOutput,
										buf, input->falseInput, input->falseArg);
							else
								fprintf(file, "    \"%s\" \"%s\x1b%s\x1b\x1b 0\x1b-1\"\n", output->def->falseOutput,
										buf, input->falseInput);
						}
					}
				}
				fprintf(file, "  }\n");
			}
		}

		if (entity->rawOutputs)
		{
			int outputLen = dynList_size(entity->rawOutputs);
			if (outputLen > 0)
			{
				fprintf(file, "\n  connections\n  {\n");
				for (int i = 0; i < outputLen; i++)
				{
					char* output = (char*)entity->rawOutputs[i];
					fprintf(file, "    %s\n", output);
					free(output);
				}
				fprintf(file, "  }\n");
				dynList_free(entity->rawOutputs);
			}
		}

		if (entity->brushes)
		{
			Brush* brushes = getBrushArray();
			int l = dynList_size(entity->brushes);
			for (int j = 0; j < l; j++)
			{
				Brush* b = &brushes[entity->brushes[j]];
				exportBrush(file, b);
			}
			dynList_free(entity->brushes);
		}

		fprintf(file, "}\n");
	}
}
