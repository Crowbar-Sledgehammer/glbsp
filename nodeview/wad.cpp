//------------------------------------------------------------------------
// WAD : WAD read/write functions.
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

///---#include "system.h"
///---
///---#include <stdio.h>
///---#include <stdlib.h>
///---#include <string.h>
///---#include <stdarg.h>
///---#include <ctype.h>
///---#include <math.h>
///---#include <limits.h>
///---
///---#include "blockmap.h"
///---#include "level.h"
///---#include "node.h"
///---#include "seg.h"
///---#include "structs.h"
///---#include "util.h"
///---#include "wad.h"


static FILE *in_file = NULL;
static FILE *out_file = NULL;


#define DEBUG_DIR   0
#define DEBUG_LUMP  0

#define APPEND_BLKSIZE  256
#define LEVNAME_BUNCH   20

#define ALIGN_LEN(len)  ((((len) + 3) / 4) * 4)


// current wad info
static wad_t wad;


/* ---------------------------------------------------------------- */


#define NUM_LEVEL_LUMPS  12
#define NUM_GL_LUMPS     5

static const char *level_lumps[NUM_LEVEL_LUMPS]=
{
  "THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS", 
  "SSECTORS", "NODES", "SECTORS", "REJECT", "BLOCKMAP",
  "BEHAVIOR",  // <-- hexen support
  "SCRIPTS"  // -JL- Lump with script sources
};

static const char *gl_lumps[NUM_GL_LUMPS]=
{
  "GL_VERT", "GL_SEGS", "GL_SSECT", "GL_NODES",
  "GL_PVS"  // -JL- PVS (Potentially Visible Set) lump
};

static const char align_filler[4] = { 0, 0, 0, 0 };


//
// CheckMagic
//
static bool CheckMagic(const char type[4])
{
  if ((type[0] == 'I' || type[0] == 'P') && 
       type[1] == 'W' && type[2] == 'A' && type[3] == 'D')
  {
    return true;
  }
  
  return false;
}


//
// CheckLevelName
//
static bool CheckLevelName(const char *name)
{
  int n;
  
  if (strlen(name) > 5)
    return false;

  for (n=0; n < wad.num_level_names; n++)
  {
    if (strcmp(wad.level_names[n], name) == 0)
      return true;
  }

  return false;
}


//
// CheckLevelLumpName
//
// Tests if the entry name is one of the level lumps.
//
static bool CheckLevelLumpName(const char *name)
{
  int i;

  for (i=0; i < NUM_LEVEL_LUMPS; i++)
  {
    if (strcmp(name, level_lumps[i]) == 0)
      return true;
  }
  
  return false;
}


//
// CheckGLLumpName
//
// Tests if the entry name matches GL_ExMy or GL_MAPxx, or one of the
// GL lump names.
//
static bool CheckGLLumpName(const char *name)
{
  int i;

  if (name[0] != 'G' || name[1] != 'L' || name[2] != '_')
    return false;

  for (i=0; i < NUM_GL_LUMPS; i++)
  {
    if (strcmp(name, gl_lumps[i]) == 0)
      return true;
  }
  
  return CheckLevelName(name+3);
}


//
// Level name helper
//
static void AddLevelName(const char *name)
{
  if ((wad.num_level_names % LEVNAME_BUNCH) == 0)
  {
    wad.level_names = (const char **) UtilRealloc((void *)wad.level_names,
        (wad.num_level_names + LEVNAME_BUNCH) * sizeof(const char *));
  }

  wad.level_names[wad.num_level_names] = UtilStrDup(name);
  wad.num_level_names++;
}


//
// NewLevel
//
// Create new level information
//
static level_t *NewLevel(int flags)
{
  level_t *cur;

  cur = (level_t *)UtilCalloc(sizeof(level_t));

  cur->flags = flags;

  return cur;
}


//
// NewLump
//
// Create new lump.  `name' must be allocated storage.
//
static lump_t *NewLump(char *name)
{
  lump_t *cur;

  cur = (lump_t *)UtilCalloc(sizeof(lump_t));

  cur->name = name;
  cur->start = cur->new_start = -1;
  cur->flags = LUMP_NEW;
  cur->length = 0;
  cur->space = 0;
  cur->data = NULL;
  cur->lev_info = NULL;

  return cur;
}


