//------------------------------------------------------------------------
//  WINDOW : Main application window
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

#ifndef __NODEVIEW_WINDOW_H__
#define __NODEVIEW_WINDOW_H__

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


#endif /* __NODEVIEW_WINDOW_H__ */
