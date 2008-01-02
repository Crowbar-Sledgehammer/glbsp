//------------------------------------------------------------------------
// QUAKE Output (".map" files)
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2007 Andrew Apted
//
//  Based on 'BSP 2.3' by Colin Reed, Lee Killough and others.
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

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "analyze.h"
#include "blockmap.h"
#include "level.h"
#include "quake.h"
#include "seg.h"
#include "structs.h"
#include "util.h"
#include "wad.h"


static void WriteKey(FILE *fp, const char *key, const char *val)
{
  fprintf(fp, " \"%s\" \"%s\"\n", key, val);
}

static void WriteNumber(FILE *fp, const char *key, int val)
{
  fprintf(fp, " \"%s\" \"%d\"\n", key, val);
}

static void WriteTriplet(FILE *fp, const char *key, float_g x, float_g y, float_g z)
{
  fprintf(fp, " \"%s\" \"%1.0f %1.0f %1.0f\"\n", key, x, y, z);
}

static void WriteThing_Q3A(FILE *fp, thing_t *th)
{
  // player?
  if (th->type == 1 || th->type == 11)
  {
    sector_t *sec = ThingFindSector(th);

    fprintf(fp, "{\n");

    WriteKey(fp, "classname", "info_player_deathmatch");
    WriteNumber(fp, "angle", th->angle);
    WriteTriplet(fp, "origin", th->x, th->y, sec->floor_h + 1 + th->dz);

    fprintf(fp, "}\n");
  }
}

static void WriteSubsec_Q3A(FILE *fp, subsec_t *sub)
{
  // TODO
}


//
// WriteMap_Q3A
//
void WriteMap_Q3A(FILE *fp, node_t *root)
{
  int i;

  fprintf(fp, "// converted by AJDMQK (AJ's Doom to Quake program)\n");
  fprintf(fp, "{\n");

  WriteKey(fp, "classname", "worldspawn");
  WriteKey(fp, "message", "an AJDMQK conversion");
  WriteNumber(fp, "ambient", 90);

  for (i=0; i < num_subsecs; i++)
  {
    subsec_t *sub = LookupSubsec(i);

    WriteSubsec_Q3A(fp, sub);
  }

  for (i=0; i < num_things; i++)
  {
    thing_t *th = LookupThing(i);

    WriteThing_Q3A(fp, th);
  }

  fprintf(fp, "}\n");
}

