//------------------------------------------------------------------------
// MAIN : Unix/FLTK Main program
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


#define MY_TITLE  ("GL-Node Viewer ")

#define GUI_PrintMsg  printf



/* ----- user information ----------------------------- */


static void ShowTitle(void)
{
  GUI_PrintMsg(
    "\n"
    "**** GL-Node Viewer " " (C) 2004 Andrew Apted ****\n\n"
  );
}

static void ShowInfo(void)
{
  GUI_PrintMsg(
    "This GL node builder was originally based on BSP 2.3, which was\n"
    "created from the basic theory stated in DEU5 (OBJECTS.C)\n"
    "\n"
    "Credits should go to :-\n"
    "  Andy Baker & Marc Pullen   for their invaluable help\n"
    "  Janis Legzdinsh            for fixing up Hexen support\n"
    "  Colin Reed & Lee Killough  for creating the original BSP\n"
    "  Matt Fell                  for the Doom Specs\n"
    "  Raphael Quinet             for DEU and the original idea\n"
    "  ... and everyone who helped with the original BSP.\n"
    "\n"
    "This program is free software, under the terms of the GNU General\n"
    "Public License, and comes with ABSOLUTELY NO WARRANTY.  See the\n"
    "accompanying documentation for more details.\n"
    "\n"
    "Note: glBSPX is the GUI (graphical user interface) version.\n"
    "Try plain \"glbsp\" if you want the command-line version.\n"
  );
}


void MainSetDefaults(void)
{
}


/* ----- main program ----------------------------- */



int main(int argc, char **argv)
{
  if (argc > 1 &&
      (strcmp(argv[1], "/?") == 0 || strcmp(argv[1], "-h") == 0 ||
       strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "--help") == 0 ||
       strcmp(argv[1], "-HELP") == 0 || strcmp(argv[1], "--HELP") == 0))
  {
    ShowTitle();
    ShowInfo();
    exit(1);
  }

  int first_arg = 1;

#ifdef MACOSX
  if (first_arg < argc && (strncmp(argv[first_arg], "-psn", 4) == 0))
    first_arg++;
#endif

  // set defaults, also initializes the nodebuildxxxx stuff
  MainSetDefaults();

///---  // read persistent data
///---  CookieSetPath(argv[0]);
///---
///---  cookie_status_t cookie_ret = CookieReadAll();

  // handle drag and drop: a single non-option argument
  //
  // NOTE: there is no support for giving options to glBSPX via the
  // command line.  Plain `glbsp' should be used if this is desired.
  // The difficult here lies in possible conflicts between given
  // options and those already set from within the GUI.  Plus we may
  // want to handle a drag-n-drop of multiple files later on.
  //
  bool unused_args = false;

  if (first_arg < argc && argv[first_arg][0] != '-')
  {
///---    GlbspFree(guix_info.input_file);
///---    GlbspFree(guix_info.output_file);
///---
///---    guix_info.input_file = GlbspStrDup(argv[first_arg]);
///---
///---    // guess an output name too
///---    
///---    guix_info.output_file = GlbspStrDup(
///---        HelperGuessOutput(guix_info.input_file));
///--- 
///---    first_arg++;
  }

  if (first_arg < argc)
    unused_args = true;


  // load icons for file chooser
  Fl_File_Icon::load_system_icons();

  guix_win = new Guix_MainWin(MY_TITLE);
   
///---  DialogLoadImages();

  ShowTitle();

///---  switch (cookie_ret)
///---  {
///---    case COOKIE_E_OK:
///---      break;
///---
///---    case COOKIE_E_NO_FILE:
///---      guix_win->text_box->AddMsg(
///---          "** Missing INI file -- Using defaults **\n\n", FL_RED, true);
///---      break;
///---
///---    case COOKIE_E_PARSE_ERRORS:
///---    case COOKIE_E_CHECK_ERRORS:
///---      guix_win->text_box->AddMsg(
///---          "** Warning: Errors found in INI file **\n\n", FL_RED, true);
///---      break;
///---  }

  // run the GUI until the user quits
  while (! guix_win->want_quit)
    Fl::wait();

  delete guix_win;
  guix_win = NULL;

///---  CookieWriteAll();
///---  DialogFreeImages();

  return 0;
}

