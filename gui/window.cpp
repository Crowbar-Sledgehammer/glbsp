//------------------------------------------------------------------------
// WINDOW : Unix/FLTK application window
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2001 Andrew Apted
//
//  Based on `BSP 2.3' by Colin Reed, Lee Killough and others.
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
#include "local.h"

#ifndef WIN32
#include <unistd.h>
#endif


#define MY_BG_COLOR  fl_gray_ramp(FL_NUM_GRAY * 9 / 24)


Guix_MainWin *guix_win;


static void main_win_close_CB(Fl_Widget *w, void *data)
{
  if (guix_win)
    guix_win->want_quit = TRUE;
}


//
// WindowSmallDelay
//
// This routine is meant to delay a short time (e.g. 1/5th of a
// second) to allow the window manager to move our windows around.
//
// Hopefully such nonsense doesn't happen under Win32.
//
void WindowSmallDelay(void)
{
#ifndef WIN32
  Fl::wait(0);  usleep(100 * 1000);
  Fl::wait(0);  usleep(100 * 1000);
#endif

  Fl::wait(0);
}


//
// MainWin Constructor
//
Guix_MainWin::Guix_MainWin(const char *title) :
    Fl_Window(guix_prefs.win_w, guix_prefs.win_h, title)
{
  int hw;

  // turn off auto-add-widget mode
  end();

  size_range(MAIN_WINDOW_MIN_W, MAIN_WINDOW_MIN_H);
 
  // Set initial position.
  //
  // Note: this may not work properly.  It seems that when my window
  // manager adds the titlebar/border, it moves the actual window
  // down and slightly to the right, causing subsequent invokations
  // to keep going lower and lower.

  position(guix_prefs.win_x, guix_prefs.win_y);

  callback((Fl_Callback *) main_win_close_CB);

  // set a nice darkish gray for the space between main boxes
  color(MY_BG_COLOR, MY_BG_COLOR);

  want_quit = FALSE;


  // create contents
  hw = (w() - 8*2 - 4) / 2;
    
  menu_bar = MenuCreate(0, 0, w(), 28);
  add(menu_bar);

  build_mode = new Guix_BuildMode(8, 32, hw, 140);
  add(build_mode);

  misc_opts  = new Guix_MiscOptions(8+hw+4, 32, hw, 140);
  add(misc_opts);
   
  files = new Guix_FileBox(8, 176, w()-8*2, 88);
  add(files);

  factor = new Guix_FactorBox(8, 268, hw, 40);
  add(factor);

  builder = new Guix_BuildButton(8+hw+4, 268, hw, 40);
  add(builder);

  text_box = new Guix_TextBox(0, 312, w(), h() - 312);
  add(text_box);
  resizable(text_box);


  // show window (pass some dummy arguments)
  int argc = 1;
  char *argv[] = { "glbspX", NULL };
  
  show(argc, argv);

  // read initial pos, giving 1/5th of a second for the WM to adjust
  // our window's position (naughty WM...)
  WindowSmallDelay();

  init_x = x(); init_y = y();
  init_w = w(); init_h = h();
}

//
// MainWin Destructor
//
Guix_MainWin::~Guix_MainWin()
{
  WritePrefs();
}


void Guix_MainWin::WritePrefs()
{
  // check if moved or resized
  if (x() != init_x || y() != init_y)
  {
    guix_prefs.win_x = x();
    guix_prefs.win_y = y();
  }

  if (w() != init_w || h() != init_h)
  {
    guix_prefs.win_w = w();
    guix_prefs.win_h = h();
  }
}


void Guix_MainWin::ReadAllInfo()
{
  // Note: do build_mode before files
  build_mode->ReadInfo();
  misc_opts->ReadInfo();
  files->ReadInfo();
  factor->ReadInfo();
}


void Guix_MainWin::WriteAllInfo()
{
  // Note: do build_mode before files
  build_mode->WriteInfo();
  misc_opts->WriteInfo();
  files->WriteInfo();
  factor->WriteInfo();
}

