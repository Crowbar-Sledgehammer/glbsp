//------------------------------------------------------------------------
//  LEVEL : Level structure read/write functions.
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

#define DEBUG_LOAD      0
#define DEBUG_BSP       0


// per-level variables

bool lev_doing_hexen;


#define LEVELARRAY(TYPE, BASEVAR, NAMESTR)  \
    container_tp<TYPE> BASEVAR(NAMESTR);

LEVELARRAY(vertex_c,  lev_vertices, "vertex")
LEVELARRAY(vertex_c,  lev_gl_verts, "gl_vert")
LEVELARRAY(linedef_c, lev_linedefs, "linedef")
LEVELARRAY(sidedef_c, lev_sidedefs, "sidedef")
LEVELARRAY(sector_c,  lev_sectors,  "sector")
LEVELARRAY(thing_c,   lev_things,   "thing")
LEVELARRAY(seg_c,     lev_segs,     "seg")
LEVELARRAY(subsec_c,  lev_subsecs,  "subsector")
LEVELARRAY(node_c,    lev_nodes,    "node")



double slope_c::HeightAt(double x, double y)
{
  double dx = ex - sx;
  double dy = ey - sy;
  double dz = ez - sz;

  double d_len = dx*dx + dy*dy;

  double along = ((x - sx) * dx + (y - sy) * dy) / d_len;

  return sz + along * dz;
}


/* ----- reading routines ------------------------------ */

static const uint8_g *lev_v2_magic = (uint8_g *)"gNd2";
static const uint8_g *lev_v3_magic = (uint8_g *)"gNd3";

// forward decls
void GetLinedefsHexen(wad_c *base);
void GetThingsHexen(wad_c *base);

vertex_c::vertex_c(int _idx, const raw_vertex_t *raw) : lines()
{
  index = _idx;

  x = (double) SINT16(raw->x);
  y = (double) SINT16(raw->y);
}

vertex_c::vertex_c(int _idx, const raw_v2_vertex_t *raw)
{
  index = _idx;

  x = (double) SINT32(raw->x) / 65536.0;
  y = (double) SINT32(raw->y) / 65536.0;
}

vertex_c::~vertex_c()
{
}


void GetVertices(wad_c *base)
{
  lump_c *lump = base->FindLumpInLevel("VERTEXES");
  int count = -1;

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_vertex_t);
  }

# if DEBUG_LOAD
  PrintDebug("GetVertices: num = %d\n", count);
# endif

  if (!lump || count == 0)
    FatalError("Couldn't find any Vertices");

  lev_vertices.Allocate(count);

  raw_vertex_t *raw = (raw_vertex_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_vertices.Set(i, new vertex_c(i, raw));
  }
}


void GetV2Verts(const uint8_g *data, int length)
{
  int count = length / sizeof(raw_v2_vertex_t);

  lev_gl_verts.Allocate(count);

  raw_v2_vertex_t *raw = (raw_v2_vertex_t *) data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_gl_verts.Set(i, new vertex_c(i, raw));
  }
}


void GetGLVerts(wad_c *base)
{
  lump_c *lump = base->FindLumpInLevel("GL_VERT");

# if DEBUG_LOAD
  PrintDebug("GetVertices: num = %d\n", count);
# endif

  if (!lump)
    FatalError("Couldn't find any GL Vertices");

  base->CacheLump(lump);

  if (lump->length >= 4 && memcmp(lump->data, lev_v2_magic, 4) == 0)
  {
    GetV2Verts((uint8_g *)lump->data + 4, lump->length - 4);
    return;
  }

  int count = lump->length / sizeof(raw_vertex_t);

  lev_gl_verts.Allocate(count);

  raw_vertex_t *raw = (raw_vertex_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_gl_verts.Set(i, new vertex_c(i, raw));
  }
}