//
// FreeLump
//
static void FreeLump(lump_t *lump)
{
  // free level lumps, if any
  if (lump->lev_info)
  {
    while (lump->lev_info->children)
    {
      lump_t *head = lump->lev_info->children;
      lump->lev_info->children = head->next;

      // the ol' recursion trick... :)
      FreeLump(head);
    }

    UtilFree(lump->lev_info);
  }

  // check `data' here, since it gets freed in WriteLumpData()
  if (lump->data)
    UtilFree(lump->data);
  
  UtilFree(lump->name);
  UtilFree(lump);
}


//
// ReadHeader
//
// Returns true if successful, or FALSE if there was a problem (in
// which case the error message as been setup).
//
static int ReadHeader(const char *filename)
{
  size_t len;
  raw_wad_header_t header;
  char strbuf[1024];

  len = fread(&header, sizeof(header), 1, in_file);

  if (len != 1)
  {
    sprintf(strbuf, "Trouble reading wad header for %s : %s", 
      filename, strerror(errno));

    SetErrorMsg(strbuf);
    return false;
  }

  if (! CheckMagic(header.type))
  {
    sprintf(strbuf, "%s does not appear to be a wad file : bad magic", 
        filename);

    SetErrorMsg(strbuf);
    return false;
  }

  wad.kind = (header.type[0] == 'I') ? IWAD : PWAD;
  
  wad.num_entries = UINT32(header.num_entries);
  wad.dir_start   = UINT32(header.dir_start);

  // initialise stuff
  wad.dir_head = NULL;
  wad.dir_tail = NULL;
  wad.current_level = NULL;
  wad.level_names = NULL;
  wad.num_level_names = 0;

  return true;
}


//
// ReadDirEntry
//
static void ReadDirEntry(void)
{
  size_t len;
  raw_wad_entry_t entry;
  lump_t *lump;
  
  len = fread(&entry, sizeof(entry), 1, in_file);

  if (len != 1)
    FatalError("Trouble reading wad directory");

  lump = NewLump(UtilStrNDup(entry.name, 8));

  lump->start  = UINT32(entry.start);
  lump->length = UINT32(entry.length);

# if DEBUG_DIR
  PrintDebug("Read dir... %s\n", lump->name);
# endif

  // link it in
  lump->next = NULL;
  lump->prev = wad.dir_tail;

  if (wad.dir_tail)
    wad.dir_tail->next = lump;
  else
    wad.dir_head = lump;

  wad.dir_tail = lump;
}

//
// DetermineLevelNames
//
static void DetermineLevelNames(void)
{
  lump_t *L, *N;
  int i;

  for (L=wad.dir_head; L; L=L->next)
  {
    // check if the next four lumps after the current lump match the
    // level-lump names.
    //
    for (i=0, N=L->next; (i < 4) && N; i++, N=N->next)
      if (strcmp(N->name, level_lumps[i]) != 0)
        break;

    if (i != 4)
      continue;

#   if DEBUG_DIR
    PrintDebug("Found level name: %s\n", L->name);
#   endif

    // check for invalid name and duplicate levels
    if (strlen(L->name) > 5)
      PrintWarn("Bad level name `%s' in wad (too long)\n", L->name);
    else if (CheckLevelName(L->name))
      PrintWarn("Level name `%s' found twice in wad\n", L->name);
    else
      AddLevelName(L->name);
  }
}

