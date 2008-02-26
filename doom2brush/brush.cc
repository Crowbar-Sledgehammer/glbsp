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


#define DUMMY_TEX  "e7/e7panelwood2"


class brush_side_c
{
public:
  double x1, y1;
  double x2, y2;

public:
   brush_side_c() { }
  ~brush_side_c() { }
};

static void GetBrushSides(subsec_c *sub, std::vector<brush_side_c>& sides)
{
  sides.clear();

  for (unsigned int k = 0; k < sub->seg_list.size(); k++)
  {
    seg_c * seg = sub->seg_list[k];

    // check if already present (co-linear segs)
    bool already = false;
    for (unsigned int n = 0; n < sides.size(); n++)
    {
      brush_side_c& b = sides[n];

      double ps = PerpDist(seg->start->x, seg->start->y,
                           b.x1, b.y1, b.x2, b.y2);

      double pe = PerpDist(seg->end->x, seg->end->y,
                           b.x1, b.y1, b.x2, b.y2);

      if (fabs(ps) < 0.02 && fabs(pe) < 0.02)
      {
        already = true; break;
      }
    }

    if (already)
      continue;

    brush_side_c new_b;

    // make the brush_side face OUTWARDS (segs face inwards)
    new_b.x1 = seg->end->x;
    new_b.y1 = seg->end->y;
    new_b.x2 = seg->start->x;
    new_b.y2 = seg->start->y;

    sides.push_back(new_b);
  }
}


static void FindSectorExtents(void)
{
  for (int loop = 0; loop < 2; loop++)
  {
    for (int i = 0; i < lev_linedefs.num; i++)
    {
      linedef_c *L = lev_linedefs.Get(i);

      if (! (L->left  && L->left->sector))
        continue;

      if (! (L->right && L->right->sector))
        continue;

      sector_c *B = L->left->sector;
      sector_c *F = L->right->sector;

      B->floor_under = MIN(B->floor_under, F->floor_under);
      B->ceil_over   = MAX(B->ceil_over  , F->ceil_over  );

      F->floor_under = B->floor_under;
      F->ceil_over   = B->ceil_over;
    }
  }
}

static void CollectExtraFloors(void)
{
  for (int i = 0; i < lev_linedefs.num; i++)
  {
    linedef_c *L = lev_linedefs.Get(i);

    if (L->zero_len || ! L->right || ! L->right->sector)
      continue;

    if (L->type == 400 && L->tag > 0)
    {
      for (int k = 0; k < lev_sectors.num; k++)
      {
        sector_c *S = lev_sectors.Get(k);

        if (S->tag != L->tag)
          continue;

        S->extrafloors.push_back(L->right->sector);
        S->ef_lines.push_back(L);
      }
    }

    if (L->type == 405 && L->tag > 0)
    {
      for (int k = 0; k < lev_sectors.num; k++)
      {
        sector_c *S = lev_sectors.Get(k);

        if (S->tag != L->tag)
          continue;

        S->liquids.push_back(L->right->sector);
      }
    }
  }
}

static void CollectSlopes(void)
{
  for (int i = 0; i < lev_linedefs.num; i++)
  {
    linedef_c *L = lev_linedefs.Get(i);

    if (! L->right || ! L->right->sector ||
        ! L->left  || ! L->left->sector  || L->zero_len)
      continue;

    if (L->type < 777 && L->type > 779)
      continue;

    // determine further point in sector from this line
    
    sector_c *S = L->right->sector;

    double far_dist = 0;

    for (int k = 0; k < lev_linedefs.num; k++)
    {
      linedef_c *M = lev_linedefs.Get(k);

      if ((M->right && M->right->sector == S) ||
          (M->left  && M->left->sector  == S))
      {
        double d1 = PerpDist(M->start->x, M->start->y,
                             L->start->x, L->start->y, L->end->x, L->end->y);

        double d2 = PerpDist(M->end->x, M->end->y,
                             L->start->x, L->start->y, L->end->x, L->end->y);

        if (d1 > far_dist)
          far_dist = d1;

        if (d2 > far_dist)
          far_dist = d2;
      }
    }

    if (far_dist < 0.01)
      FatalError("Bad slope in sector %d\n", S->index);

    double nx = L->end->y - L->start->y;
    double ny = L->start->x - L->end->x;

    double n_len = ComputeDist(nx, ny);

    nx = far_dist * nx / n_len;
    ny = far_dist * ny / n_len;

    if (L->type == 777 || L->type == 779)
    {
      S->floor_slope = new slope_c();

      S->floor_slope->sx = L->start->x;
      S->floor_slope->sy = L->start->y;
      S->floor_slope->sz = L->left->sector->floor_h;

      S->floor_slope->ex = L->start->x + nx;
      S->floor_slope->ey = L->start->y + ny;
      S->floor_slope->ez = S->floor_h;
    }

    if (L->type == 778 || L->type == 779)
    {
      S->ceil_slope = new slope_c();

      S->ceil_slope->sx = L->start->x;
      S->ceil_slope->sy = L->start->y;
      S->ceil_slope->sz = L->left->sector->ceil_h;

      S->ceil_slope->ex = L->start->x + nx;
      S->ceil_slope->ey = L->start->y + ny;
      S->ceil_slope->ez = S->ceil_h;
    }
  }
}

