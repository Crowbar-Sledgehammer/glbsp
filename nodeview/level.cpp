//------------------------------------------------------------------------
// LEVEL : Level structure read/write functions.
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2000-2004 Andrew Apted
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

///---#include "system.h"
///---
///---#include <stdio.h>
///---#include <stdlib.h>
///---#include <string.h>
///---#include <stdarg.h>
///---#include <ctype.h>
///---#include <math.h>
///---#include <limits.h>
///---#include <assert.h>
///---
///---#include "analyze.h"
///---#include "blockmap.h"
///---#include "level.h"
///---#include "node.h"
///---#include "reject.h"
///---#include "seg.h"
///---#include "structs.h"
///---#include "util.h"
///---#include "wad.h"


#define DEBUG_LOAD      0
#define DEBUG_BSP       0

#define ALLOC_BLKNUM  1024


// per-level variables

bool lev_doing_normal;
bool lev_doing_hexen;

static bool lev_v3_segs;
static bool lev_v3_subsecs;


#define LEVELARRAY(TYPE, BASEVAR, NUMVAR)  \
    TYPE ** BASEVAR = NULL;  \
    int NUMVAR = 0;


LEVELARRAY(vertex_t,  lev_vertices,   num_vertices)
LEVELARRAY(linedef_t, lev_linedefs,   num_linedefs)
LEVELARRAY(sidedef_t, lev_sidedefs,   num_sidedefs)
LEVELARRAY(sector_t,  lev_sectors,    num_sectors)
LEVELARRAY(thing_t,   lev_things,     num_things)

static LEVELARRAY(seg_t,     segs,       num_segs)
static LEVELARRAY(subsec_t,  subsecs,    num_subsecs)
static LEVELARRAY(node_t,    nodes,      num_nodes)
static LEVELARRAY(node_t,    stale_nodes,num_stale_nodes)


int num_normal_vert = 0;
int num_gl_vert = 0;
int num_complete_seg = 0;


/* ----- allocation routines ---------------------------- */

#define ALLIGATOR(TYPE, BASEVAR, NUMVAR)  \
{  \
  if ((NUMVAR % ALLOC_BLKNUM) == 0)  \
  {  \
    BASEVAR = (TYPE **) UtilRealloc(BASEVAR, (NUMVAR + ALLOC_BLKNUM) *   \
        sizeof(TYPE *));  \
  }  \
  BASEVAR[NUMVAR] = (TYPE *) UtilCalloc(sizeof(TYPE));  \
  NUMVAR += 1;  \
  return BASEVAR[NUMVAR - 1];  \
}


vertex_t *NewVertex(void)
  ALLIGATOR(vertex_t, lev_vertices, num_vertices)

linedef_t *NewLinedef(void)
  ALLIGATOR(linedef_t, lev_linedefs, num_linedefs)

sidedef_t *NewSidedef(void)
  ALLIGATOR(sidedef_t, lev_sidedefs, num_sidedefs)

sector_t *NewSector(void)
  ALLIGATOR(sector_t, lev_sectors, num_sectors)

thing_t *NewThing(void)
  ALLIGATOR(thing_t, lev_things, num_things)

seg_t *NewSeg(void)
  ALLIGATOR(seg_t, segs, num_segs)

subsec_t *NewSubsec(void)
  ALLIGATOR(subsec_t, subsecs, num_subsecs)

node_t *NewNode(void)
  ALLIGATOR(node_t, nodes, num_nodes)

node_t *NewStaleNode(void)
  ALLIGATOR(node_t, stale_nodes, num_stale_nodes)


/* ----- free routines ---------------------------- */

#define FREEMASON(TYPE, BASEVAR, NUMVAR)  \
{  \
  int i;  \
  for (i=0; i < NUMVAR; i++)  \
    UtilFree(BASEVAR[i]);  \
  if (BASEVAR)  \
    UtilFree(BASEVAR);  \
  BASEVAR = NULL; NUMVAR = 0;  \
}


void FreeVertices(void)
  FREEMASON(vertex_t, lev_vertices, num_vertices)

void FreeLinedefs(void)
  FREEMASON(linedef_t, lev_linedefs, num_linedefs)

void FreeSidedefs(void)
  FREEMASON(sidedef_t, lev_sidedefs, num_sidedefs)

void FreeSectors(void)
  FREEMASON(sector_t, lev_sectors, num_sectors)

void FreeThings(void)
  FREEMASON(thing_t, lev_things, num_things)

void FreeSegs(void)
  FREEMASON(seg_t, segs, num_segs)

void FreeSubsecs(void)
  FREEMASON(subsec_t, subsecs, num_subsecs)

void FreeNodes(void)
  FREEMASON(node_t, nodes, num_nodes)

