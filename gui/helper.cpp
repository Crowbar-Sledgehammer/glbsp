//------------------------------------------------------------------------
// HELPER : Unix/FLTK Little Helpers...
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


//
// HelperCaseCmp
//
int HelperCaseCmp(const char *A, const char *B)
{
  for (; *A || *B; A++, B++)
  {
    if (toupper(*A) != toupper(*B))
      return (toupper(*A) - toupper(*B));
  }

  // strings are equal
  return 0;
}


//
// HelperFilenameValid
//
boolean_g HelperFilenameValid(const char *filename)
{
  if (! filename)
    return FALSE;

  int len = strlen(filename);

  if (len == 0)
    return FALSE;

#ifdef WIN32
  // check drive letter
  if (len >= 2 && filename[1] == ':')
  {
    if (! isalpha(filename[0]))
      return FALSE;

    if (len == 2)
      return FALSE;
  }
#endif

  switch (filename[len - 1])
  {
    case '.':
    case '/':
    case '\\':
      return FALSE;
      
    default:
      break;
  }

  return TRUE;
}

//
// HelperHasExt
//
boolean_g HelperHasExt(const char *filename)
{
  int A = strlen(filename) - 1;

  if (A > 0 && filename[A] == '.')
    return FALSE;

  for (; A >= 0; A--)
  {
    if (filename[A] == '.')
      return TRUE;

    if (filename[A] == '/')
      break;
 
#ifdef WIN32
    if (filename[A] == '\\' || filename[A] == ':')
      break;
#endif
  }

  return FALSE;
}

//
// HelperCheckExt
//
boolean_g HelperCheckExt(const char *filename, const char *ext)
{
  int A = strlen(filename) - 1;
  int B = strlen(ext) - 1;

  for (; B >= 0; B--, A--)
  {
    if (A < 0)
      return FALSE;
    
    if (toupper(filename[A]) != toupper(ext[B]))
      return FALSE;
  }

  return (A >= 1) && (filename[A] == '.');
}

//
// HelperReplaceExt
//
// Returns NULL if the filename was NULL, otherwise returns a pointer
// to a static buffer containing the new filename.
// 
char *HelperReplaceExt(const char *filename, const char *ext)
{
  char *dot_pos;
  static char buffer[512];

  if (! filename || filename[0] == 0)
    return NULL;

  strcpy(buffer, filename);
  
  dot_pos = strrchr(buffer, '.');

  if (dot_pos)
    dot_pos[1] = 0;
  else
    strcat(buffer, ".");
  
  strcat(buffer, ext);
  return buffer;
}

//
// HelperGuessOutput
//
// Computes an output filename given an input one.  Returns NULL if
// the filename was NULL, otherwise returns a pointer to a static
// buffer containing the new filename.
// 
char *HelperGuessOutput(const char *filename)
{
  char *dot_pos;
  static char buffer[512];

  if (! filename || filename[0] == 0)
    return NULL;

  strcpy(buffer, filename);
  
  dot_pos = strrchr(buffer, '.');

  if (dot_pos)
    dot_pos[0] = 0;
  else
    dot_pos = buffer + strlen(buffer);
  
  // check for existing modification ("_b" etc) and update it when
  // found rather than getting level_b_b_b_b.wad :)
  
  dot_pos -= 2;

  if (dot_pos > buffer && dot_pos[0] == '_' &&
      ('a' <= dot_pos[1] && dot_pos[1] <= 'z'))
  {
    if (dot_pos[1] == 'z')
      dot_pos[1] = 'a';
    else
      dot_pos[1] += 1;
  }
  else
  {
    strcat(buffer, "_b");
  }

  strcat(buffer, ".wad");
  return buffer;
}

//
// HelperFileExists
//
boolean_g HelperFileExists(const char *filename)
{
  FILE *fp = fopen(filename, "rb");

  if (fp)
  {
    fclose(fp);
    return TRUE;
  }

  return FALSE;
}

