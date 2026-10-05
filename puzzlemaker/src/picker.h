#pragma once

#include "antline/antline.h"
#include "item/item.h"

#define PICKER_INACTIVE 0
#define PICKER_ITEM 1
#define PICKER_ANTLINE 2

typedef struct
{
  char active;
  Item** ptr;
  Antline** ant;
} Picker;