void FreeStaleNodes(void)
  FREEMASON(node_t, stale_nodes, num_stale_nodes)


/* ----- lookup routines ------------------------------ */

#define LOOKERUPPER(BASEVAR, NUMVAR, NAMESTR)  \
{  \
  if (index < 0 || index >= NUMVAR)  \
    FatalError("No such %s number #%d", NAMESTR, index);  \
    \
  return BASEVAR[index];  \
}

vertex_t *LookupVertex(int index)
  LOOKERUPPER(lev_vertices, num_vertices, "vertex")

linedef_t *LookupLinedef(int index)
  LOOKERUPPER(lev_linedefs, num_linedefs, "linedef")
  
sidedef_t *LookupSidedef(int index)
  LOOKERUPPER(lev_sidedefs, num_sidedefs, "sidedef")
  
sector_t *LookupSector(int index)
  LOOKERUPPER(lev_sectors, num_sectors, "sector")
  
thing_t *LookupThing(int index)
  LOOKERUPPER(lev_things, num_things, "thing")
  
seg_t *LookupSeg(int index)
  LOOKERUPPER(segs, num_segs, "seg")
  
subsec_t *LookupSubsec(int index)
  LOOKERUPPER(subsecs, num_subsecs, "subsector")
  
node_t *LookupNode(int index)
  LOOKERUPPER(nodes, num_nodes, "node")

node_t *LookupStaleNode(int index)
  LOOKERUPPER(stale_nodes, num_stale_nodes, "stale_node")


/* ----- reading routines ------------------------------ */


//
// CheckForNormalNodes
//
bool CheckForNormalNodes(void)
{
  lump_t *lump;
  
  /* Note: an empty NODES lump can be valid */
  if (FindLevelLump("NODES") == NULL)
    return false;
 
  lump = FindLevelLump("SEGS");
  
  if (! lump || lump->length == 0 || CheckLevelLumpZero(lump))
    return false;

  lump = FindLevelLump("SSECTORS");
  
  if (! lump || lump->length == 0 || CheckLevelLumpZero(lump))
    return false;

  return true;
}

//
// GetVertices
//
void GetVertices(void)
{
  int i, count=-1;
  raw_vertex_t *raw;
  lump_t *lump = FindLevelLump("VERTEXES");

  if (lump)
    count = lump->length / sizeof(raw_vertex_t);

# if DEBUG_LOAD
  PrintDebug("GetVertices: num = %d\n", count);
# endif

  if (!lump || count == 0)
    FatalError("Couldn't find any Vertices");

  raw = (raw_vertex_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    vertex_t *vert = NewVertex();

    vert->x = (double) SINT16(raw->x);
    vert->y = (double) SINT16(raw->y);

    vert->index = i;
  }

  num_normal_vert = num_vertices;
  num_gl_vert = 0;
  num_complete_seg = 0;
}

//
// GetSectors
//
void GetSectors(void)
{
  int i, count=-1;
  raw_sector_t *raw;
  lump_t *lump = FindLevelLump("SECTORS");

  if (lump)
    count = lump->length / sizeof(raw_sector_t);

  if (!lump || count == 0)
    FatalError("Couldn't find any Sectors");

# if DEBUG_LOAD
  PrintDebug("GetSectors: num = %d\n", count);
# endif

  raw = (raw_sector_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    sector_t *sector = NewSector();

    sector->floor_h = SINT16(raw->floor_h);
    sector->ceil_h  = SINT16(raw->ceil_h);

    memcpy(sector->floor_tex, raw->floor_tex, sizeof(sector->floor_tex));
    memcpy(sector->ceil_tex,  raw->ceil_tex,  sizeof(sector->ceil_tex));

    sector->light = UINT16(raw->light);
    sector->special = UINT16(raw->special);
    sector->tag = SINT16(raw->tag);

    sector->coalesce = (sector->tag >= 900 && sector->tag < 1000) ?
        true : false;

    /* sector indices never change */
    sector->index = i;

    /* Note: rej_* fields are handled completely in reject.c */
  }
}

//
// GetThings
//
void GetThings(void)
{
  int i, count=-1;
  raw_thing_t *raw;
  lump_t *lump = FindLevelLump("THINGS");

  if (lump)
    count = lump->length / sizeof(raw_thing_t);

  if (!lump || count == 0)
  {
    // Note: no error if no things exist, even though technically a map
    // will be unplayable without the player starts.
    PrintWarn("Couldn't find any Things");
    return;
  }

# if DEBUG_LOAD
  PrintDebug("GetThings: num = %d\n", count);
# endif

  raw = (raw_thing_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    thing_t *thing = NewThing();

    thing->x = SINT16(raw->x);
    thing->y = SINT16(raw->y);

    thing->type = UINT16(raw->type);
    thing->options = UINT16(raw->options);

    thing->index = i;
  }
}

