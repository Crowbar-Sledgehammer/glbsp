//------------------------------------------------------------------------
// MENU : Unix/FLTK Menu handling
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


static boolean_g menu_want_to_quit;


static void menu_quit_CB(Fl_Widget *w, void *data)
{
  menu_want_to_quit = TRUE;
}

///---static void menu_do_reset(Fl_Widget *w, void * data)
///---{
///---  /// FIXME
///---}

static void menu_do_clear_log(Fl_Widget *w, void * data)
{
  guix_win->text_box->ClearLog();
}

static void menu_do_exit(Fl_Widget *w, void * data)
{
  guix_win->want_quit = TRUE;
}


//------------------------------------------------------------------------

static void menu_do_prefs(Fl_Widget *w, void * data)
{
  guix_pref_win = new Guix_PrefWin();

  // run the GUI until the user closes
  while (! guix_pref_win->want_quit)
    Fl::wait();

  delete guix_pref_win;
}


//------------------------------------------------------------------------

static const char *about_Info =
  "By Andrew Apted (C) 2000-2001\n"
  "\n"
  "Based on BSP 2.3 (C) 1998 Colin Reed, Lee Killough\n"
  "\n"
  "Additional credits to...\n"
  "    Andy Baker & Marc Pullen, for invaluable help\n"
  "    Janis Legzdinsh, for fixing up Hexen support\n"
  "    Matt Fell, for the Doom Specs\n"
  "    Raphael Quinet, for DEU and the original idea\n"
  "    ... and everyone else who deserves it !\n"
  "\n"
  "This program is free software, under the terms of\n"
  "the GNU General Public License, and comes with\n"
  "ABSOLUTELY NO WARRANTY.\n"
  "\n"
  "Website:  http://glbsp.sourceforge.net";


static void menu_do_about(Fl_Widget *w, void * data)
{
  menu_want_to_quit = FALSE;

  Fl_Window *ab_win = new Fl_Window(600, 340, "About glBSP");
  ab_win->end();

  // non-resizable
  ab_win->size_range(ab_win->w(), ab_win->h(), ab_win->w(), ab_win->h());
  ab_win->position(guix_prefs.manual_x, guix_prefs.manual_y);
  ab_win->callback((Fl_Callback *) menu_quit_CB);

  // add the about image
  Fl_Group *group = new Fl_Group(0, 0, 230, ab_win->h());
  group->box(FL_FLAT_BOX);
  group->color(FL_BLACK, FL_BLACK);
  ab_win->add(group);
  
  Fl_Box *box = new Fl_Box(0, 60, ABOUT_IMG_W+2, ABOUT_IMG_H+2);
  about_image->label(box);
  group->add(box); 


  // nice big logo text
  box = new Fl_Box(240, 5, 350, 50, "glbspX  " GLBSP_VER);
  box->labelsize(24);
  ab_win->add(box);

  // about text
  box = new Fl_Box(240, 60, 350, 270, about_Info);
  box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
  ab_win->add(box);
   
 
  // finally add an "OK" button
  Fl_Button *button = new Fl_Button(ab_win->w()-10-60, ab_win->h()-10-30, 
      60, 30, "OK");
  button->callback((Fl_Callback *) menu_quit_CB);
  ab_win->add(button);
    
  ab_win->set_modal();
  ab_win->show();
  
  // capture initial size
  WindowSmallDelay();
  int init_x = ab_win->x();
  int init_y = ab_win->y();
  
  // run the GUI until the user closes
  while (! menu_want_to_quit)
    Fl::wait();

  // check if the user moved/resized the window
  if (ab_win->x() != init_x || ab_win->y() != init_y)
  {
    guix_prefs.manual_x = ab_win->x();
    guix_prefs.manual_y = ab_win->y();
  }
 
  // this deletes all the child widgets too...
  delete ab_win;
}


//------------------------------------------------------------------------

static void menu_do_manual(Fl_Widget *w, void * data)
{
  guix_book_win = new Guix_Book();

  guix_book_win->LoadPage(guix_prefs.manual_page);

  // run the GUI until the user closes
  while (! guix_book_win->want_quit)
  {
    guix_book_win->Update();

    Fl::wait();
  }

  delete guix_book_win;
}


//------------------------------------------------------------------------

static void menu_do_license(Fl_Widget *w, void * data)
{
  menu_want_to_quit = FALSE;

  Fl_Window *lic_win = new Fl_Window(guix_prefs.manual_w, 
      guix_prefs.manual_h, "glBSP License");

  lic_win->end();
  lic_win->size_range(MANUAL_WINDOW_MIN_W, MANUAL_WINDOW_MIN_H);
  lic_win->position(guix_prefs.manual_x, guix_prefs.manual_y);
  lic_win->callback((Fl_Callback *) menu_quit_CB);
 
  Fl_Browser *browser = new Fl_Browser(0, 0, lic_win->w(), lic_win->h());
 
  int i;

  for (i=0; license_text[i]; i++)
    browser->add(license_text[i]);

  browser->position(0);

  lic_win->add(browser);
  lic_win->resizable(browser);
  lic_win->set_modal();
  lic_win->show();

  // capture initial size
  WindowSmallDelay();
  int init_x = lic_win->x();
  int init_y = lic_win->y();
  int init_w = lic_win->w();
  int init_h = lic_win->h();
  
  // run the GUI until the user closes
  while (! menu_want_to_quit)
    Fl::wait();

  // check if the user moved/resized the window
  if (lic_win->x() != init_x || lic_win->y() != init_y)
  {
    guix_prefs.manual_x = lic_win->x();
    guix_prefs.manual_y = lic_win->y();
  }
 
  if (lic_win->w() != init_w || lic_win->h() != init_h)
  {
    guix_prefs.manual_w = lic_win->w();
    guix_prefs.manual_h = lic_win->h();
  }
 
  delete lic_win;
}


//------------------------------------------------------------------------

#undef FCAL
#define FCAL  (Fl_Callback *)

static Fl_Menu_Item menu_items[] = 
{
  { "&File", 0, 0, 0, FL_SUBMENU },
    { "&Preferences...",    0, FCAL menu_do_prefs },
/// { "&Reset All Options", 0, FCAL menu_do_reset },
    { "&Clear Log",         0, FCAL menu_do_clear_log, 0, FL_MENU_DIVIDER },
/// { "&Save Log...",       0, FCAL menu_do_save_log, 0, FL_MENU_DIVIDER },
    { "E&xit",   FL_ALT + 'q', FCAL menu_do_exit },
    { 0 },

  { "&Help", 0, 0, 0, FL_SUBMENU },
    { "&About...",         0,  FCAL menu_do_about },
    { "&License...",       0,  FCAL menu_do_license },
    { "&Manual...",   FL_F+1,  FCAL menu_do_manual },
    { 0 },

  { 0 }
};


//
// MenuCreate
//
Fl_Menu_Bar * MenuCreate(int x, int y, int w, int h)
{
  Fl_Menu_Bar *bar = new Fl_Menu_Bar(x, y, w, h);

  bar->menu(menu_items);
 
  return bar;
}

