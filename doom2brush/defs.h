//------------------------------------------------------------------------
//  DEFINITIONS
//------------------------------------------------------------------------
//
//  Doom-2-Brush (C) 2008 Andrew Apted
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

#ifndef __DM2BR_DEFS_H__
#define __DM2BR_DEFS_H__

#define PROG_NAME  "Doom2Brush"

//
//  SYSTEM INCLUDES
//
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <math.h>
#include <limits.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#include <map>
#include <string>
#include <vector>


//
//  LOCAL INCLUDES
//
#include "system.h"
#include "asserts.h"
#include "structs.h"
#include "util.h"
#include "wad.h"
#include "level.h"
#include "texture.h"
#include "brush.h"

extern FILE *map_fp;

#endif /* __DM2BR_DEFS_H__ */