//
// GetThingsHexen
//
void GetThingsHexen(void)
{
  int i, count=-1;
  raw_hexen_thing_t *raw;
  lump_t *lump = FindLevelLump("THINGS");

  if (lump)
    count = lump->length / sizeof(raw_hexen_thing_t);

  if (!lump || count == 0)
  {
    // Note: no error if no things exist, even though technically a map
    // will be unplayable without the player starts.
    PrintWarn("Couldn't find any Things");
    return;
  }

# if DEBUG_LOAD
  PrintDebug("GetThingsHexen: num = %d\n", count);
# endif

  raw = (raw_hexen_thing_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    thing_t *thing = NewThing();

    thing->x = SINT16(raw->x);
    thing->y = SINT16(raw->y);

    thing->type = UINT16(raw->type);
    thing->options = UINT16(raw->options);

    thing->index = i;
  }
}

//
// GetSidedefs
//
void GetSidedefs(void)
{
  int i, count=-1;
  raw_sidedef_t *raw;
  lump_t *lump = FindLevelLump("SIDEDEFS");

  if (lump)
    count = lump->length / sizeof(raw_sidedef_t);

  if (!lump || count == 0)
    FatalError("Couldn't find any Sidedefs");

# if DEBUG_LOAD
  PrintDebug("GetSidedefs: num = %d\n", count);
# endif

  raw = (raw_sidedef_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    sidedef_t *side = NewSidedef();

    side->sector = (SINT16(raw->sector) == -1) ? NULL :
        LookupSector(UINT16(raw->sector));

    if (side->sector)
      side->sector->ref_count++;

    side->x_offset = SINT16(raw->x_offset);
    side->y_offset = SINT16(raw->y_offset);

    memcpy(side->upper_tex, raw->upper_tex, sizeof(side->upper_tex));
    memcpy(side->lower_tex, raw->lower_tex, sizeof(side->lower_tex));
    memcpy(side->mid_tex,   raw->mid_tex,   sizeof(side->mid_tex));

    /* sidedef indices never change */
    side->index = i;
  }
}

static sidedef_t *SafeLookupSidedef(uint16_g num)
{
  if (num == 0xFFFF)
    return NULL;

  if ((int)num >= num_sidedefs && (sint16_g)(num) < 0)
    return NULL;

  return LookupSidedef(num);
}

//
// GetLinedefs
//
void GetLinedefs(void)
{
  int i, count=-1;
  raw_linedef_t *raw;
  lump_t *lump = FindLevelLump("LINEDEFS");

  if (lump)
    count = lump->length / sizeof(raw_linedef_t);

  if (!lump || count == 0)
    FatalError("Couldn't find any Linedefs");

# if DEBUG_LOAD
  PrintDebug("GetLinedefs: num = %d\n", count);
# endif

  raw = (raw_linedef_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    linedef_t *line;

    vertex_t *start = LookupVertex(UINT16(raw->start));
    vertex_t *end   = LookupVertex(UINT16(raw->end));

    start->ref_count++;
    end->ref_count++;

    line = NewLinedef();

    line->start = start;
    line->end   = end;

    /* check for zero-length line */
    line->zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && 
        (fabs(start->y - end->y) < DIST_EPSILON);

    line->flags = UINT16(raw->flags);
    line->type = UINT16(raw->type);
    line->tag  = SINT16(raw->tag);

    line->two_sided = (line->flags & LINEFLAG_TWO_SIDED) ? true : false;
    line->is_precious = (line->tag >= 900 && line->tag < 1000) ? 
        true : false;

    line->right = SafeLookupSidedef(UINT16(raw->sidedef1));
    line->left  = SafeLookupSidedef(UINT16(raw->sidedef2));

    if (line->right)
    {
      line->right->ref_count++;
      line->right->on_special |= (line->type > 0) ? 1 : 0;
    }

    if (line->left)
    {
      line->left->ref_count++;
      line->left->on_special |= (line->type > 0) ? 1 : 0;
    }

    line->index = i;
  }
}

