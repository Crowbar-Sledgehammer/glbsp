//------------------------------------------------------------------------
// DEFS : Unix/FLTK local definitions
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2004 Andrew Apted
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

#ifndef __NODEVIEW_DEFS_H__
#define __NODEVIEW_DEFS_H__

//
//  SYSTEM INCLUDES
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <math.h>
#include <limits.h>
#include <assert.h>

#ifdef WIN32
#include <FL/x.H>
#else
#include <sys/time.h>
#endif

//
//  FLTK INCLUDES
//
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Slider.H>
#ifdef MACOSX
#include <FL/Fl_Sys_Menu_Bar.H>
#endif
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/fl_file_chooser.H>

//
//  LOCAL INCLUDES
//
#include "system.h"
#include "structs.h"
#include "util.h"
#include "wad.h"
#include "level.h"


//
//  DIALOG
//

void DialogLoadImages(void);
void DialogFreeImages(void);

int DialogShowAndGetChoice(const char *title, Fl_Pixmap *pic, 
    const char *message, const char *left = "OK", 
    const char *middle = NULL, const char *right = NULL);

int DialogQueryFilename(const char *message,
    const char ** name_ptr, const char *guess_name);

void GUI_FatalError(const char *str, ...);

#define ALERT_TXT  "GL-Node Viewer Alert"
#define MISSING_COMMS  "(Not Specified)"


//
//  GRID
//

class W_Grid : public Fl_Widget
{
public:
	W_Grid(int X, int Y, int W, int H, const char *label = 0);
	~W_Grid();

	void SetZoom(int new_zoom);
	// changes the current zoom factor.

	void SetPos(double new_x, double new_y);
	// changes the current position.

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

public:
	int handle_key(int key);

private:

#define MIN_GRID_ZOOM  4
#define DEF_GRID_ZOOM  18
#define MAX_GRID_ZOOM  30

	int zoom;
	// zoom factor: (2 ^ (zoom/2)) pixels per 512 units on the map

	double zoom_mul;
	// derived from 'zoom'.

	double mid_x;
	double mid_y;
};

#define GRID_FIND(x,y)  int(((x) < 0) ? fmod((x),(y)) + (y) : fmod((x),(y)))


//
//  MENU
//

#define MANUAL_WINDOW_MIN_W  500
#define MANUAL_WINDOW_MIN_H  200

#ifdef MACOSX
Fl_Sys_Menu_Bar
#else
Fl_Menu_Bar
#endif
* MenuCreate(int x, int y, int w, int h);

void GUI_PrintMsg(const char *str, ...);


//
//  WINDOW
//

#define MAIN_BG_COLOR  fl_gray_ramp(FL_NUM_GRAY * 9 / 24)

#define MAIN_WINDOW_MIN_W  540
#define MAIN_WINDOW_MIN_H  450

class Guix_MainWin : public Fl_Window
{
public:
	Guix_MainWin(const char *title);
	virtual ~Guix_MainWin();

  // main child widgets
  
#ifdef MACOSX
	Fl_Sys_Menu_Bar *menu_bar;
#else
	Fl_Menu_Bar *menu_bar;
#endif

	W_Grid *grid;

	// user closed the window
	bool want_quit;

	// routine to capture the current main window state into the
	// guix_preferences_t structure.
	// 
	void WritePrefs();

	// this routine is useful if the nodebuildinfo has changed.  It
	// causes all the widgets to update themselves using the new
	// parameters.
	// 
	void ReadAllInfo();

	// routine to capture all of the nodebuildinfo state from the
	// various widgets.  Note: don't need to call this before
	// destructing everything -- the widget destructors will do it
	// automatically.
	// 
	void WriteAllInfo();

protected:
  
	// initial window size, read after the window manager has had a
	// chance to move the window somewhere else.  If the window is still
	// there when CaptureState() is called, we don't need to update the
	// coords in the cookie file.
	// 
	int init_x, init_y, init_w, init_h;
};

extern Guix_MainWin * guix_win;

void WindowSmallDelay(void);


#endif /* __NODEVIEW_DEFS_H__ */