static const char *DetermineSideTex(subsec_c *sub, brush_side_c& b, int is_ceil)
{
  // find longest seg on the brush side
  const char *best = NULL;

  double best_len = -1;

  for (unsigned int k = 0; k < sub->seg_list.size(); k++)
  {
    seg_c * seg = sub->seg_list[k];

    if (! seg->linedef)
      continue;

    double ps = PerpDist(seg->start->x, seg->start->y,
                         b.x1, b.y1, b.x2, b.y2);

    double pe = PerpDist(seg->end->x, seg->end->y,
                         b.x1, b.y1, b.x2, b.y2);

    if (! (fabs(ps) < 0.02 && fabs(pe) < 0.02))
      continue;

    // use opposite side (brushes face out, but segs face in)
    sidedef_c *side = seg->side ? seg->linedef->right : seg->linedef->left;

    if (! side)
      continue;

    const char *tex_name = is_ceil ? side->upper_tex : side->lower_tex;

    if (strlen(tex_name) == 0 || tex_name[0] == '-')
      continue;

    double len = ComputeDist(seg->start->x - seg->end->x,
                             seg->start->y - seg->end->y);

    if (best && len < best_len)
      continue;

    best = tex_name;
    best_len = len;
  }

  return best ? Texture_Convert(best) : NULL;
}


static void WriteSlopedFloor(sector_c *S, const char *tex)
{
  slope_c *P = S->floor_slope;
  SYS_ASSERT(P);

  double nx = P->ey - P->sy;
  double ny = P->sx - P->ex;

  fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
      P->sx, P->sy, P->sz,
      P->ex, P->ey, P->ez, 
      P->ex + nx, P->ey + ny, P->ez,
      tex);
}

static void WriteSlopedCeiling(sector_c *S, const char *tex)
{
  slope_c *P = S->ceil_slope;
  SYS_ASSERT(P);

  double nx = P->ey - P->sy;
  double ny = P->sx - P->ex;

  fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
      P->sx, P->sy, P->sz,
      P->ex, P->ey, P->ez, 
      P->ex - nx, P->ey - ny, P->ez,
      tex);
}


