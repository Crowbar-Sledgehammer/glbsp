//------------------------------------------------------------------------
// MAIN : Unix/FLTK Main program
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


#define MY_TITLE  ("glBSP Node Builder " GLBSP_VER)

#define DUMMY_FILENAME  "____dumm.wad"


// Node building info

nodebuildinfo_t guix_info;

volatile nodebuildcomms_t guix_comms;

const nodebuildfuncs_t guix_funcs =
{
  GUI_FatalError,
  GUI_PrintMsg,
  GUI_Ticker,

  GUI_DisplayOpen,
  GUI_DisplaySetTitle,
  GUI_DisplaySetBar,
  GUI_DisplaySetBarLimit,
  GUI_DisplaySetBarText,
  GUI_DisplayClose
};


// GUI-specific Globals

guix_preferences_t guix_prefs;

const guix_preferences_t default_guiprefs =
{
  40,  0,      // win_x, win_y
  560, 480,    // win_w, win_h;

  0, 0,        // progress_x, progress_y   // FIXME

  0, 0,        // dialog_x, dialog_y  // FIXME

  0, 0,        // manual_x, manual_y
  610, 400,    // manual_w, manual_h

  TRUE,        // overwrite_warn
  TRUE         // same_file_warn
};


/* ----- user information ----------------------------- */


static void ShowTitle(void)
{
  GUI_PrintMsg(
    "\n"
    "*** GL BSP Node Builder.  " GLBSP_VER " (C) 2001 Andrew Apted.  ***\n"
    "*** Based on BSP 2.3 (C) 1998 Colin Reed, Lee Killough ***\n\n"
  );
}

static void ShowInfo(void)
{
  GUI_PrintMsg(
    "This GL node builder is based on BSP 2.3, which was created\n"
    "from the basic theory stated in DEU5 (OBJECTS.C)\n"
    "\n"
    "Credits should go to :-\n"
    "  Janis Legzdinsh            for fixing up Hexen support\n"
    "  Andy Baker & Marc Pullen   for their invaluable help\n"
    "  Colin Reed & Lee Killough  for creating the original BSP\n"
    "  Matt Fell                  for the Doom Specs\n"
    "  Raphael Quinet             for DEU and the original idea\n"
    "  ... and everyone who helped with the original BSP.\n"
    "\n"
    "This program is free software, under the terms of the GNU General\n"
    "Public License, and comes with ABSOLUTELY NO WARRANTY.  See the\n"
    "accompanying documentation for more details.\n"
    "\n"
    "For a list of the available options, type: glbspX -options\n"
  );
}

static void ShowOptions(void)
{
  GUI_PrintMsg(
    "Usage: glbspX [ [options] input.wad [ -o output.wad ] ]\n"
    "\n"
    "General Options:\n"
    "  -factor <nnn>    Changes the cost assigned to SEG splits\n"
    "  -noreject        Does not clobber the reject map\n"
    "  -noprog          Does not show progress indicator\n"
    "  -warn            Show extra warning messages\n"
    "  -packsides       Pack sidedefs (remove duplicates)\n"
    "  -v1              Output V1.0 vertices (backwards compat.)\n"
    "\n"
    "Rarely Useful:\n"
    "  -loadall         Loads all data from source wad (don't copy)\n"
    "  -nogl            Does not compute the GL-friendly nodes\n"
    "  -nonormal        Does not add (if missing) the normal nodes\n"
    "  -forcenormal     Forces the normal nodes to be recomputed\n"
    "  -forcegwa        Forces the output file to be GWA style\n"
    "  -keepsect        Don't prune unused sectors\n"
    "  -noprune         Don't prune anything that is unused\n"
    "  -maxblock <nnn>  Sets the BLOCKMAP truncation limit\n"
  );
}

static void FatalError(const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vfprintf(stderr, str, args);
  va_end(args);

  exit(5);
}


static void MainSetDefaults(void)
{
  // this is much more messy than it was in C
  memcpy((nodebuildinfo_t  *) &guix_info,  &default_buildinfo,  
      sizeof(guix_info));

  memcpy((nodebuildcomms_t *) &guix_comms, &default_buildcomms, 
      sizeof(guix_comms));

  memcpy((guix_preferences_t *) &guix_prefs, &default_guiprefs,
      sizeof(guix_prefs));

  // FIXME: update all GUI elements !!! (not here tho)
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

  if (argc > 1 &&
      (strcmp(argv[1], "-options")  == 0 || 
       strcmp(argv[1], "--options") == 0 ||
       strcmp(argv[1], "-OPTIONS")  == 0 || 
       strcmp(argv[1], "--OPTIONS") == 0))
  {
    ShowTitle();
    ShowOptions();
    exit(1);
  }

  // set defaults, also initializes the nodebuildxxxx stuff
  MainSetDefaults();

  // read persistent data
  cookie_status_t cookie_ret = CookieReadAll();

  if (argc > 1)
  {
    // if run with arguments (presumably from a terminal window),
    // parse the arguments so that the values appear in the main
    // window.  We need to re-initialise the build-info in case the
    // given options conflict with those in the cookie file.

    memcpy((nodebuildinfo_t *) &guix_info, &default_buildinfo,  
        sizeof(guix_info));

    if (GLBSP_E_OK != GlbspParseArgs(&guix_info, &guix_comms, 
        (const char **)(argv+1), argc-1))
    {
      FatalError("Error: %s\n", guix_comms.message ?
          guix_comms.message : "(Unknown error parsing arguments)");
    }
  
    // check if arguments were valid.  This will balk if there was no
    // input file present -- ideally you could set options without
    // one, but it's gonna get too messy.  Besides this a GUI tool.

    if (GLBSP_E_OK != GlbspCheckInfo(&guix_info, &guix_comms))
    {
      FatalError("Error: %s\n", guix_comms.message ?
          guix_comms.message : "(Unknown error parsing arguments)");
    }
  }

  guix_win = new Guix_MainWin(MY_TITLE);
   
  DialogLoadImages();

  ShowTitle();

  switch (cookie_ret)
  {
    case COOKIE_E_OK:
      break;

    case COOKIE_E_NO_FILE:
      guix_win->text_box->AddMsg("** Missing INI file -- Using defaults **",
          FL_RED, TRUE);
      break;

    case COOKIE_E_PARSE_ERRORS:
    case COOKIE_E_CHECK_ERRORS:
      guix_win->text_box->AddMsg("** Warning: Errors found in INI file **",
          FL_RED, TRUE);
      break;
  }

  // run the GUI until the user quits
  while (! guix_win->want_quit)
    Fl::wait();

  delete guix_win;
  guix_win = NULL;

  CookieWriteAll();

  DialogFreeImages();

  return 0;
}