sector_c::sector_c(int _idx, const raw_sector_t *raw) :
            extrafloors(), ef_lines(), liquids()
{
  index = _idx;

  floor_h = SINT16(raw->floor_h);
  ceil_h  = SINT16(raw->ceil_h);

  // these are updated by FindSectorExtents
  floor_under = floor_h;
  ceil_over   = ceil_h;

  memcpy(floor_tex, raw->floor_tex, sizeof(floor_tex));
  memcpy(ceil_tex,  raw->ceil_tex,  sizeof(ceil_tex));

  // ensure NUL terminated
  floor_tex[8] = ceil_tex[8] = 0;

  light   = UINT16(raw->light);
  special = UINT16(raw->special);
  tag     = SINT16(raw->tag);

  floor_slope = ceil_slope = NULL;
}

sector_c::~sector_c()
{
}


void GetSectors(wad_c *base)
{
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("SECTORS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_sector_t);
  }

  if (!lump || count == 0)
    FatalError("Couldn't find any Sectors");

# if DEBUG_LOAD
  PrintDebug("GetSectors: num = %d\n", count);
# endif

  lev_sectors.Allocate(count);

  raw_sector_t *raw = (raw_sector_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_sectors.Set(i, new sector_c(i, raw));
  }
}

thing_c::thing_c(int _idx, const raw_thing_t *raw)
{
  index = _idx;

  x = SINT16(raw->x);
  y = SINT16(raw->y);

  type    = UINT16(raw->type);
  options = UINT16(raw->options);

  angle   = SINT16(raw->angle);
  height  = 0;
}

thing_c::thing_c(int _idx, const raw_hexen_thing_t *raw)
{
  index = _idx;

  x = SINT16(raw->x);
  y = SINT16(raw->y);

  type    = UINT16(raw->type);
  options = UINT16(raw->options);

  angle   = SINT16(raw->angle);
  height  = SINT16(raw->height);
}

thing_c::~thing_c()
{
}


void GetThings(wad_c *base)
{
  if (lev_doing_hexen)
  {
    GetThingsHexen(base);
    return;
  }
  
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("THINGS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_thing_t);
  }

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

  lev_things.Allocate(count);

  raw_thing_t *raw = (raw_thing_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_things.Set(i, new thing_c(i, raw));
  }
}


void GetThingsHexen(wad_c *base)
{
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("THINGS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_hexen_thing_t);
  }

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

  lev_things.Allocate(count);

  raw_hexen_thing_t *raw = (raw_hexen_thing_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_things.Set(i, new thing_c(i, raw));
  }
}


sidedef_c::sidedef_c(int _idx, const raw_sidedef_t *raw)
{
  index = _idx;

  sector = (SINT16(raw->sector) == -1) ? NULL :
    lev_sectors.Get(UINT16(raw->sector));

  x_offset = SINT16(raw->x_offset);
  y_offset = SINT16(raw->y_offset);

  memcpy(upper_tex, raw->upper_tex, sizeof(upper_tex));
  memcpy(lower_tex, raw->lower_tex, sizeof(lower_tex));
  memcpy(mid_tex,   raw->mid_tex,   sizeof(mid_tex));

  // ensure NUL terminated
  upper_tex[8] = lower_tex[8] = mid_tex[8] = 0;
}

sidedef_c::~sidedef_c()
{
}

void GetSidedefs(wad_c *base)
{
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("SIDEDEFS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_sidedef_t);
  }

  if (!lump || count == 0)
    FatalError("Couldn't find any Sidedefs");

# if DEBUG_LOAD
  PrintDebug("GetSidedefs: num = %d\n", count);
# endif

  lev_sidedefs.Allocate(count);

  raw_sidedef_t *raw = (raw_sidedef_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_sidedefs.Set(i, new sidedef_c(i, raw));
  }
}

static sidedef_c *SafeLookupSidedef(uint16_g num)
{
  if (num == 0xFFFF)
    return NULL;

  if ((int)num >= lev_sidedefs.num && (sint16_g)(num) < 0)
    return NULL;

  return lev_sidedefs.Get(num);
}