void Brush_ConvertSectors(void)
{
  FindSectorExtents();

  CollectExtraFloors();
  CollectSlopes();

  std::vector<brush_side_c> sides;

  for (int i = 0; i < lev_subsecs.num; i++)
  {
    subsec_c *sub = lev_subsecs.Get(i);

    sector_c *S = sub->sector;

    if (! sub->sector)
    {
      fprintf(stderr, "Warning: subsec:%d has no sector\n", i);
      continue;
    }

    GetBrushSides(sub, sides);

    if (sides.size() < 3)
    {
      fprintf(stderr, "Warning: subsec:%d in sector:%d invalid (only %d sides)\n",
              i, S->index, (int)sides.size());
      continue;
    }

    int num_pass = 3 + (int)S->extrafloors.size();

    for (int pass = 0; pass < num_pass; pass++)
    {
      double z1, z2;
      const char *flat_name = NULL;

      if (pass == 0)
      {
        z1 = S->floor_under - 64;
        z2 = S->floor_h;

        flat_name = S->floor_tex;
      }
      else if (pass == 1)
      {
        z1 = S->ceil_h;
        z2 = S->ceil_over + 64;

        flat_name = S->ceil_tex;
      }
      else if (pass == 2)  // liquid
      {
        if (S->liquids.size() == 0)
          continue;

        sector_c *liq = S->liquids[0];

        z1 = S->floor_under - 32;
        z2 = liq->floor_h;

        flat_name = liq->floor_tex;
      }
      else  // extrafloor
      {
        unsigned int e_idx = pass - 3;
        SYS_ASSERT(e_idx < S->extrafloors.size());

        sector_c *EF = S->extrafloors[e_idx];

        z1 = EF->floor_h;
        z2 = EF->ceil_h;

        flat_name = EF->ceil_tex;
      }

      flat_name = Texture_Convert(flat_name);

      fprintf(map_fp, "// %s sector:%d subsec:%d\n",
              (pass == 0) ? "floor" : (pass == 1) ? "ceiling" :
              (pass == 2) ? "liquid" : "extrafloor",
              S->index, i);

      fprintf(map_fp, "{\n");

      // Top
      if (pass == 0 && S->floor_slope)
      {
        WriteSlopedFloor(S, flat_name);
      }
      else
      {
        fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
            0.0, 0.0, z2,  0.0, 1.0, z2,  1.0, 0.0, z2,
            flat_name);
      }

      // Bottom
      if (pass == 1 && S->ceil_slope)
      {
        WriteSlopedCeiling(S, flat_name);
      }
      else
      {
        fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
            0.0, 0.0, z1,  1.0, 0.0, z1,  0.0, 1.0, z1,
            flat_name);
      }

      // Sides
      for (unsigned int k = 0; k < sides.size(); k++)
      {
        brush_side_c& b = sides[k];

        const char *side_tex = NULL;

        if (pass == 1 && strncmp(S->ceil_tex, "F_SKY", 5) == 0)
        {
          side_tex = Texture_Convert("NODRAW");
        }
        else if (pass < 2)
        {
          side_tex = DetermineSideTex(sub, b, pass == 1);
        }
        else if (pass == 2)
        {
          side_tex = Texture_Convert("NODRAW");
        }
        else if (pass > 2)
        {
          linedef_c *EL = S->ef_lines[pass - 3];
          SYS_ASSERT(EL && EL->right);

          if (EL->right->mid_tex[0] && EL->right->mid_tex[0] != '-')
            side_tex = Texture_Convert(EL->right->mid_tex);
        }

        if (! side_tex)
          side_tex = flat_name;

        fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
            b.x1, b.y1, z1,  b.x1, b.y1, z2,  b.x2, b.y2, z2,
            side_tex);
      }

      fprintf(map_fp, "}\n");
    }
  }
}


//------------------------------------------------------------------------

static void CollectLinesAtVertices(void)
{
  for (int i = 0; i < lev_linedefs.num; i++)
  {
    linedef_c *L = lev_linedefs.Get(i);

    if (L->zero_len)
      continue;

    L->start->lines.push_back(L);
    L->end  ->lines.push_back(L);
  }
}

static double GetInteriorAngle(vertex_c *V, linedef_c *L)
{
  // returns the angle on the back side of the linedef
  // and the neighbouring linedef (both one-sided) at the
  // given vertex.

  SYS_ASSERT(V == L->start || V == L->end);

  double line_ang = ComputeAngle(L->end->x - L->start->x, L->end->y - L->start->y);

  if (V == L->end)
  {
    line_ang = line_ang + 180.0;
    if (line_ang >= 360.0)
      line_ang -= 360.0;
  }

  double best_ang = 999.9;

  for (unsigned int i = 0; i < V->lines.size(); i++)
  {
    linedef_c *N = V->lines[i];

    // we only do one-sided linedefs
    if (N->left || ! N->right || ! N->right->sector)
      continue;

    if (N->zero_len)
      continue;

    SYS_ASSERT(V == N->start || V == N->end);

    double nb_ang = ComputeAngle(N->end->x - N->start->x, N->end->y - N->start->y);

    if (V == N->end)
    {
      nb_ang = nb_ang + 180.0;
      if (nb_ang >= 360.0)
        nb_ang -= 360.0;
    }

    double diff = line_ang - nb_ang;

    if (diff < 0)
      diff += 360.0;

    fprintf(stderr, "AT (%1.0f %1.0f) diff=%1.4f\n", V->x, V->y, diff);

    best_ang = MIN(best_ang, diff);
  }

  if (best_ang > 360.0)
  {
    fprintf(stderr, "Warning: unconnected one-sided line at (%1.0f,%1.0f)\n",
            V->x, V->y);

    return 90.0;
  }

  return best_ang;
}

