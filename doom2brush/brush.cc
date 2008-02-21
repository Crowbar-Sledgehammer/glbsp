//------------------------------------------------------------------------
//  BRUSH Creation
//------------------------------------------------------------------------
//
//  Doom-2-Brush (C) 2008 Andrew Apted
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//------------------------------------------------------------------------

// this includes everything we need
#include "defs.h"


void Brush_ConvertSectors(void)
{
  // TODO
}

void Brush_ConvertExtraFloors(void)
{
  // TODO
}

void Brush_ConvertLiquids(void)
{
  // TODO
}

void Brush_ConvertWalls(void)
{
  // TODO
}


//------------------------------------------------------------------------

void Brush_WriteField(const char *field, const char *val_str, ...)
{
  fprintf(map_fp, "  \"%s\"  \"", field);

  va_list args;

  va_start(args, val_str);
  vfprintf(map_fp, val_str, args);
  va_end(args);

  fprintf(map_fp, "\"\n");
}

void Brush_ConvertThings(void)
{
  for (int i = 0; i < lev_things.num; i++)
  {
    thing_c *T = lev_things.Get(i);

    // FIXME: thing --> entity conversion
    if (T->type != 1)
      continue;

    int z = 128;  // FIXME !!!!

    fprintf(map_fp, "// thing #%d type:%d\n", i, T->type);
    fprintf(map_fp, "{\n");

    Brush_WriteField("classname", "info_player_start");
    Brush_WriteField("origin", "%d %d %d", T->x, T->y, z);

    // FIXME: Brush_WriteField("angle", "%d", T->angle);

    fprintf(map_fp, "}\n");
  }
}