linedef_c::linedef_c(int _idx, const raw_linedef_t *raw)
{
  index = _idx;

  start = lev_vertices.Get(UINT16(raw->start));
  end   = lev_vertices.Get(UINT16(raw->end));

  /* check for zero-length line */
  zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && 
         (fabs(start->y - end->y) < DIST_EPSILON);

  flags = UINT16(raw->flags);
  type  = UINT16(raw->type);
  tag   = SINT16(raw->tag);

  two_sided = (flags & LINEFLAG_TWO_SIDED) ? true : false;

  right = SafeLookupSidedef(UINT16(raw->sidedef1));
  left  = SafeLookupSidedef(UINT16(raw->sidedef2));
}

linedef_c::linedef_c(int _idx, const raw_hexen_linedef_t *raw)
{
  index = _idx;

  start = lev_vertices.Get(UINT16(raw->start));
  end   = lev_vertices.Get(UINT16(raw->end));

  // check for zero-length line
  zero_len = (fabs(start->x - end->x) < DIST_EPSILON) && 
             (fabs(start->y - end->y) < DIST_EPSILON);

  flags = UINT16(raw->flags);
  type  = UINT8(raw->type);
  tag   = 0;

  /* read specials */
  for (int j = 0; j < 5; j++)
    specials[j] = UINT8(raw->specials[j]);

  // -JL- Added missing twosided flag handling that caused a broken reject
  two_sided = (flags & LINEFLAG_TWO_SIDED) ? true : false;

  right = SafeLookupSidedef(UINT16(raw->sidedef1));
  left  = SafeLookupSidedef(UINT16(raw->sidedef2));
}

linedef_c::~linedef_c()
{
}


void GetLinedefs(wad_c *base)
{
  if (lev_doing_hexen)
  {
    GetLinedefsHexen(base);
    return;
  }
  
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("LINEDEFS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_linedef_t);
  }

  if (!lump || count == 0)
    FatalError("Couldn't find any Linedefs");

# if DEBUG_LOAD
  PrintDebug("GetLinedefs: num = %d\n", count);
# endif

  lev_linedefs.Allocate(count);

  raw_linedef_t *raw = (raw_linedef_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_linedefs.Set(i, new linedef_c(i, raw));
  }
}

void GetLinedefsHexen(wad_c *base)
{
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("LINEDEFS");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_hexen_linedef_t);
  }

  if (!lump || count == 0)
    FatalError("Couldn't find any Linedefs");

# if DEBUG_LOAD
  PrintDebug("GetLinedefsHexen: num = %d\n", count);
# endif

  lev_linedefs.Allocate(count);

  raw_hexen_linedef_t *raw = (raw_hexen_linedef_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_linedefs.Set(i, new linedef_c(i, raw));
  }
}


static vertex_c *FindVertex16(uint16_g index)
{
  if (index & 0x8000)
    return lev_gl_verts.Get(index & 0x7FFF);

  return lev_vertices.Get(index);
}

static vertex_c *FindVertex32(int index)
{
  if (index & IS_GL_VERTEX)
    return lev_gl_verts.Get(index & ~IS_GL_VERTEX);

  return lev_vertices.Get(index);
}


seg_c::seg_c(int _idx, const raw_gl_seg_t *raw)
{
  index = _idx;

/// fprintf(stderr, "SEG %d  V %d..%d line %d side %d\n",
/// index, UINT16(raw->start), UINT16(raw->end), SINT16(raw->linedef), SINT16(raw->side));

  start = FindVertex16(UINT16(raw->start));
  end   = FindVertex16(UINT16(raw->end));

  linedef = (SINT16(raw->linedef) == -1) ? NULL :
    lev_linedefs.Get(UINT16(raw->linedef));

  side = UINT16(raw->side);

  sidedef_c *sss = NULL;

  if (linedef)
    sss = side ? linedef->left : linedef->right;
  
  sector = sss ? sss->sector : NULL;

  // partner is currently ignored

  precompute_data();
}