void Brush_ConvertWalls(void)
{
  CollectLinesAtVertices();

  for (int i = 0; i < lev_linedefs.num; i++)
  {
    linedef_c *L = lev_linedefs.Get(i);

    // we only do one-sided linedefs
    if (L->left || ! L->right || ! L->right->sector)
      continue;

    if (L->zero_len)
      continue;

    double left_A  = GetInteriorAngle(L->start, L);
//!!!    double right_A = GetInteriorAngle(L->end,   L);

    double x[4], y[4];

    x[0] = L->start->x;  y[0] = L->start->y;
    x[1] = L->end  ->x;  y[1] = L->end  ->y;

    // FIXME: compute back coordinates properly !!!!

    double nx = y[1] - y[0];
    double ny = x[0] - x[1];

    double n_len = ComputeDist(nx, ny);

    nx /= n_len;
    ny /= n_len;

    x[2] = x[1] - nx * 8;
    y[2] = y[1] - ny * 8;

    x[3] = x[0] - nx * 8;
    y[3] = y[0] - ny * 8;

    double z1 = L->right->sector->floor_under - 64;
    double z2 = L->right->sector->ceil_over + 64;

    const char *tex_name = Texture_Convert(L->right->mid_tex);

    fprintf(map_fp, "// wall line:%d sector:%d\n", i, L->right->sector->index);
    fprintf(map_fp, "{\n");

    // Top
    fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
        0.0, 0.0, z2,  0.0, 1.0, z2,  1.0, 0.0, z2,
        tex_name);

    // Bottom
    fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
        0.0, 0.0, z1,  1.0, 0.0, z1,  0.0, 1.0, z1,
        tex_name);

    // Sides
    for (int k1 = 0; k1 < 4; k1++)
    {
      int k2 = (k1+1) % 4;

      fprintf(map_fp, "  ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) ( %1.4f %1.4f %1.4f ) %s 0 0 0 0.50 0.50\n",
          x[k1], y[k1], z1,  x[k1], y[k1], z2,  x[k2], y[k2], z2,
          tex_name);
    }

    fprintf(map_fp, "}\n");
  }
}


//------------------------------------------------------------------------

void Brush_WriteField(const char *field, const char *val_str, ...)
{
  fprintf(map_fp, "  \"%s\"  \"", field);

  va_list arg_ptr;

  va_start(arg_ptr, val_str);
  vfprintf(map_fp, val_str, arg_ptr);
  va_end(arg_ptr);

  fprintf(map_fp, "\"\n");
}


static sector_c *PointInSector(double x, double y)
{
  // we have a BSP tree, may as well use it

  if (lev_nodes.num == 0)
  {
    // no nodes means a very simple level (one convex sector)
    return lev_sectors.Get(0);
  }

  // begin at the root node
  node_c *nd = lev_nodes.Get(lev_nodes.num - 1);

  for (int loop_limit = 0; loop_limit < 9000; loop_limit++)
  {
    double d = PerpDist(x, y, nd->x, nd->y, nd->x + nd->dx, nd->y + nd->dy);

    if (d < 0)
    {
      if (nd->l.subsec)
        return nd->l.subsec->sector;

      nd = nd->l.node;
    }
    else
    {
      if (nd->r.subsec)
        return nd->r.subsec->sector;

      nd = nd->r.node;
    }

    SYS_ASSERT(nd);
  }

  FatalError("PointInSector: infinite node loop?\n");
  return NULL; /* NOT REACHED */
}


void Brush_ConvertThings(void)
{
  int telept_target = 1;

  for (int i = 0; i < lev_things.num; i++)
  {
    thing_c *T = lev_things.Get(i);

    const char *ent_name = Entity_Convert(T->type);

    if (! ent_name)
      continue;

    sector_c *sec = PointInSector(T->x, T->y);
    SYS_ASSERT(sec);

    int z;

    // use "Ambush" flag to mean "place on 2nd extrafloor"
    if ((T->options & 8) && sec->extrafloors.size() > 0)
      z = sec->extrafloors[0]->ceil_h;
    else
      z = sec->floor_h;

    z += T->height;

    fprintf(map_fp, "// thing #%d type:%d\n", i, T->type);
    fprintf(map_fp, "{\n");

    Brush_WriteField("classname", ent_name);
    Brush_WriteField("origin", "%d %d %d", T->x, T->y, z + 25);

    if (T->angle != 0)
    {
      Brush_WriteField("angle", "%d", T->angle);
    }

    if (T->type == 14)
    {
      Brush_WriteField("targetname", "t%d", telept_target);
      telept_target++;
    }

    fprintf(map_fp, "}\n");
  }
}

