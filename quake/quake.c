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

static void WriteFlatPlane(FILE *fp, float_g z, const char *flat_name, int dir,
                           subsec_t *sub, sector_t *sector)
{
  float_g A = (dir < 0) ? 1 : 0;
  float_g B = 1 - A;
  
  fprintf(fp, "    ( %1.2f %1.2f %1.2f )", 0.0, 0.0, z);
  fprintf(fp,    " ( %1.2f %1.2f %1.2f )", A, B, z);
  fprintf(fp,    " ( %1.2f %1.2f %1.2f )", B, A, z);
  
  fprintf(fp, " ( ( %1.2f %1.2f %1.2f )",   1.0, 0.0, 0.0);
  fprintf(fp,   " ( %1.2f %1.2f %1.2f ) )", 0.0, 1.0, 0.0);

  fprintf(fp, " %s 0 %d 0\n", flat_name, (strcmp(flat_name, "void")==0) ? 4 : 0);
}

static void ShiftVertex(subsec_t *sub, float_g *x, float_g *y)
{
  float_g dx = *x - sub->mid_x;
  float_g dy = *y - sub->mid_y;

  float_g len = sqrt(dx*dx + dy*dy);
  
  // this shouldn't happen, but just in case we avoid the div-by-zero
  if (len < 0.1)
    return;

  (*x) += 0.5 * dx / len;
  (*y) += 0.5 * dy / len;
}

static void WriteWallPlane(FILE *fp, seg_t *seg, seg_t *seg2,
                           float_g z1, float_g z2, const char *backup_tex,
                           subsec_t *sub, sector_t *sector)
{
  float_g x1 = seg->start->x;
  float_g y1 = seg->start->y;
  float_g x2 = seg->end->x;
  float_g y2 = seg->end->y;

  // when writing a quake brush, most vertices are shifted out a
  // little bit so that brushes intersect (rather than touch,
  // which leads to leaks).

  ShiftVertex(sub, &x1, &y1);
  ShiftVertex(sub, &x2, &y2);

  fprintf(fp, "    ( %1.2f %1.2f %1.2f )", x1, y1, z1);
  fprintf(fp,    " ( %1.2f %1.2f %1.2f )", x2, y2, z1);
  fprintf(fp,    " ( %1.2f %1.2f %1.2f )", x1, y1, z2);

  fprintf(fp, " ( ( %1.2f %1.2f %1.2f )",   1.0, 0.1, 0.4);
  fprintf(fp,   " ( %1.2f %1.2f %1.2f ) )", 0.2, 1.0, 0.6);

  const char *tex_name = "metal"; //!!!!! FIXME

  if (! seg->linedef)
    tex_name = "void";

  fprintf(fp, " %s 0 %d 0\n", tex_name, (strcmp(tex_name, "void")==0) ? 4 : 0);
}

static void WriteSubsec_Q3A(FILE *fp, subsec_t *sub)
{
  int pass;
  int is_void;

  if (sub->seg_count <= 2)
    return;
  
  sector_t *sector = sub->seg_list->sector;
  if (! sector)
    sector = LookupSector(0);

  is_void = (sector->ceil_h < -29000) ? TRUE : FALSE;

  // first pass  : floors or void space
  // second pass : ceilings
  for (pass = 0; pass < (is_void ? 1 : 2); pass++)
  {
    seg_t *seg;
    float_g bottom, top;
    const char *flat_name;

    fprintf(fp, " {\n");
    fprintf(fp, "brushDef\n");
    fprintf(fp, "  {\n");

    if (is_void)
    {
      bottom = -30000.0;
      top = +30000.0;
      flat_name = "void";
    }
    else if (pass == 0)
    {
      bottom = -30000.0;
      top = sector->floor_h;
      flat_name = sector->floor_tex;
flat_name = "metal";
    }
    else // pass == 1
    {
      bottom = sector->ceil_h - 0.5;
      top = +30000.0;
      
      flat_name = sector->ceil_tex;
flat_name = "metal"; //!!!!!
    }

    WriteFlatPlane(fp, bottom, flat_name, -1, sub, sector);
    WriteFlatPlane(fp, top,    flat_name, +1, sub, sector);

    for (seg = sub->seg_list; seg; seg = seg->next)
    {
      WriteWallPlane(fp, seg, seg->next ? seg->next : sub->seg_list,
                     bottom, top, flat_name, sub, sector);
    }

    fprintf(fp, "  }\n");
    fprintf(fp, " }\n");
  }
}


//
// WriteMap_Q3A
//
void WriteMap_Q3A(const char *filename, node_t *root)
{
  int i;

  FILE *fp = fopen(filename, "w");
  if (! fp)
    FatalError("Failed to create MAP file: %s\n", filename);
  
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

  fprintf(fp, "}\n");
  fprintf(fp, "// Entities\n");

  for (i=0; i < num_things; i++)
  {
    thing_t *th = LookupThing(i);

    WriteThing_Q3A(fp, th);
  }

  fclose(fp);
}

