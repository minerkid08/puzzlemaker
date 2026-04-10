#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "item.h"

#include "dynList.h"
#include "cimgui.h"

#include <string.h>

const char** itemNames;

void initItemPanel()
{
  ItemDefinition* defs = getItemDefinitions();

  int s = dynList_size(defs);

  int len = 0;
  for(int i = 0; i < s; i++)
  {
    len += strlen(defs->name);
    len++;
  }
  len++;

  addLabel = malloc(len);
  len = 0;
  for(int i = 0; i < s; i++)
  {
    strcpy(addLabel + len, defs->name);
    len += strlen(defs->name);
    addLabel[len] = 0;
    len++;
  }
  addLabel[len] = 0;
}

void itemPanelRender()
{
  igShowDemoWindow(0);
}