//
// ProcessDirEntry
//
static void ProcessDirEntry(lump_t *lump)
{
  // ignore previous GL lump info
  if (CheckGLLumpName(lump->name))
  {
#   if DEBUG_DIR
    PrintDebug("Discarding previous GL info: %s\n", lump->name);
#   endif

    FreeLump(lump);
    wad.num_entries--;

    return;
  }

///---  // mark the lump as `ignorable' when in GWA mode.
///---  if (cur_info->gwa_mode)
///---    lump->flags |= LUMP_IGNORE_ME;

  // --- LEVEL MARKERS ---

  if (CheckLevelName(lump->name))
  {
    /* NOTE !  Level marks can have data (in Hexen anyway) */
///---    if (cur_info->load_all)
///---      lump->flags |= LUMP_READ_ME;
///---    else
      lump->flags |= LUMP_COPY_ME;

    // OK, start a new level

    lump->lev_info = NewLevel(0);

    wad.current_level = lump;

#   if DEBUG_DIR
    PrintDebug("Process dir... %s :\n", lump->name);
#   endif

    // link it in
    lump->next = NULL;
    lump->prev = wad.dir_tail;

    if (wad.dir_tail)
      wad.dir_tail->next = lump;
    else
      wad.dir_head = lump;

    wad.dir_tail = lump;

    return;
  }

  // --- LEVEL LUMPS ---

  if (wad.current_level)
  {
    if (CheckLevelLumpName(lump->name))
    {
      // check for duplicates
      if (FindLevelLump(lump->name))
      {
        PrintWarn("Duplicate entry `%s' ignored in %s\n",
            lump->name, wad.current_level->name);

        FreeLump(lump);
        wad.num_entries--;

        return;
      }

#     if DEBUG_DIR
      PrintDebug("Process dir... |--- %s\n", lump->name);
#     endif

      // mark it to be loaded
      lump->flags |= LUMP_READ_ME;
    
      // link it in
      lump->next = wad.current_level->lev_info->children;
      lump->prev = NULL;

      if (lump->next)
        lump->next->prev = lump;

      wad.current_level->lev_info->children = lump;
      return;
    }
      
    // OK, non-level lump.  End the previous level.

    wad.current_level = NULL;
  }

  // --- ORDINARY LUMPS ---

# if DEBUG_DIR
  PrintDebug("Process dir... %s\n", lump->name);
# endif

  if (CheckLevelLumpName(lump->name))
    PrintWarn("Level lump `%s' found outside any level\n", lump->name);

  // maybe load data
///---  if (cur_info->load_all)
///---    lump->flags |= LUMP_READ_ME;
///---  else
    lump->flags |= LUMP_COPY_ME;

  // link it in
  lump->next = NULL;
  lump->prev = wad.dir_tail;

  if (wad.dir_tail)
    wad.dir_tail->next = lump;
  else
    wad.dir_head = lump;

  wad.dir_tail = lump;
}

//
// ReadDirectory
//
static void ReadDirectory(void)
{
  int i;
  int total_entries = wad.num_entries;
  lump_t *prev_list;

  fseek(in_file, wad.dir_start, SEEK_SET);

  for (i=0; i < total_entries; i++)
  {
    ReadDirEntry();
  }

  DetermineLevelNames();

  // finally, unlink all lumps and process each one in turn
  
  prev_list = wad.dir_head;
  wad.dir_head = wad.dir_tail = NULL;
  
  while (prev_list)
  {
    lump_t *cur = prev_list;
    prev_list = cur->next;

    ProcessDirEntry(cur);
  }
}


//
// ReadLumpData
//
static void ReadLumpData(lump_t *lump)
{
  size_t len;

///---  cur_comms->file_pos++;
///---  DisplaySetBar(1, cur_comms->file_pos);
///---  DisplayTicker();

# if DEBUG_LUMP
  PrintDebug("Reading... %s (%d)\n", lump->name, lump->length);
# endif
  
  if (lump->length == 0)
    return;

  lump->data = UtilCalloc(lump->length);

  fseek(in_file, lump->start, SEEK_SET);

  len = fread(lump->data, lump->length, 1, in_file);

  if (len != 1)
  {
    if (wad.current_level)
      PrintWarn("Trouble reading lump `%s' in %s\n",
          lump->name, wad.current_level->name);
    else
      PrintWarn("Trouble reading lump `%s'\n", lump->name);
  }

  lump->flags &= ~LUMP_READ_ME;
}


//
// ReadAllLumps
//
// Returns number of entries read.
//
static int ReadAllLumps(void)
{
  lump_t *cur, *L;
  int count = 0;

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    count++;

    if (cur->flags & LUMP_READ_ME)
      ReadLumpData(cur);

    if (cur->lev_info && ! (cur->lev_info->flags & LEVEL_IS_GL))
    {
      for (L=cur->lev_info->children; L; L=L->next)
      {
        count++;

        if (L->flags & LUMP_READ_ME)
          ReadLumpData(L);
      }
    }
  }

  return count;
}


/* ---------------------------------------------------------------- */

//
// CreateGLMarker
//
lump_t *CreateGLMarker(lump_t *level)
{
  lump_t *cur;
  char name_buf[16];

  sprintf(name_buf, "GL_%s", level->name);

  cur = NewLump(UtilStrDup(name_buf));

  cur->lev_info = NewLevel(LEVEL_IS_GL);

  // link it in
  cur->next = level->next;
  cur->prev = level;

  if (cur->next)
    cur->next->prev = cur;

  level->next = cur;
  level->lev_info->buddy = cur;

  return cur;
}


