//------------------------------------------------------------------------
// MAIN : Command-line version main program
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2002 Andrew Apted
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

#include "../glbsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include "display.h"


// consistency check
#ifndef GLBSP_TEXT
#error GLBSP_TEXT should be defined when compiling this file
#endif


static nodebuildinfo_t info;
static volatile nodebuildcomms_t comms;


/* ----- user information ----------------------------- */

static void ShowTitle(void)
{
  TextPrintMsg(
    "\n"
    "*** GL BSP Node Builder.  " GLBSP_VER " (C) 2001 Andrew Apted.  ***\n"
    "*** Based on BSP 2.3 (C) 1998 Colin Reed, Lee Killough ***\n\n"
  );
}

static void ShowInfo(void)
{
  TextPrintMsg(
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
    "Usage: glbsp [options] input.wad [ -o output.wad ]\n"
    "\n"
    "For a list of the available options, type: glbsp -help\n"
  );
}

static void ShowOptions(void)
{
  TextPrintMsg(
    "Usage: glbsp [options] input.wad [ -o output.wad ]\n"
    "\n"
    "General Options:\n"
    "  -factor <nnn>    Changes the cost assigned to SEG splits\n"
    "  -fresh           Choose fresh partition lines\n"
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
    "  -noprune         Don't prune anything which is unused\n"
    "  -maxblock <nnn>  Sets the BLOCKMAP truncation limit\n"
  );
}


/* ----- main program ----------------------------- */

int main(int argc, char **argv)
{
  TextStartup();

  ShowTitle();

  // skip program name itself
  argv++, argc--;
  
  if (argc <= 0)
  {
    ShowInfo();
    TextShutdown();
    exit(1);
  }

  if (strcmp(argv[0], "/?") == 0 || strcmp(argv[0], "-h") == 0 ||
      strcmp(argv[0], "-help") == 0 || strcmp(argv[0], "--help") == 0 ||
      strcmp(argv[0], "-HELP") == 0 || strcmp(argv[0], "--HELP") == 0)
  {
    ShowOptions();
    TextShutdown();
    exit(1);
  }

  info  = default_buildinfo;
  comms = default_buildcomms;

  if (GLBSP_E_OK != GlbspParseArgs(&info, &comms, 
      (const char **)argv, argc))
  {
    TextFatalError("Error: %s\n", comms.message ? comms.message : 
        "(Unknown error when parsing args)");
  }

  if (GLBSP_E_OK != GlbspCheckInfo(&info, &comms)) 
  {
    TextFatalError("Error: %s\n", comms.message ? comms.message : 
        "(Unknown error when checking args)");
  }

  if (GLBSP_E_OK != GlbspBuildNodes(&info, &cmdline_funcs, &comms))
  {
    TextFatalError("Error: %s\n", comms.message ? comms.message : 
        "(Unknown error during build)");
  }

  TextShutdown();

  return 0;
}

