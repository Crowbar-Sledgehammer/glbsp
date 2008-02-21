//------------------------------------------------------------------------
//  ASSERTIONS
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

#include "defs.h"


void AssertFail(const char *msg, ...)
{
  char buffer[1024];

  va_list argptr;

  va_start(argptr, msg);
  vsprintf(buffer, msg, argptr);
  va_end(argptr);

  // assertion messages shouldn't overflow... (famous last words)
  buffer[sizeof(buffer) - 1] = 0;

  PrintDebug("%s\n", buffer);
  
  fprintf(stderr, "%s\n", buffer);

  exit(9);
}