seg_c::seg_c(int _idx, const raw_v3_seg_t *raw)
{
  index = _idx;

  start = FindVertex32(UINT32(raw->start));
  end   = FindVertex32(UINT32(raw->end));

  linedef = (SINT16(raw->linedef) == -1) ? NULL :
    lev_linedefs.Get(UINT16(raw->linedef));

  side = UINT16(raw->flags) & V3SEG_F_LEFT;

  sidedef_c *sss = NULL;

  if (linedef)
    sss = side ? linedef->left : linedef->right;

  sector = sss ? sss->sector : NULL;

  // partner is currently ignored

  precompute_data();
}

seg_c::~seg_c()
{
}

void seg_c::precompute_data()
{
  psx = start->x;
  psy = start->y;

  pex = end->x;
  pey = end->y;

  pdx = pex - psx;
  pdy = pey - psy;

  p_length = ComputeDist(pdx, pdy);
  p_angle  = ComputeAngle(pdx, pdy);

/// fprintf(stderr, "|  (%1.0f,%1.0f)->(%1.0f,%1.0f)  Delta (%1.6f, %1.6f)\n",
/// psx, psy, pex, pey, pdx, pdy);

  if (p_length <= 0)
    PrintWarn("Seg %d has zero p_length.\n", index);

  p_perp =  psy * pdx - psx * pdy;
  p_para = -psx * pdx - psy * pdy;
}

void GetV3Segs(const uint8_g *data, int length)
{
  int count = length / sizeof(raw_v3_seg_t);

  lev_segs.Allocate(count);

  raw_v3_seg_t *raw = (raw_v3_seg_t *) data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_segs.Set(i, new seg_c(i, raw));
  }
}

void GetGLSegs(wad_c *base)
{
  lump_c *lump = base->FindLumpInLevel("GL_SEGS");

# if DEBUG_LOAD
  PrintDebug("GetVertices: num = %d\n", count);
# endif

  if (!lump)
    FatalError("Couldn't find any GL Segs");

  base->CacheLump(lump);

  if (lump->length >= 4 && memcmp(lump->data, lev_v3_magic, 4) == 0)
  {
    GetV3Segs((uint8_g *)lump->data + 4, lump->length - 4);
    return;
  }

  int count = lump->length / sizeof(raw_gl_seg_t);

  lev_segs.Allocate(count);

  raw_gl_seg_t *raw = (raw_gl_seg_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_segs.Set(i, new seg_c(i, raw));
  }
}


subsec_c::subsec_c(int _idx, const raw_subsec_t *raw) : seg_list()
{
  index = _idx;

  seg_count = UINT16(raw->num);
  int first = UINT16(raw->first);

  build_seg_list(first, seg_count);
}

subsec_c::subsec_c(int _idx, const raw_v3_subsec_t *raw) : seg_list()
{
  index = _idx;

  seg_count = UINT32(raw->num);
  int first = UINT32(raw->first);

  build_seg_list(first, seg_count);
}

subsec_c::~subsec_c()
{
}

void subsec_c::append_seg(seg_c *cur)
{
  if (! cur)
    InternalError("Missing seg for subsector !\n");

  mid_x += (cur->start->x + cur->end->x) / 2.0;
  mid_y += (cur->start->y + cur->end->y) / 2.0;

  // add it to the tail
  seg_list.push_back(cur);
}

void subsec_c::build_seg_list(int first, int count)
{
  sector = NULL;

  mid_x = 0;
  mid_y = 0;

  for (; count > 0; first++, count--)
  {
    seg_c *seg = lev_segs.Get(first);

    append_seg(seg);

    if (!sector && seg->sector)
      sector = seg->sector;
  }

  mid_x /= count;
  mid_y /= count;
}

void GetV3Subsecs(const uint8_g *data, int length)
{
  int count = length / sizeof(raw_v3_subsec_t);

  lev_subsecs.Allocate(count);

  raw_v3_subsec_t *raw = (raw_v3_subsec_t *) data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_subsecs.Set(i, new subsec_c(i, raw));
  }
}

