//------------------------------------------------------------------------
//  PATH : storage for path points
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

//
// Path Constructor
//
path_c::path_c() : point_num(0), points(NULL)
{
}

//
// Path Destructor
//
path_c::~path_c()
{
	if (points)
		delete[] points;
}

//
// Path Reading
//
bool path_c::ReadFile(const char *filename)
{
	FILE *fp = fopen(filename, "r");

	if (! fp)
	{
		PrintWarn("Unable to open path file: %s\n", strerror(errno));
		return false;
	}

	points = new int[MAX_PTS * 2];

	for (point_num = 0; point_num < MAX_PTS; point_num++)
	{
		int *cur_pt = points + (point_num * 2);

		if (fscanf(fp, " %d %d ", cur_pt, cur_pt+1) != 2)
			break;
	}

	fclose(fp);

	return (point_num > 0);
}

