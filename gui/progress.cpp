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
      fl_color_cube(FL_NUM_RED-1, (FL_NUM_GREEN-1)*7/7, 0)

#define BAR_CYANISH  \
      fl_color_cube(0, (FL_NUM_GREEN-1)*1/7, FL_NUM_BLUE-1)


void Guix_ProgressBox::CreateOneBar(guix_bar_t& bar,
    int x, int y, int w, int h,
    const char *label_short, Fl_Color col)
{
  bar.label = new Fl_Box(x+6, y+4, 50, 20, label_short);
  bar.label->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
  add(bar.label);

  bar.slide = new Fl_Slider(group->x(), y+6, group->w(), 16);
  bar.slide->set_output();
  bar.slide->slider(FL_FLAT_BOX);
  bar.slide->type(FL_HOR_FILL_SLIDER);
  bar.slide->selection_color(col);
  group->add(bar.slide);

  bar.perc = new Fl_Box(x + w - 50, y+4, 40, 20);
  bar.perc->box(FL_FLAT_BOX);
  bar.perc->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  add(bar.perc);
}

//
// ProgressBox Constructor
//
Guix_ProgressBox::Guix_ProgressBox(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h, "Progress")
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
 
  box(FL_THIN_UP_BOX);
  resizable(0);  // no resizing the kiddies, please
  
  labelfont(FL_HELVETICA | FL_BOLD);
  labeltype(FL_NORMAL_LABEL);
  align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

  curr_bars = 0;

  bars[0].lab_str = bars[1].lab_str = NULL;
  title_str = NULL;

  // create the resizable
 
  int len = w - 60 - 50;

  group = new Fl_Group(x+60, y, len, h);
  group->end();

  add(group);
  resizable(group);
      
  // create bars

  y += 16;
  CreateOneBar(bars[0], x, y, w, h, "Map", BAR_ORANGEY);
      
  y += 24;
  CreateOneBar(bars[1], x, y, w, h, "File", BAR_CYANISH);
}


//
// ProgressBox Destructor
//
Guix_ProgressBox::~Guix_ProgressBox()
{
  GlbspFree(bars[0].lab_str);
  GlbspFree(bars[1].lab_str);
  GlbspFree(title_str);
}


void Guix_ProgressBox::ClearBars(void)
{
  for (int i=0; i < 2; i++)
  {
    bars[i].slide->value(0);
    bars[i].slide->redraw();

    bars[i].perc->label(NULL);
    bars[i].perc->redraw();
  }
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
      guix_win->progress->curr_bars = 2;
      break;

    case DIS_FILEPROGRESS:
      guix_win->progress->curr_bars = 1;
      break;

    default:
      return FALSE;  // unknown display type
  }

  return TRUE;
}

//
// GUI_DisplaySetTitle
//
void GUI_DisplaySetTitle(const char *str)
{
  if (guix_win->progress->curr_bars == 0)
    return;

  // copy the string
  GlbspFree(guix_win->progress->title_str);
  guix_win->progress->title_str = GlbspStrDup(str);

///  progress_window->label(progress_window->title_str);
///  progress_window->redraw();
}

//
// GUI_DisplaySetBarText
//
void GUI_DisplaySetBarText(int barnum, const char *str)
{
  if (guix_win->progress->curr_bars == 0)
    return;
 
  // select the correct bar
  if (barnum < 1 || barnum > guix_win->progress->curr_bars)
    return;
 
  guix_bar_t *bar = guix_win->progress->bars + (barnum-1);

  // we must copy the string, as the label() method only stores the
  // pointer -- not good if we've been passed an on-stack buffer.

  GlbspFree(bar->lab_str);
  bar->lab_str = GlbspStrDup(str);

///  bar->label->label(bar->lab_str);
///  bar->label->redraw();

  // redraw window too 
  guix_win->progress->redraw();
}

//
// GUI_DisplaySetBarLimit
//
void GUI_DisplaySetBarLimit(int barnum, int limit)
{
  if (guix_win->progress->curr_bars == 0)
    return;
 
  // select the correct bar
  if (barnum < 1 || barnum > guix_win->progress->curr_bars)
    return;
 
  guix_bar_t *bar = guix_win->progress->bars + (barnum-1);

  bar->slide->range(0, limit);
}

//
// GUI_DisplaySetBar
//
void GUI_DisplaySetBar(int barnum, int count)
{
  if (guix_win->progress->curr_bars == 0)
    return;

  // select the correct bar
  if (barnum < 1 || barnum > guix_win->progress->curr_bars)
    return;
 
  guix_bar_t *bar = guix_win->progress->bars + (barnum-1);

  // work out percentage
  int perc = 0;

  if (count >= 0 && count <= bar->slide->maximum() && 
      bar->slide->maximum() > 0)
  {
    perc = (int)(count * 100.0 / bar->slide->maximum());
  }

  sprintf(bar->perc_buf, "%d%%", perc);

  bar->perc->label(bar->perc_buf);
  bar->perc->redraw();

  bar->slide->value(count);
  bar->slide->redraw();
}

//
// GUI_DisplayClose
//
void GUI_DisplayClose(void)
{
  guix_win->progress->curr_bars = 0;
}

