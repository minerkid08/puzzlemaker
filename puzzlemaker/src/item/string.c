#include "dynList.h"
#include "item/item.h"
#include "utils.h"
#include <string.h>

char* itemProcessString(const char* str, ItemKv* kvs)
{
	char buf[512];
  memset(buf, 0, 512);
	int len = strlen(str);
	int k = 0;
	char mode = 0;
	for (int i = 0; i < len; i++)
	{
		char c = str[i];
		if (c == '{')
			mode = 1;
		if (c == '[')
			mode = 2;

		if (mode)
		{
			int l = dynList_size(kvs);
			for (int j = 0; j < l; j++)
			{
				const char* b = kvs[j].def->name;
				if (startsWith(str + i + 1, b))
				{
					i += strlen(b);
					ItemKv* kv = &kvs[j];
					int t = kv->def->type;
					t &= ~(TYPE_INSTANCE);
					if (t == TYPE_INT)
						k += strncatf(buf, 512, "%d", kv->value.i);
					if (t == TYPE_FLOAT)
						k += strncatf(buf, 512, "%f", kv->value.f);
					if (t == TYPE_BOOL)
						k += strncatf(buf, 512, "%d", kv->value.b);
					if (t == TYPE_STRING)
						k += strncatf(buf, 512, "%s", kv->value.s);
					if (t == TYPE_BITFIELD)
					{
						int value = 0;
						int len = dynList_size(kv->def->dropNames);
						for (int j = 0; j < len; j++)
						{
							if (kv->value.s[j])
								value |= 1 << kv->def->dropValues[j].i;
						}
						k += strncatf(buf, 512, "%d", value);
					}
					if (t == TYPE_PICKER)
					{
						if (kv->value.i == -1)
							k += strncatf(buf, 512, "(none)");
						else
						{
							Item* target = getItem(kv->value.i);
							k += strncatf(buf, 512, "%s%d", target->def->name, target->index);
						}
					}
					if (t & TYPE_DROPDOWN)
					{
						V* values = kv->def->dropValues;
						int v = kv->value.i;
						if (mode == 1)
						{
							if (t == (TYPE_DROPDOWN | TYPE_STRING))
								k += strncatf(buf, 512, "%s", values[v].s);
							if (t == (TYPE_DROPDOWN | TYPE_INT))
								k += strncatf(buf, 512, "%d", values[v].i);
						}
						else
							k += strncatf(buf, 512, "%s", kv->def->dropNames[v]);
					}
					i++;
					break;
				}
			}
			mode = 0;
		}
		else
			buf[k++] = c;
	}
	buf[k] = 0;
	return strdup(buf);
}
