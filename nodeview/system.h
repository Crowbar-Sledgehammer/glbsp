//------------------------------------------------------------------------
// SYSTEM : Bridging code
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

#ifndef __NODEVIEW_SYSTEM_H__
#define __NODEVIEW_SYSTEM_H__


/* ----- basic types and macros ---------------------------- */

typedef char  sint8_g;
typedef short sint16_g;
typedef int   sint32_g;
   
typedef unsigned char  uint8_g;
typedef unsigned short uint16_g;
typedef unsigned int   uint32_g;

typedef double angle_g;  // degrees, 0 is E, 90 is N


/* ----- function prototypes ---------------------------- */

// fatal error messages (these don't return)
void FatalError(const char *str, ...);
void InternalError(const char *str, ...);

// display normal messages & warnings to the screen
void PrintMsg(const char *str, ...);
void PrintVerbose(const char *str, ...);
void PrintWarn(const char *str, ...);
void PrintMiniWarn(const char *str, ...);

// set message for certain errors
void SetErrorMsg(const char *str);

// endian handling
void InitEndian(void);
uint16_g Endian_U16(uint16_g);
uint32_g Endian_U32(uint32_g);

// these are only used for debugging
void InitDebug(void);
void TermDebug(void);
void PrintDebug(const char *str, ...);


#endif /* __NODEVIEW_SYSTEM_H__ */
