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

  map_name = new Fl_Output(X+88, Y, W-88, 22, "Map Name:");
  map_name->align(FL_ALIGN_LEFT);
  map_name->value("unknown");
  add(map_name);

  Y += map_name->h() + 4;

  node_type = new Fl_Output(X+88, Y, W-88, 22, "Node Type:");
  node_type->align(FL_ALIGN_LEFT);
  node_type->value("unknown");
  add(node_type);

  Y += map_name->h() + 4;

  grid_size = new Fl_Output(X+88, Y, W-88, 22, "Zooming:");
  grid_size->align(FL_ALIGN_LEFT);
  grid_size->value("unset");
  add(grid_size);

  Y += map_name->h() + 4;

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