/* ---------------------------------------------------------------- */


//
// CheckExtension
//
bool CheckExtension(const char *filename, const char *ext)
{
  int A = (int)strlen(filename) - 1;
  int B = (int)strlen(ext) - 1;

  for (; B >= 0; B--, A--)
  {
    if (A < 0)
      return false;
    
    if (toupper(filename[A]) != toupper(ext[B]))
      return false;
  }

  return (A >= 1) && (filename[A] == '.');
}

//
// ReplaceExtension
//
char *ReplaceExtension(const char *filename, const char *ext)
{
  char *dot_pos;
  char buffer[512];

  strcpy(buffer, filename);
  
  dot_pos = strrchr(buffer, '.');

  if (dot_pos)
    dot_pos[1] = 0;
  else
    strcat(buffer, ".");
  
  strcat(buffer, ext);

  return UtilStrDup(buffer);
}


//
// CreateLevelLump
//
lump_t *CreateLevelLump(const char *name)
{
  lump_t *cur;

# if DEBUG_DIR
  PrintDebug("Create Lump... %s\n", name);
# endif

  // already exists ?
  for (cur=wad.current_level->lev_info->children; cur; cur=cur->next)
  {
    if (strcmp(name, cur->name) == 0)
      break;
  }
  
  if (cur)
  {
    if (cur->data)
      UtilFree(cur->data);
    
    cur->data = NULL;
    cur->length = 0;
    cur->space  = 0;

    return cur;
  }

  // nope, allocate new one
  cur = NewLump(UtilStrDup(name));

  // link it in
  cur->next = wad.current_level->lev_info->children;
  cur->prev = NULL;

  if (cur->next)
    cur->next->prev = cur;

  wad.current_level->lev_info->children = cur;

  return cur;
}


//
// CreateGLLump
//
lump_t *CreateGLLump(const char *name)
{
  lump_t *cur;
  lump_t *gl_level;

# if DEBUG_DIR
  PrintDebug("Create GL Lump... %s\n", name);
# endif

  // create GL level marker if necessary
  if (! wad.current_level->lev_info->buddy)
    CreateGLMarker(wad.current_level);
  
  gl_level = wad.current_level->lev_info->buddy;

  // check if already exists
  for (cur=gl_level->lev_info->children; cur; cur=cur->next)
  {
    if (strcmp(name, cur->name) == 0)
      break;
  }

  if (cur)
  {
    if (cur->data)
      UtilFree(cur->data);
    
    cur->data = NULL;
    cur->length = 0;
    cur->space  = 0;

    return cur;
  }

  // nope, allocate new one
  cur = NewLump(UtilStrDup(name));

  // link it in
  cur->next = gl_level->lev_info->children;
  cur->prev = NULL;

  if (cur->next)
    cur->next->prev = cur;

  gl_level->lev_info->children = cur;

  return cur;
}


//
// AppendLevelLump
//
void AppendLevelLump(lump_t *lump, void *data, int length)
{
  if (length == 0)
    return;

  if (lump->length == 0)
  {
    lump->space = MAX(length, APPEND_BLKSIZE);
    lump->data = UtilCalloc(lump->space);
  }
  else if (lump->space < length)
  {
    lump->space = MAX(length, APPEND_BLKSIZE);
    lump->data = UtilRealloc(lump->data, lump->length + lump->space);
  }

  memcpy(((char *)lump->data) + lump->length, data, length);

  lump->length += length;
  lump->space  -= length;
}


//
// CountLevels
//
int CountLevels(void)
{
  lump_t *cur;
  int result = 0;
  
  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    if (cur->lev_info && ! (cur->lev_info->flags & LEVEL_IS_GL))
      result++;
  }

  return result;
}

//
// FindNextLevel
//
int FindNextLevel(void)
{
  lump_t *cur;
  
  if (wad.current_level)
    cur = wad.current_level->next;
  else
    cur = wad.dir_head;

  while (cur && ! (cur->lev_info && ! (cur->lev_info->flags & LEVEL_IS_GL)))
    cur=cur->next;

  wad.current_level = cur;

  return (cur != NULL);
}

