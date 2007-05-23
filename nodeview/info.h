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

#ifndef __NODEVIEW_INFO_H__
#define __NODEVIEW_INFO_H__

class W_Info : public Fl_Group
{
public:
  W_Info(int X, int Y, int W, int H, const char *label = 0);
  ~W_Info();

private:
  Fl_Output *map_name;
  Fl_Output *node_type;
  Fl_Output *grid_size;

  Fl_Output *ns_index;

  Fl_Box    *pt_label;
  Fl_Output *pt_x;
  Fl_Output *pt_y;
  Fl_Output *pt_dx;
  Fl_Output *pt_dy;

  Fl_Box    *bb_label;
  Fl_Output *bb_x1;
  Fl_Output *bb_y1;
  Fl_Output *bb_x2;
  Fl_Output *bb_y2;

  Fl_Output *mouse_coord;

public:
  void SetMap(const char *name);
  void SetNodes(const char *type);
  void SetZoom(float zoom_mul);

  void SetNodeIndex(int index);
  void SetSubsectorIndex(int index);
  void SetCurBBox(const bbox_t *bbox);
  void SetPartition(const node_c *part);
};

#endif /* __NODEVIEW_INFO_H__ */
