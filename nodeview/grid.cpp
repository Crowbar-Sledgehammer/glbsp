//------------------------------------------------------------------------
// GRID : Draws the map (lines, nodes, etc)
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


static void foo_win_close_CB(Fl_Widget *w, void *data)
{
}


//
// W_Grid Constructor
//
W_Grid::W_Grid(int X, int Y, int W, int H, const char *label) : 
        Fl_Widget(X, Y, W, H, label),
        zoom(DEF_GRID_ZOOM), zoom_mul(1.0),
        mid_x(0), mid_y(0)
{
}

//
// W_Grid Destructor
//
W_Grid::~W_Grid()
{
}


void W_Grid::SetZoom(int new_zoom)
{
	zoom = new_zoom;

	zoom_mul = pow(2.0, (zoom / 2.0 - 9.0));

	//  fprintf(stderr, "Zoom %d  Mul %1.5f\n", zoom, zoom_mul);

	redraw();
}


void W_Grid::SetPos(double new_x, double new_y)
{
	/// FIXME

	redraw();
}

void W_Grid::MapToWin(double mx, double my, int *X, int *Y) const
{
	double hx = x() + w() / 2.0;
	double hy = y() + h() / 2.0;

	(*X) = I_ROUND(hx + (mx - mid_x) * zoom_mul);
	(*Y) = I_ROUND(hy - (my - mid_y) * zoom_mul);
}

void W_Grid::WinToMap(int X, int Y, double *mx, double *my) const
{
	double hx = x() + w() / 2.0;
	double hy = y() + h() / 2.0;

	(*mx) = mid_x + (X - hx) / zoom_mul;
	(*my) = mid_y - (Y - hy) / zoom_mul;
}

//------------------------------------------------------------------------

void W_Grid::resize(int X, int Y, int W, int H)
{
	Fl_Widget::resize(X, Y, W, H);
}

void W_Grid::draw()
{
	/// FIXME

	fl_push_clip(x(), y(), w(), h());

	fl_color(FL_BLACK);
	fl_rectf(x(), y(), w(), h());

	if (zoom >= 22)
	{
		fl_color(fl_color_cube(0, 0, 1));
		draw_grid(1);
	}

	if (zoom >= 16)
	{
		fl_color(fl_color_cube(0, 1, 1));
		draw_grid(8);
	}

	if (zoom >= 9)
	{
		fl_color(fl_color_cube(0, 0, 2));
		draw_grid(64);
	}

	if (true)
	{
		fl_color(fl_color_cube(0, 0, 4));
		draw_grid(512);
	}

	fl_pop_clip();
}

void W_Grid::draw_grid(int spacing)
{
	int x1 = x();
	int y1 = y();

	int x2 = x() + w();
	int y2 = y() + h();

	int gx = GRID_FIND(mid_x, spacing);
	int gy = GRID_FIND(mid_y, spacing);

	int dx, dy;
	int wx, wy;

	for (dx = 0; true; dx++)
	{
		MapToWin(gx + dx * spacing, gy, &wx, &wy);
		if (wx > x2) break;
		fl_yxline(wx, y1, y2);
	}

	for (dx = -1; true; dx--)
	{
		MapToWin(gx + dx * spacing, gy, &wx, &wy);
		if (wx < x1) break;
		fl_yxline(wx, y1, y2);
	}

	for (dy = 0; true; dy++)
	{
		MapToWin(gx, gy + dy * spacing, &wy, &wy);
		if (wy < y1) break;
		fl_xyline(x1, wy, x2);
	}

	for (dy = -1; true; dy--)
	{
		MapToWin(gx, gy + dy * spacing, &wy, &wy);
		if (wy > y2) break;
		fl_xyline(x1, wy, x2);
	}
}

//------------------------------------------------------------------------

int W_Grid::handle(int event)
{
	switch (event)
	{
		case FL_FOCUS:
			return 1;

		case FL_KEYDOWN:
		case FL_SHORTCUT:
			return handle_key(Fl::event_key());

		case FL_ENTER:
		case FL_LEAVE:
			return 1;

		case FL_MOVE:
			//...
			return 1;

		case FL_PUSH:
			//...
			redraw();
			return 1;

		case FL_DRAG:
		case FL_RELEASE:
			// these are currently ignored.
			return 1;

		default:
			break;
	}

	return 0;  // unused
}

int W_Grid::handle_key(int key)
{
	if (key == 0)
		return 0;

	if (key == '+' || key == '=')
	{
		if (zoom < MAX_GRID_ZOOM)
			SetZoom(zoom + 1);

		return 1;
	}

	if (key == '-' || key == '_')
	{
		if (zoom > MIN_GRID_ZOOM)
			SetZoom(zoom - 1);

		return 1;
	}

	return 0;  // unused
}