//
// GetLevelName
//
const char *GetLevelName(void)
{
  if (!wad.current_level)
    InternalError("GetLevelName: no current level");
    
  return wad.current_level->name;
}

//
// FindLevelLump
//
lump_t *FindLevelLump(const char *name)
{
  lump_t *cur = wad.current_level->lev_info->children;

  while (cur && (strcmp(cur->name, name) != 0))
    cur=cur->next;

  return cur;
}

//
// CheckLevelLumpZero
//
bool CheckLevelLumpZero(lump_t *lump)
{
  int i;
  
  if (lump->length == 0)
    return true;
  
  // ASSERT(lump->data)

  for (i=0; i < lump->length; i++)
  {
    if (((uint8_g *)lump->data)[i])
      return false;
  }

  return true;
}

//
// ReadWadFile
//
int ReadWadFile(const char *filename)
{
  int check;
  char strbuf[1024];

  // open input wad file & read header
  in_file = fopen(filename, "rb");

  if (! in_file)
  {
    sprintf(strbuf, "Cannot open WAD file %s : %s", filename, 
        strerror(errno));
    
    SetErrorMsg(strbuf);

    return -1;
  }
  
  if (! ReadHeader(filename))
  {
    fclose(in_file);
    return -1;
  }

  PrintMsg("Opened %cWAD file : %s\n", (wad.kind == IWAD) ? 'I' : 'P', 
      filename); 
  PrintVerbose("Reading %d dir entries at 0x%X\n", wad.num_entries, 
      wad.dir_start);

  // read directory
  ReadDirectory();

///---  DisplayOpen(DIS_FILEPROGRESS);
///---  DisplaySetTitle("glBSP Reading Wad");
  
  sprintf(strbuf, "Reading: %s", filename);

///---  DisplaySetBarText(1, strbuf);
///---  DisplaySetBarLimit(1, CountLumpTypes(LUMP_READ_ME, LUMP_READ_ME));
///---  DisplaySetBar(1, 0);
///---  cur_comms->file_pos = 0;

  // now read lumps
  check = ReadAllLumps();

  if (check != wad.num_entries)
    InternalError("Read directory count consistency failure (%d,%d)",
      check, wad.num_entries);
  
  wad.current_level = NULL;

///---  DisplayClose();

  return 0;
}


//
// DeleteGwaFile
//
void DeleteGwaFile(const char *base_wad_name)
{
  char *gwa_file = ReplaceExtension(base_wad_name, "gwa");

  if (remove(gwa_file) == 0)
    PrintMsg("Deleted GWA file : %s\n", gwa_file);
 
  UtilFree(gwa_file);
}


//
// CloseWads
//
void CloseWads(void)
{
  int i;

  if (in_file)
  {
    fclose(in_file);
    in_file = NULL;
  }
  
  if (out_file)
  {
    fclose(out_file);
    out_file = NULL;
  }
  
  /* free directory entries */
  while (wad.dir_head)
  {
    lump_t *head = wad.dir_head;
    wad.dir_head = head->next;

    FreeLump(head);
  }

  /* free the level names */
  if (wad.level_names)
  {
    for (i=0; i < wad.num_level_names; i++)
      UtilFree((char *) wad.level_names[i]);

    UtilFree((void *)wad.level_names);
    wad.level_names = NULL;
  }
}


/* ---------------------------------------------------------------- */


//
// Mark failure routines
//
void MarkSoftFailure(int soft)
{
  wad.current_level->lev_info->soft_limit |= soft;
}

void MarkHardFailure(int hard)
{
  wad.current_level->lev_info->hard_limit |= hard;
}

void MarkV3Switch(int v3)
{
  wad.current_level->lev_info->v3_switch |= v3;
}


