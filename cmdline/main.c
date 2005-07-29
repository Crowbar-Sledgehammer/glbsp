//------------------------------------------------------------------------
// MAIN : Command-line version main program
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2005 Andrew Apted
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
    "**** GL BSP Node Builder " GLBSP_VER " (C) 2005 Andrew Apted ****\n\n"
  );
}

static void ShowInfo(void)
{
  TextPrintMsg(
    "This GL node builder was originally based on BSP 2.3, which was\n"
    "created from the basic theory stated in DEU5 (OBJECTS.C)\n"
    "\n"
    "Credits should go to :-\n"
    "  Janis Legzdinsh            for fixing up Hexen support\n"
    "  Andy Baker & Marc Pullen   for their invaluable help\n"
    "  Colin Reed & Lee Killough  for creating the original BSP\n"
    "  Matt Fell                  for the Doom Specs\n"
    "  Raphael Quinet             for DEU and the original idea\n"
    "  ... and everyone who helped with the original BSP.\n"
    "\n");

  TextPrintMsg(
    "This program is free software, under the terms of the GNU General\n"
    "Public License, and comes with ABSOLUTELY NO WARRANTY.  See the\n"
    "accompanying documentation for more details.\n"
    "\n"
    "Usage: glbsp [options] input.wad ... [ -o output.wad ]\n"
    "\n"
    "For a list of the available options, type: glbsp -help\n"
  );
}

static void ShowOptions(void)
{
  TextPrintMsg(
    "Usage: glbsp [options] input.wad ... [ -o output.wad ]\n"
    "\n"
    "General Options:\n"
    "  -q               Quieter output, no level statistics\n"
    "  -fast            Reuse original nodes to build faster\n"
    "  -warn            Show extra warning messages\n"
    "  -normal          Forces the normal nodes to be recomputed\n"
    "  -factor  <nnn>   Changes the cost assigned to SEG splits\n"
    "  -packsides       Pack sidedefs (remove duplicates)\n"
    "  -noreject        Don't clobber the reject map\n"
    "\n");

  TextPrintMsg(
    "Rarely Useful:\n"
    "  -v1 .. -v5       Version of GL-Nodes to use (1,2,3 or 5)\n"
    "  -loadall         Loads all data from source wad (don't copy)\n"
    "  -noprog          Don't show progress indicator\n"
    "  -nonormal        Don't add (if missing) the normal nodes\n"
    "  -forcegwa        Forces the output file to be GWA style\n"
    "  -keepsect        Don't remove unused sectors\n"
    "  -noprune         Don't remove anything which is unused\n"
    "  -maxblock <nnn>  Sets the BLOCKMAP truncation limit\n"
  );
}

static void ShowDivider(void)
{
  TextPrintMsg("\n------------------------------------------------------------\n\n");
}

static int FileExists(const char *filename)
{
  FILE *fp = fopen(filename, "rb");

  if (fp)
  {
    fclose(fp);
    return TRUE;
  }

  return FALSE;
}


/* ----- main program ----------------------------- */

int main(int argc, char **argv)
{
  int extra_idx = 0;

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

  if (info.extra_files)
  {
    int ext_j;

    /* catch this mistake: glbsp in.wad out.wad (forget the -o) */

    if (info.input_file && info.extra_files[0] && ! info.extra_files[1] &&
        FileExists(info.input_file) && ! FileExists(info.extra_files[0]))
    {
      TextFatalError("Error: Cannot find WAD file %s ("
          "Maybe you forgot -o)\n", info.extra_files[0]);
    }

    /* balk NOW if any of the input files doesn't exist */

    if (! FileExists(info.input_file))
      TextFatalError("Error: Cannot find WAD file %s\n",
          info.input_file);

    for (ext_j = 0; info.extra_files[ext_j]; ext_j++)
    {
      if (FileExists(info.extra_files[ext_j]))
        continue;

      TextFatalError("Error: Cannot find WAD file %s\n",
          info.extra_files[ext_j]);
    }
  }

  /* process each input file */

  for (;;)
  {
    if (GLBSP_E_OK != GlbspCheckInfo(&info, &comms)) 
    {
      TextFatalError("Error: %s\n", comms.message ? comms.message : 
          "(Unknown error when checking args)");
    }

    if (info.no_progress)
      TextDisableProgress();

    if (GLBSP_E_OK != GlbspBuildNodes(&info, &cmdline_funcs, &comms))
    {
      TextFatalError("Error: %s\n", comms.message ? comms.message : 
          "(Unknown error during build)");
    }

    /* when there are extra input files, process them too */

    if (! info.extra_files || ! info.extra_files[extra_idx])
      break;

    ShowDivider();

    GlbspFree(info.input_file);
    GlbspFree(info.output_file);

    info.input_file  = GlbspStrDup(info.extra_files[extra_idx]);
    info.output_file = NULL;

    extra_idx++;
  }

  TextShutdown();

  return 0;
}