//
// GetLinedefsHexen
//
void GetLinedefsHexen(void)
{
  int i, j, count=-1;
  raw_hexen_linedef_t *raw;
  lump_t *lump = FindLevelLump("LINEDEFS");

  if (lump)
    count = lump->length / sizeof(raw_hexen_linedef_t);

  if (!lump || count == 0)
    FatalError("Couldn't find any Linedefs");

# if DEBUG_LOAD
  PrintDebug("GetLinedefsHexen: num = %d\n", count);
# endif

  raw = (raw_hexen_linedef_t *) lump->data;

  for (i=0; i < count; i++, raw++)
  {
    linedef_t *line;

    vertex_t *start = LookupVertex(UINT16(raw->start));
    vertex_t *end   = LookupVertex(UINT16(raw->end));

    start->ref_count++;
    end->ref_count++;

    line = NewLinedef();

    line->start = start;
    line->end   = end;

    // check for zero-length line
    line->zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && 
        (fabs(start->y - end->y) < DIST_EPSILON);

    line->flags = UINT16(raw->flags);
    line->type = UINT8(raw->type);
    line->tag  = 0;

    /* read specials */
    for (j=0; j < 5; j++)
      line->specials[j] = UINT8(raw->specials[j]);

    // -JL- Added missing twosided flag handling that caused a broken reject
    line->two_sided = (line->flags & LINEFLAG_TWO_SIDED) ? true : false;

    line->right = SafeLookupSidedef(UINT16(raw->sidedef1));
    line->left  = SafeLookupSidedef(UINT16(raw->sidedef2));

    // -JL- Added missing sidedef handling that caused all sidedefs to be
    //      pruned.
    if (line->right)
    {
      line->right->ref_count++;
      line->right->on_special |= (line->type > 0) ? 1 : 0;
    }

    if (line->left)
    {
      line->left->ref_count++;
      line->left->on_special |= (line->type > 0) ? 1 : 0;
    }

    line->index = i;
  }
}

//
// GetStaleNodes
//
void GetStaleNodes(void)
{
  int i, count=-1;
  raw_node_t *raw;
  lump_t *lump = FindLevelLump("NODES");

  if (lump)
    count = lump->length / sizeof(raw_node_t);

  if (!lump || count < 5)
    return;

# if DEBUG_LOAD
  PrintDebug("GetStaleNodes: num = %d\n", count);
# endif

  raw = (raw_node_t *) lump->data;

  /* must allocate all the nodes beforehand, since they contain
   * internal references to each other.
   */
  for (i=0; i < count; i++)
  {
    NewStaleNode();
  }

  for (i=0; i < count; i++, raw++)
  {
    node_t *nd = LookupStaleNode(i);

    nd->x  = SINT16(raw->x);
    nd->y  = SINT16(raw->y);
    nd->dx = SINT16(raw->dx);
    nd->dy = SINT16(raw->dy);

    nd->index = i;
    
    /* Note: we ignore the subsector references */
     
    if ((UINT16(raw->right) & 0x8000) == 0)
    {
      nd->r.node = LookupStaleNode(UINT16(raw->right));
    }

    if ((UINT16(raw->left) & 0x8000) == 0)
    {
      nd->l.node = LookupStaleNode(UINT16(raw->left));
    }

    /* we also ignore the bounding boxes -- not needed */
  }
}


/* ----- writing routines ------------------------------ */

static const uint8_g *lev_v2_magic = (uint8_g *)"gNd2";
static const uint8_g *lev_v3_magic = (uint8_g *)"gNd3";


/* ----- whole-level routines --------------------------- */

//
// LoadLevel
//
void LoadLevel(void)
{
  char message[256];

  const char *level_name = GetLevelName();

  bool normal_exists = CheckForNormalNodes();

  lev_doing_normal = false;

  // -JL- Identify Hexen mode by presence of BEHAVIOR lump
  lev_doing_hexen = (FindLevelLump("BEHAVIOR") != NULL);

  lev_v3_segs = false;
  lev_v3_subsecs = lev_v3_segs;

  if (lev_doing_normal)
    sprintf(message, "Building normal and GL nodes on %s", level_name);
  else
    sprintf(message, "Building GL nodes on %s", level_name);
 
  if (lev_doing_hexen)
    strcat(message, " (Hexen)");

///---  DisplaySetBarText(1, message);

  PrintVerbose("\n\n");
  PrintMsg("%s\n", message);
  PrintVerbose("\n");

  GetVertices();
  GetSectors();
  GetSidedefs();

  if (lev_doing_hexen)
  {
    GetLinedefsHexen();
    GetThingsHexen();
  }
  else
  {
    GetLinedefs();
    GetThings();
  }

  PrintVerbose("Loaded %d vertices, %d sectors, %d sides, %d lines, %d things\n", 
      num_vertices, num_sectors, num_sidedefs, num_linedefs, num_things);

  GetStaleNodes();
}

//
// FreeLevel
//
void FreeLevel(void)
{
  FreeVertices();
  FreeSidedefs();
  FreeLinedefs();
  FreeSectors();
  FreeThings();
  FreeSegs();
  FreeSubsecs();
  FreeNodes();
  FreeStaleNodes();
}

