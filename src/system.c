//------------------------------------------------------------------------
// SYSTEM : System specific code
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000 Andrew Apted
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

#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <assert.h>


#define DEBUG_ENABLED   0


static char message_buf[1024];


//
// FatalError
//
void FatalError(const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vsprintf(message_buf, str, args);
  va_end(args);

  (* cur_funcs->fatal_error)("\nError: *** %s ***\n\n", message_buf);
}

//
// InternalError
//
void InternalError(const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vsprintf(message_buf, str, args);
  va_end(args);

  (* cur_funcs->fatal_error)("\nINTERNAL ERROR: *** %s ***\n\n", message_buf);
}

//
// PrintMsg
//
void PrintMsg(const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vsprintf(message_buf, str, args);
  va_end(args);

  (* cur_funcs->print_msg)("%s", message_buf);
}

//
// PrintWarn
//
void PrintWarn(const char *str, ...)
{
  va_list args;

  va_start(args, str);
  vsprintf(message_buf, str, args);
  va_end(args);

  (* cur_funcs->print_msg)("Warning: %s", message_buf);
}

//
// PrintMiniWarn
//
void PrintMiniWarn(const char *str, ...)
{
  if (cur_info->mini_warnings)
  {
    va_list args;

    va_start(args, str);
    vsprintf(message_buf, str, args);
    va_end(args);

    (* cur_funcs->print_msg)("Warning: %s", message_buf);
  }
}


/* -------- debugging code ----------------------------- */

//
// InitDebug
//
void InitDebug(void)
{
#if DEBUG_ENABLED
  //!!!!
#endif
}

//
// TermDebug
//
void TermDebug(void)
{
#if DEBUG_ENABLED
  //!!!!
#endif
}

//
// PrintDebug
//
void PrintDebug(const char *str, ...)
{
#if DEBUG_ENABLED
  //!!! FIXME TO WRITE INTO FILE

  va_list args;

  fprintf(stderr, "Debug: ");

  va_start(args, str);
  vfprintf(stderr, str, args);
  va_end(args);
#else
  (void) str;
#endif
}