void GetGLSubsecs(wad_c *base)
{
  lump_c *lump = base->FindLumpInLevel("GL_SSECT");

# if DEBUG_LOAD
  PrintDebug("GetVertices: num = %d\n", count);
# endif

  if (!lump)
    FatalError("Couldn't find any GL Subsectors");

  base->CacheLump(lump);

  if (lump->length >= 4 && memcmp(lump->data, lev_v3_magic, 4) == 0)
  {
    GetV3Subsecs((uint8_g *)lump->data + 4, lump->length - 4);
    return;
  }

  int count = lump->length / sizeof(raw_subsec_t);

  lev_subsecs.Allocate(count);

  raw_subsec_t *raw = (raw_subsec_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_subsecs.Set(i, new subsec_c(i, raw));
  }
}


void bbox_t::from_raw(const raw_bbox_t *raw)
{
  minx = SINT16(raw->minx);
  miny = SINT16(raw->miny);
  maxx = SINT16(raw->maxx);
  maxy = SINT16(raw->maxy);
}

node_c::node_c(int _idx, const raw_node_t *raw)
{
  index = _idx;

  x  = SINT16(raw->x);
  y  = SINT16(raw->y);
  dx = SINT16(raw->dx);
  dy = SINT16(raw->dy);

  r.bounds.from_raw(&raw->b1);
  l.bounds.from_raw(&raw->b2);

  r.node = l.node = NULL;
  r.subsec = l.subsec = NULL;

  int r_idx = UINT16(raw->right);
  int l_idx = UINT16(raw->left);

  if (r_idx & 0x8000)
    r.subsec = lev_subsecs.Get(r_idx & 0x7FFF);
  else
    r.node = lev_nodes.Get(r_idx);

  if (l_idx & 0x8000)
    l.subsec = lev_subsecs.Get(l_idx & 0x7FFF);
  else
    l.node = lev_nodes.Get(l_idx);
}

node_c::~node_c()
{
}

void GetGLNodes(wad_c *base)
{
  int count = -1;
  lump_c *lump = base->FindLumpInLevel("GL_NODES");

  if (lump)
  {
    base->CacheLump(lump);
    count = lump->length / sizeof(raw_node_t);
  }

  if (!lump || count < 1)
    return;

# if DEBUG_LOAD
  PrintDebug("GetStaleNodes: num = %d\n", count);
# endif

  lev_nodes.Allocate(count);

  raw_node_t *raw = (raw_node_t *) lump->data;

  for (int i = 0; i < count; i++, raw++)
  {
    lev_nodes.Set(i, new node_c(i, raw));
  }
}



/* ----- whole-level routines --------------------------- */

void LoadLevel(const char *level_name)
{
  // ---- Normal stuff ----

  if (! the_wad->FindLevel(level_name))
    FatalError("Unable to find level: %s\n", level_name);

  // -JL- Identify Hexen mode by presence of BEHAVIOR lump
  lev_doing_hexen = (the_wad->FindLumpInLevel("BEHAVIOR") != NULL);

  GetVertices(the_wad);
  GetSectors(the_wad);
  GetSidedefs(the_wad);
  GetLinedefs(the_wad);
  GetThings(the_wad);

  // ---- GL stuff ----

  char gl_name[16];
  
  sprintf(gl_name, "GL_%s", the_wad->current_level->name);

  wad_c *gl_wad = the_wad;

  if (! gl_wad->FindLevel(gl_name))
  {
    gl_wad = the_gwa;

    if (! gl_wad || ! gl_wad->FindLevel(gl_name))
      FatalError("Unable to find GL info (%s lump)\n", gl_name);
  }

  GetGLVerts(gl_wad);
  GetGLSegs(gl_wad);
  GetGLSubsecs(gl_wad);
  GetGLNodes(gl_wad);

/// PrintMsg("Loaded %d vertices, %d sectors, %d sides, %d lines, %d things\n", 
///     num_vertices, num_sectors, num_sidedefs, num_linedefs, num_things);

}

void FreeLevel(void)
{
  lev_vertices.FreeAll();
  lev_gl_verts.FreeAll();
  lev_linedefs.FreeAll();
  lev_sidedefs.FreeAll();
  lev_sectors.FreeAll();
  lev_things.FreeAll();
  lev_segs.FreeAll();
  lev_subsecs.FreeAll();
  lev_nodes.FreeAll();
}

