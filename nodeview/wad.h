//------------------------------------------------------------------------
//  WAD : Wad file read/write functions.
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

#ifndef __NODEVIEW_WAD_H__
#define __NODEVIEW_WAD_H__


class lump_c;


typedef enum { IWAD, PWAD } wad_kind_e;

// wad header

class wad_c
{
public:
	wad_c();
	~wad_c();

	// kind of wad file
	int kind;

	// number of entries in directory
	int num_entries;

	// offset to start of directory
	int dir_start;

	// current directory entries
	list_c dir;

	// current level
	lump_c *current_level;

	// array of level names found
	const char ** level_names;
	int num_level_names;
};


// level information

class level_c
{
public:
	level_c();
	~level_c();

	// various flags
	int flags;

	// the child lump list
	list_c children;

	// for normal levels, this is the associated GL level lump
	lump_c *buddy;

	// information on overflow
	int soft_limit;
	int hard_limit;
	int v3_switch;
};

/* this level information holds GL lumps */
#define LEVEL_IS_GL   0x0002


// directory entry

class lump_c : public listnode_c
{
public:
	lump_c();
	~lump_c();

	// name of lump
	const char *name;

	// offset to start of lump
	int start;
	int new_start;

	// length of lump
	int length;
	int space;

	// various flags
	int flags;

	// data of lump
	void *data;

	// level information, usually NULL
	level_c *lev_info;
};

/* this lump should be copied from the input wad */
#define LUMP_COPY_ME       0x0004

/* this lump shouldn't be written to the output wad */
#define LUMP_IGNORE_ME     0x0008

/* this lump needs to be loaded */
#define LUMP_READ_ME       0x0100

/* this lump is new (didn't exist in the original) */
#define LUMP_NEW           0x0200


/* ----- function prototypes --------------------- */

// open the input wad file and read the contents into memory.  When
// `load_all' is false, lumps other than level info will be marked as
// copyable instead of loaded.
//
// Returns 0 if all went well, otherwise an error code (in
// which case cur_comms->message has been set and all files/memory
// have been freed).
//
int ReadWadFile(const char *filename);

// close all wad files and free any memory.
void CloseWads(void);

// find a particular level in the wad directory, and store the
// reference in `wad.current_level'.  Returns false if not
// found.
//
bool FindLevel(const char *map_name);

// return the current level name
const char *GetLevelName(void);

// find the level lump with the given name in the current level, and
// return a reference to it.  Returns NULL if no such lump exists.
// Level lumps are always present in memory (i.e. never marked
// copyable).
//
lump_c *FindLevelLump(const char *name);

// tests if the level lump contains nothing but zeros.
bool CheckLevelLumpZero(lump_c *lump);


/* ----- conversion macros ----------------------- */

#define UINT8(x)   ((uint8_g) (x))
#define SINT8(x)   ((sint8_g) (x))

#define UINT16(x)  Endian_U16(x)
#define UINT32(x)  Endian_U32(x)

#define SINT16(x)  ((sint16_g) Endian_U16((uint16_g) (x)))
#define SINT32(x)  ((sint32_g) Endian_U32((uint32_g) (x)))


#endif /* __NODEVIEW_WAD_H__ */
