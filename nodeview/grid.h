//------------------------------------------------------------------------
//  GRID : Draws the map (lines, nodes, etc)
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2004-2005 Andrew Apted
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

#ifndef __NODEVIEW_GRID_H__
#define __NODEVIEW_GRID_H__

class W_Grid : public Fl_Widget
{
public:
	W_Grid(int X, int Y, int W, int H, const char *label = 0);
	~W_Grid();

	void SetZoom(int new_zoom);
	// changes the current zoom factor.

	void SetPos(double new_x, double new_y);
	// changes the current position.

	void FitBBox(double lx, double ly, double hx, double hy);
	// set zoom and position so that the bounding area fits.

	void MapToWin(double mx, double my, int *X, int *Y) const;
	// convert a map coordinate into a window coordinate, using
	// current grid position and zoom factor.

	void WinToMap(int X, int Y, double *mx, double *my) const;
	// convert a map coordinate into a window coordinate, using
	// current grid position and zoom factor.

public:
	int handle(int event);
	// FLTK virtual method for handling input events.

	void resize(int X, int Y, int W, int H);
	// FLTK virtual method for resizing.

private:
	void draw();
	// FLTK virtual method for drawing.

	void draw_grid(int spacing);
	void draw_partition(const node_c *nd);
	void draw_node(const node_c *nd);
	void draw_child(const child_t *ch);

	void draw_line(double x1, double y1, double x2, double y2);

	void scroll(int dx, int dy);
	// scroll the map

public:
	int handle_key(int key);

private:
	int zoom;
	// zoom factor: (2 ^ (zoom/2)) pixels per 512 units on the map

	double zoom_mul;
	// derived from 'zoom'.

	static const int MIN_GRID_ZOOM = 4;
	static const int DEF_GRID_ZOOM = 18;  // 1:1 ratio
	static const int MAX_GRID_ZOOM = 30;

	double mid_x;
	double mid_y;

	static inline int GRID_FIND(double x, double y)
	{
		return int(x - fmod(x,y) + (x < 0) ? y : 0);
	}

	static const int O_TOP    = 1;
	static const int O_BOTTOM = 2;
	static const int O_LEFT   = 4;
	static const int O_RIGHT  = 8;

	static int MAP_OUTCODE(double x, double y,
		double lx, double ly, double hx, double hy)
	{
		return
			((y < ly) ? O_BOTTOM : 0) |
			((y > hy) ? O_TOP    : 0) |
			((x < lx) ? O_LEFT   : 0) |
			((x > hx) ? O_RIGHT  : 0);
    }

};

#endif /* __NODEVIEW_GRID_H__ */
