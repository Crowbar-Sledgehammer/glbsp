//------------------------------------------------------------------------
//  MAIN Program
//------------------------------------------------------------------------
//
//  GL-Node Viewer (C) 2004-2005 Andrew Apted
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
		"**** GL-Node Viewer " " (C) 2005 Andrew Apted ****\n\n"
	);
}

static void ShowInfo(void)
{
	GUI_PrintMsg(
		"Info...\n\n"
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

		const char *filename = "doom2.wad";

		if (arg_count > 0 && ! ArgvIsOption(0))
			filename = arg_list[0];
		else
		{
			int params;
			int idx = ArgvFind(0, "file", &params);

			if (idx >= 0 && params >= 1)
				filename = arg_list[idx + 1];
		}

		if (ReadWadFile(filename) < 0)
		{
			fl_alert("Unable to read file: %s\n", filename);
			exit(2);
		}

		const char *level_name = "MAP01";
		{
			int params;
			int idx = ArgvFind(0, "level", &params);

			if (idx < 0)
				idx = ArgvFind(0, "warp", &params);

			if (idx >= 0 && params >= 1)
				level_name = arg_list[idx + 1];
		}

		if (! FindLevel(level_name))
		{
			fl_alert("Unable to find level: %s\n", level_name);
			exit(2);
		}

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

