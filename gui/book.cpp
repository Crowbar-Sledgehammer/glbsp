//------------------------------------------------------------------------
// BOOK : Unix/FLTK Manual Code
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


Guix_Book *guix_book_win;


static void book_quit_CB(Fl_Widget *w, void *data)
{
  if (guix_book_win)
    guix_book_win->want_quit = TRUE;
}

static void book_contents_CB(Fl_Widget *w, void *data)
{
  if (guix_book_win)
    guix_book_win->want_page = 0;
}

static void book_prev_CB(Fl_Widget *w, void *data)
{
  if (guix_book_win)
    guix_book_win->want_page = guix_book_win->cur_page - 1;
}

static void book_next_CB(Fl_Widget *w, void *data)
{
  if (guix_book_win)
    guix_book_win->want_page = guix_book_win->cur_page + 1;
}

static void book_selector_CB(Fl_Widget *w, void *data)
{
  if (guix_book_win)
    guix_book_win->FollowLink(guix_book_win->browser->value() - 1);
}


//------------------------------------------------------------------------

//
// Book Constructor
//
Guix_Book::Guix_Book() : Fl_Window(guix_prefs.manual_w,
    guix_prefs.manual_h, "glBSP Manual")
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
 
  size_range(MANUAL_WINDOW_MIN_W, MANUAL_WINDOW_MIN_H);
  position(guix_prefs.manual_x, guix_prefs.manual_y);
 
  // allow manual closing of window
  callback((Fl_Callback *) book_quit_CB);
  
  want_quit = FALSE;
  want_page = -2;
  cur_page = 0;

  // create buttons in top row

  group = new Fl_Group(0, 0, w(), 34);
  group->resizable(0);
  add(group);
  
  quit = new Fl_Button(4, 4, 96, 26, "&Quit");
  quit->box(FL_THIN_UP_BOX);
  quit->callback((Fl_Callback *) book_quit_CB);
  group->add(quit);

  contents = new Fl_Button(104, 4, 96, 26, "&Contents");
  contents->box(FL_THIN_UP_BOX);
  contents->callback((Fl_Callback *) book_contents_CB);
  group->add(contents);

  prev = new Fl_Button(204, 4, 96, 26, "<<  &Prev");
  prev->box(FL_THIN_UP_BOX);
  prev->callback((Fl_Callback *) book_prev_CB);
  group->add(prev);

  next = new Fl_Button(304, 4, 96, 26, "&Next  >>");
  next->box(FL_THIN_UP_BOX);
  next->callback((Fl_Callback *) book_next_CB);
  group->add(next);

  // create the browser

  browser = new Fl_Hold_Browser(0, 34, w(), h() - 34);
  browser->callback((Fl_Callback *) book_selector_CB);
 
  add(browser);
  resizable(browser);

  // show the window
  set_modal();
  show();

  // read initial pos (same logic as in Guix_MainWin)
  WindowSmallDelay();
  
  init_x = x(); init_y = y();
  init_w = w(); init_h = h();
}


//
// Book Destructor
//
Guix_Book::~Guix_Book()
{
  // update preferences if user moved the window
  if (x() != init_x || y() != init_y)
  {
    guix_prefs.manual_x = x();
    guix_prefs.manual_y = y();
  }

  if (w() != init_w || h() != init_h)
  {
    guix_prefs.manual_w = w();
    guix_prefs.manual_h = h();
  }
}


void Guix_Book::resize(int X, int Y, int W, int H)
{
  if (W != w())
    want_reformat = TRUE;

  Fl_Window::resize(X, Y, W, H);
}


int Guix_Book::PageCount()
{
  // we don't need anything super efficient

  int count;
  
  for (count=0; book_pages[count].text; count++)
  { /* nothing */ }

  return count;
}


void Guix_Book::LoadPage(int new_num)
{
  if (new_num < 0 || new_num >= PageCount())
    return;

  cur_page = new_num;
  want_page = -2;

  if (cur_page == 0)
    prev->deactivate();
  else
    prev->activate();

  if (cur_page == PageCount() - 1)
    next->deactivate();
  else
    next->activate();

  // -- create browser text --

  browser->clear();

  int i;
  const char ** lines = book_pages[cur_page].text;
  const char *L;

  for (i=0; lines[i]; i++)
  {
    L = lines[i];

    if (L[0] == '#' && L[1] == 'L')
      L += 4;
    
    browser->add(L);
  }

  browser->position(0);
}


void Guix_Book::FollowLink(int line)
{
  if (line < 0)
  {
    browser->deselect();
    return;
  }

  const char * L = book_pages[cur_page].text[line];
  
  if (L[0] == '#' && L[1] == 'L')
  {
    want_page = (L[2] - '0') * 10 + (L[3] - '0');
  }
  else
  {
    // for lines without links, the best we can do is just clear the
    // hold selection.
    //
    browser->deselect();
  }
}

