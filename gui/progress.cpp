//------------------------------------------------------------------------
// PROGRESS : Unix/FLTK Progress display
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


// consistency check
#ifndef GLBSP_GUI
#error GLBSP_GUI should be defined when compiling this file
#endif


#define BAR_ORANGEY  \
      fl_color_cube(FL_NUM_RED-1, (FL_NUM_GREEN-1)*5/7, 0)

#define BAR_CYANISH  \
      fl_color_cube(0, (FL_NUM_GREEN-1)*4/7, FL_NUM_BLUE-1)


static Guix_Progress *progress_window = NULL;


static void progress_cancel_CB(Fl_Widget *w, void *data)
{
   guix_comms.cancelled = TRUE;
}

//
// Progress Constructor
//
// Note: no label passed to Fl_Window constructor.  The main code will
// call GUI_DisplaySetTitle to do it.
//
Guix_Progress::Guix_Progress(int num_bars) :
    Fl_Window(400, (num_bars >= 2) ? 190 : 70)
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
 
  // non-resizable window
  size_range(w(), h(), w(), h());

  position(guix_prefs.progress_x, guix_prefs.progress_y);
  
  // allow manual closing of window
  callback((Fl_Callback *) progress_cancel_CB);
  
  curr_bars = num_bars;

  bars[0].lab_str = bars[1].lab_str = NULL;
  title_str = NULL;

  // create bars

  Fl_Color bar_col = (num_bars == 2) ? BAR_ORANGEY : BAR_CYANISH;
 
  bars[0].label = new Fl_Box(8, 4, w() - 16, 24);
  bars[0].label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  add(bars[0].label);
 
  bars[0].slide = new Fl_Slider(8, 30, w() - 16, 26);
  bars[0].slide->type(FL_HOR_FILL_SLIDER);
  bars[0].slide->selection_color(bar_col);
  add(bars[0].slide);

  if (curr_bars >= 2)
  {
    bars[1].label = new Fl_Box(8, 74, w() - 16, 24);
    bars[1].label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    add(bars[1].label);
  
    bars[1].slide = new Fl_Slider(8, 100, w() - 16, 26);
    bars[1].slide->type(FL_HOR_FILL_SLIDER);
    bars[1].slide->selection_color(bar_col);
    add(bars[1].slide);

    // create cancel button

    cancel = new Fl_Button(150, 150, 100, 30, "Cancel");
    cancel->callback((Fl_Callback *) progress_cancel_CB);
    cancel->shortcut(FL_Escape);
    add(cancel);
  }

  // show the window
  set_modal();
  show();

  // read initial pos (same logic as in Guix_MainWin)
  WindowSmallDelay();
  init_x = x(); init_y = y();
}


//
// Progress Destructor
//
Guix_Progress::~Guix_Progress()
{
  // update preferences if user moved the window
  if (x() != init_x || y() != init_y)
  {
    guix_prefs.progress_x = x();
    guix_prefs.progress_y = y();
  }

  GlbspFree(bars[0].lab_str);
  GlbspFree(bars[1].lab_str);
  GlbspFree(title_str);
}


//------------------------------------------------------------------------

//
// GUI_Ticker
//
void GUI_Ticker(void)
{
  Fl::check();
}


//
// GUI_DisplayOpen
//
boolean_g GUI_DisplayOpen(displaytype_e type)
{
  // shutdown any existing display
  GUI_DisplayClose();

  switch (type)
  {
    case DIS_BUILDPROGRESS:
      progress_window = new Guix_Progress(2);
      break;

    case DIS_FILEPROGRESS:
      progress_window = new Guix_Progress(1);
      break;

    default:
      return FALSE;  // unknown display type
  }

  progress_window->set_modal();
  progress_window->show();

  return TRUE;
}

//
// GUI_DisplaySetTitle
//
void GUI_DisplaySetTitle(const char *str)
{
  if (! progress_window)
    return;

  // copy the string
  GlbspFree(progress_window->title_str);
  progress_window->title_str = GlbspStrDup(str);

  progress_window->label(progress_window->title_str);
  progress_window->redraw();
}

//
// GUI_DisplaySetBarText
//
void GUI_DisplaySetBarText(int barnum, const char *str)
{
  if (! progress_window)
    return;
 
  // select the correct bar
  if (barnum < 1 || barnum > progress_window->curr_bars)
    return;
 
  guix_bar_t *bar = progress_window->bars + (barnum-1);

  // we must copy the string, as the label() method only stores the
  // pointer -- not good if we've been passed an on-stack buffer.

  GlbspFree(bar->lab_str);
  bar->lab_str = GlbspStrDup(str);

  bar->label->label(bar->lab_str);
  bar->label->redraw();

  // redraw window too 
  progress_window->redraw();
}

//
// GUI_DisplaySetBarLimit
//
void GUI_DisplaySetBarLimit(int barnum, int limit)
{
  if (! progress_window)
    return;
 
  // select the correct bar
  if (barnum < 1 || barnum > progress_window->curr_bars)
    return;
 
  guix_bar_t *bar = progress_window->bars + (barnum-1);

  bar->slide->range(0, limit);
}

//
// GUI_DisplaySetBar
//
void GUI_DisplaySetBar(int barnum, int count)
{
  if (! progress_window)
    return;

  // select the correct bar
  if (barnum < 1 || barnum > progress_window->curr_bars)
    return;
 
  guix_bar_t *bar = progress_window->bars + (barnum-1);

  // work out percentage
  int perc = 0;

  if (count >= 0 && count <= bar->slide->maximum() && 
      bar->slide->maximum() > 0)
  {
    perc = (int)(count * 100.0 / bar->slide->maximum());
  }

  sprintf(bar->perc_buf, "%d%%", perc);

//FIXME: DONT WORK:  bar->label(progress_window->percent_buf[barnum]);

  bar->slide->value(count);
  bar->slide->redraw();
}

//
// GUI_DisplayClose
//
void GUI_DisplayClose(void)
{
  if (! progress_window)
    return;

  delete progress_window;
  progress_window = NULL;
}

