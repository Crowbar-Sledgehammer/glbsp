//------------------------------------------------------------------------
// TEXTBOX : Unix/FLTK Text messages
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


//
// TextBox Constructor
//
Guix_TextBox::Guix_TextBox(int x, int y, int w, int h) :
    Fl_Browser(x, y, w, h)
{
  // cancel the automatic `begin' in Fl_Group constructor
  // (it's an ancestor of Fl_Browser).
  end();
 
  textfont(FL_COURIER);
  textsize(16);
}


//
// TextBox Destructor
//
Guix_TextBox::~Guix_TextBox()
{
  // nothing to do
}


void Guix_TextBox::AddMsg(const char *msg, Fl_Color col = FL_BLACK,
    boolean_g bold = FALSE)
{
  const char *r;

  char buffer[2048];
  int b_idx;

  // setup formatting string
  buffer[0] = 0;

  if (col != FL_BLACK)
    sprintf(buffer, "@C%d", col);

  if (bold)
    strcat(buffer, "@b");
  
  strcat(buffer, "@.");
  b_idx = strlen(buffer);

  while ((r = strchr(msg, '\n')) != NULL)
  {
    strncpy(buffer+b_idx, msg, r - msg);
    buffer[b_idx + r - msg] = 0;

    if (r - msg == 0)
    {
      // workaround for FLTK bug
      strcpy(buffer+b_idx, " ");
    }

    add(buffer);
    msg = r+1;
  }

  if (strlen(msg) > 0)
  {
    strcpy(buffer+b_idx, msg);
    add(buffer);
  }

  // move browser to last line
  if (size() > 0)
    bottomline(size());
}


void Guix_TextBox::AddHorizBar()
{
  add(" ");
  add(" ");
  add("@-");
  add(" ");
  add(" ");
 
  // move browser to last line
  if (size() > 0)
    bottomline(size());
}


void Guix_TextBox::ClearLog()
{
  clear();
}


boolean_g Guix_TextBox::SaveLog(const char *filename)
{
  // FIXME: implement SaveLog

  return FALSE;
}


//------------------------------------------------------------------------

//
// GUI_PrintMsg
//
void GUI_PrintMsg(const char *str, ...)
{
  char buffer[2048];
  
  va_list args;

  va_start(args, str);
  vsprintf(buffer, str, args);
  va_end(args);

  // handle pre-windowing text (ShowOptions and friends)
  if (guix_win)
  {
    guix_win->text_box->AddMsg(buffer,
        (strncmp(buffer, "Warning", 7) == 0) ? FL_RED : FL_BLACK);
  }
  else
  {
    printf("%s", buffer);
    fflush(stdout);
  }
}