//
// ReportOneOverflow(
//
void ReportOneOverflow(const lump_t *lump, int limit, bool hard)
{
  PrintMsg("%-6s : ", lump->name);

  const char *msg = hard ? "overflowed the absolute limit" :
    "overflowed the original limit";

  switch (limit)
  {
    case LIMIT_VERTEXES: PrintMsg("Number of Vertices %s.\n", msg); break;
    case LIMIT_SECTORS:  PrintMsg("Number of Sectors %s.\n", msg); break;
    case LIMIT_SIDEDEFS: PrintMsg("Number of Sidedefs %s\n", msg); break;
    case LIMIT_LINEDEFS: PrintMsg("Number of Linedefs\n", msg); break;

    case LIMIT_SEGS:     PrintMsg("Number of Segs %s.\n", msg); break;
    case LIMIT_SSECTORS: PrintMsg("Number of Subsectors %s.\n", msg); break;
    case LIMIT_NODES:    PrintMsg("Number of Nodes %s.\n", msg); break;

    case LIMIT_GL_VERT:  PrintMsg("Number of GL vertices %s.\n", msg); break;
    case LIMIT_GL_SEGS:  PrintMsg("Number of GL segs %s.\n", msg); break;
    case LIMIT_GL_SSECT: PrintMsg("Number of GL subsectors %s.\n", msg); break;
    case LIMIT_GL_NODES: PrintMsg("Number of GL nodes %s.\n", msg); break;

    case LIMIT_BAD_SIDE:   PrintMsg("One or more linedefs has a bad sidedef.\n"); break;
    case LIMIT_BMAP_TRUNC: PrintMsg("Blockmap area was too big - truncated.\n"); break;
    case LIMIT_BLOCKMAP:   PrintMsg("Blockmap lump %s.\n", msg); break;

    default:
      InternalError("UNKNOWN LIMIT BIT: 0x%06x", limit);
  }
}

// ReportOverflows
//
void ReportOverflows(bool hard)
{
  if (hard)
  {
    PrintMsg(
      "ERRORS.  The following levels failed to be built, and won't\n"
      "work in any Doom port (and may even crash it).\n\n"
    );
  }
  else  // soft
  {
    PrintMsg(
      "POTENTIAL FAILURES.  The following levels should work in a\n"
      "modern Doom port, but may fail (or even crash) in older ports.\n\n"
    );
  }

  lump_t *cur;

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    level_t *lev = cur->lev_info;

    int limits, one_lim;

    if (! (lev && ! (lev->flags & LEVEL_IS_GL)))
      continue;

    limits = hard ? lev->hard_limit : lev->soft_limit;

    if (limits == 0)
      continue;

    for (one_lim = (1<<20); one_lim; one_lim >>= 1)
    {
      if (limits & one_lim)
        ReportOneOverflow(cur, one_lim, hard);
    }
  }
}

//
// ReportV3Switches
//
void ReportV3Switches(void)
{
  PrintMsg(
    "V3 FORMAT UPGRADES.  The following levels require a Doom port\n"
    "which supports V3 GL-Nodes, otherwise they will fail (or crash).\n\n"
  );

  lump_t *cur;

  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    level_t *lev = cur->lev_info;

    if (! (lev && ! (lev->flags & LEVEL_IS_GL)))
      continue;

    if (lev->v3_switch == 0)
      continue;

    int v3 = lev->v3_switch;

    if (v3 & LIMIT_VERTEXES)
    {
      PrintMsg("%-6s : Number of vertices overflowed the limit.\n", cur->name);
    }

    if (v3 & LIMIT_GL_SSECT)
    {
      PrintMsg("%-6s : Number of GL segs overflowed the limit.\n", cur->name);
    }
  }
}

//
// ReportFailedLevels
//
void ReportFailedLevels(void)
{
  lump_t *cur;
  int lev_count = 0;

  int fail_soft = 0;
  int fail_hard = 0;
  int fail_v3   = 0;

  bool need_spacer = false;
 
  for (cur=wad.dir_head; cur; cur=cur->next)
  {
    if (! (cur->lev_info && ! (cur->lev_info->flags & LEVEL_IS_GL)))
      continue;

    lev_count++;

    if (cur->lev_info->soft_limit != 0) fail_soft++;
    if (cur->lev_info->hard_limit != 0) fail_hard++;
    if (cur->lev_info->v3_switch  != 0) fail_v3++;
  }

  PrintMsg("\n");

  if (fail_soft + fail_hard + fail_v3 == 0)
  {
    PrintMsg("All levels were built successfully.\n");
    return;
  }

  PrintMsg("*** Problem Report ***\n\n");

  if (fail_soft > 0)
  {
    ReportOverflows(false);
    need_spacer = true;
  }

  if (fail_v3 > 0)
  {
    if (need_spacer)
      PrintMsg("\n");

    ReportV3Switches();
    need_spacer = true;
  }

  if (fail_hard > 0)
  {
    if (need_spacer)
      PrintMsg("\n");

    ReportOverflows(true);
  }

  PrintMsg("\nEnd of problem report.\n");
}

