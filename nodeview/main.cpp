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
	InitDebug();
	InitEndian();

    // skip program name
    argv++, argc--;
                                                                                            
    ArgvInit(argc, (const char **)argv);
 
    if (ArgvFind('?', NULL) >= 0 || ArgvFind('h', "help") >= 0)
    {
		ShowTitle();
        ShowInfo();
        exit(1);
    }

	Fl::scheme(NULL);
	fl_message_font(FL_HELVETICA /* _BOLD */, 18);

	// load icons for file chooser
	Fl_File_Icon::load_system_icons();

	try
	{
		// set defaults, also initializes the nodebuildxxxx stuff
		MainSetDefaults();

		ReadWadFile("hhh.wad");

		FindNextLevel();
		FindNextLevel();

		LoadLevel();

		guix_win = new Guix_MainWin(MY_TITLE);

		// run the GUI until the user quits
		while (! guix_win->want_quit)
			Fl::wait();
	}
	catch (assert_fail_c err)
	{
		fl_alert("Sorry, an internal error occurred:\n%s", err.GetMessage());
	}
	catch (...)
	{
		fl_alert("An unknown problem occurred (UI code)");
	}

	delete guix_win;
	guix_win = NULL;

	return 0;
}

