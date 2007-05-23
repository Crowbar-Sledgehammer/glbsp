//------------------------------------------------------------------------
//  INFO : Information Panel
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2004-2007 Andrew Apted
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


#define INFO_BG_COLOR  fl_rgb_color(96)


//
// W_Info Constructor
//
W_Info::W_Info(int X, int Y, int W, int H, const char *label) : 
    Fl_Group(X, Y, W, H, label)
{
  end();  // cancel begin() in Fl_Group constructor

  box(FL_FLAT_BOX);

//  color(INFO_BG_COLOR, INFO_BG_COLOR);


  X += 6;
  Y += 6;
  W -= 12;

  // ---- top section ----
  
  map_name = new Fl_Output(X+88, Y, W-88, 22, "Map Name:");
  map_name->align(FL_ALIGN_LEFT);
  add(map_name);

  Y += map_name->h() + 4;

  node_type = new Fl_Output(X+88, Y, W-88, 22, "Node Type:");
  node_type->align(FL_ALIGN_LEFT);
  add(node_type);

  Y += node_type->h() + 4;

  grid_size = new Fl_Output(X+88, Y, W-88, 22, "Zooming:");
  grid_size->align(FL_ALIGN_LEFT);
  add(grid_size);

  Y += grid_size->h() + 4;

  
  // ---- middle section ----
 
  Y += 12;

  ns_index = new Fl_Output(X+88, Y, W-88, 22, "Node #");
  ns_index->align(FL_ALIGN_LEFT);
  add(ns_index);

  Y += ns_index->h() + 4;


  pt_label = new Fl_Box(FL_NO_BOX, X, Y, W, 22, "Partition:");
  pt_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  add(pt_label);
  
  Y += pt_label->h() + 4;

  pt_x  = new Fl_Output(X+32,   Y, 64, 22, "x");
  pt_dx = new Fl_Output(X+W-64, Y, 64, 22, "dx");

  pt_x ->align(FL_ALIGN_LEFT);
  pt_dx->align(FL_ALIGN_LEFT);

  add(pt_x);
  add(pt_dx);

  Y += pt_x->h() + 4;

  pt_y  = new Fl_Output(X+32,   Y, 64, 22, "y");
  pt_dy = new Fl_Output(X+W-64, Y, 64, 22, "dy");

  pt_y ->align(FL_ALIGN_LEFT);
  pt_dy->align(FL_ALIGN_LEFT);

  add(pt_y);
  add(pt_dy);

  Y += pt_dy->h() + 4;


  bb_label = new Fl_Box(FL_NO_BOX, X, Y, W, 22, "Bounding Box:");
  bb_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  add(bb_label);
  
  Y += bb_label->h() + 4;

  bb_x1 = new Fl_Output(X+32,   Y, 64, 22, "x1");
  bb_x2 = new Fl_Output(X+W-64, Y, 64, 22, "x2");

  bb_x1->align(FL_ALIGN_LEFT);
  bb_x2->align(FL_ALIGN_LEFT);

  add(bb_x1);
  add(bb_x2);

  Y += bb_x1->h() + 4;

  bb_y1 = new Fl_Output(X+32,   Y, 64, 22, "y1");
  bb_y2 = new Fl_Output(X+W-64, Y, 64, 22, "y2");

  bb_y1->align(FL_ALIGN_LEFT);
  bb_y2->align(FL_ALIGN_LEFT);

  add(bb_y1);
  add(bb_y2);

  Y += bb_y2->h() + 4;

  // ETC....
}

//
// W_Info Destructor
//
W_Info::~W_Info()
{
}


void W_Info::SetMap(const char *name)
{
  char *upper = UtilStrUpper(name);

  map_name->value(upper);

  UtilFree(upper);
}

void W_Info::SetNodes(const char *type)
{
  node_type->value(type);
}

void W_Info::SetZoom(float zoom_mul)
{
  char buffer[60];

///  if (0.99 < zoom_mul && zoom_mul < 1.01)
///  {
///    grid_size->value("1:1");
///    return;
///  }

  if (zoom_mul < 0.99)
  {
    sprintf(buffer, "1/%1.3f", 1.0/zoom_mul);
  }
  else // zoom_mul > 1
  {
    sprintf(buffer, "%1.3f", zoom_mul);
  }

  grid_size->value(buffer);
}


void W_Info::SetNodeIndex(int index)
{
  char buffer[60];

  sprintf(buffer, "%d", index);
  
  ns_index->label("Node #");
  ns_index->value(buffer);

  redraw();
}

void W_Info::SetSubsectorIndex(int index)
{
  char buffer[60];

  sprintf(buffer, "%d", index);
  
  ns_index->label("Subsec #");
  ns_index->value(buffer);

  redraw();
}

void W_Info::SetCurBBox(const bbox_t *bbox)
{
  if (! bbox)
  {
    bb_x1->value("");
    bb_y1->value("");
    bb_x2->value("");
    bb_y2->value("");

    return;
  }

  char buffer[60]; 

  sprintf(buffer, "%d", bbox->minx);  bb_x1->value(buffer);
  sprintf(buffer, "%d", bbox->miny);  bb_y1->value(buffer);
  sprintf(buffer, "%d", bbox->maxx);  bb_x2->value(buffer);
  sprintf(buffer, "%d", bbox->maxy);  bb_y2->value(buffer);
}

void W_Info::SetPartition(const node_c *part)
{
  if (! part)
  {
    pt_x ->value("");
    pt_y ->value("");
    pt_dx->value("");
    pt_dy->value("");

    return;
  }

  char buffer[60]; 

  sprintf(buffer, "%d", part->x );  pt_x ->value(buffer);
  sprintf(buffer, "%d", part->y );  pt_y ->value(buffer);
  sprintf(buffer, "%d", part->dx);  pt_dx->value(buffer);
  sprintf(buffer, "%d", part->dy);  pt_dy->value(buffer);
}

