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

// this includes everything we need
#include "defs.h"


static FILE *in_file = NULL;


#define DEBUG_DIR   1
#define DEBUG_LUMP  0

#define APPEND_BLKSIZE  256
#define LEVNAME_BUNCH   20

#define ALIGN_LEN(len)  ((((len) + 3) / 4) * 4)


// current wad info
static wad_c *wad;


wad_c::wad_c() :
	kind(-1), num_entries(0), dir_start(-1),
	dir(), current_level(NULL),
	level_names(NULL), num_level_names(0)
{
}

wad_c::~wad_c()
{
	/* free directory entries */
	for (lump_c *cur = (lump_c *)dir.pop_front(); cur != NULL;
	             cur = (lump_c *)dir.pop_front())
	{
		delete cur;
	}

	/* free the level names */
	if (level_names)
	{
		for (int i=0; i < num_level_names; i++)
			UtilFree((void *) level_names[i]);

		UtilFree(level_names);
	}
}

level_c::level_c() :
	flags(0), children(), buddy(NULL),
	soft_limit(0), hard_limit(0), v3_switch(0)
{
}

level_c::~level_c()
{
	for (lump_c *cur = (lump_c *)children.pop_front(); cur != NULL;
	             cur = (lump_c *)children.pop_front())
	{
		delete cur;
	}
}

lump_c::lump_c() :
	name(NULL), start(-1), new_start(-1), length(0), space(0),
	flags(LUMP_NEW), data(NULL), lev_info(NULL)
{
}

lump_c::~lump_c()
{
	delete lev_info;

	if (data)
		UtilFree((void*)data);

	if (name)
		UtilFree((void*)name);
}

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

	for (n=0; n < wad->num_level_names; n++)
	{
		if (strcmp(wad->level_names[n], name) == 0)
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
	if ((wad->num_level_names % LEVNAME_BUNCH) == 0)
	{
		wad->level_names = (const char **) UtilRealloc((void *)wad->level_names,
				(wad->num_level_names + LEVNAME_BUNCH) * sizeof(const char *));
	}

	wad->level_names[wad->num_level_names] = UtilStrDup(name);
	wad->num_level_names++;
}


//
// NewLevel
//
// Create new level information
//
static level_c *NewLevel(int flags)  // FIXME @@@@
{
	level_c *cur = new level_c();

	cur->flags = flags;

	return cur;
}


//
// NewLump
//
// Create new lump.  `name' must be allocated storage.
//
static lump_c *NewLump(const char *name)  // FIXME @@@@
{
	lump_c *cur = new lump_c();

	cur->name = name;

	return cur;
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

///		SetErrorMsg(strbuf);
		return false;
	}

	if (! CheckMagic(header.type))
	{
		sprintf(strbuf, "%s does not appear to be a wad file : bad magic", 
				filename);

///		SetErrorMsg(strbuf);
		return false;
	}

	wad = new wad_c();

	wad->kind = (header.type[0] == 'I') ? IWAD : PWAD;

	wad->num_entries = UINT32(header.num_entries);
	wad->dir_start   = UINT32(header.dir_start);

	return true;
}


//
// ReadDirEntry
//
static void ReadDirEntry(void)
{
	size_t len;
	raw_wad_entry_t entry;
	lump_c *lump;

	len = fread(&entry, sizeof(entry), 1, in_file);

	if (len != 1)
		FatalError("Trouble reading wad directory");

	lump = NewLump(UtilStrNDup(entry.name, 8));

	lump->start  = UINT32(entry.start);
	lump->length = UINT32(entry.length);

# if DEBUG_DIR
	PrintDebug("Read dir... %s\n", lump->name);
# endif

	wad->dir.push_back(lump);
}

//
// DetermineLevelNames
//
static void DetermineLevelNames(void)
{
	lump_c *L, *N;
	int i;

	for (L = (lump_c*)wad->dir.begin(); L; L = (lump_c*)L->NodeNext())
	{
		// check if the next four lumps after the current lump match the
		// level-lump names.
		//
		for (i=0, N = (lump_c*)L->NodeNext(); (i < 4) && N; i++, N = (lump_c*)N->NodeNext())
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
static void ProcessDirEntry(lump_c *lump)
{
	// FIXME !!! handle separate GL_MAPxx markers

	// ignore previous GL lump info
	if (false) /// CheckGLLumpName(lump->name))
	{
#   if DEBUG_DIR
		PrintDebug("Discarding previous GL info: %s\n", lump->name);
#   endif

		delete lump;

		wad->num_entries--;
		return;
	}

	// --- LEVEL MARKERS ---

	if (CheckLevelName(lump->name))
	{
		/* NOTE !  Level marks can have data (in Hexen anyway) */
		lump->flags |= LUMP_COPY_ME;

		// OK, start a new level

		lump->lev_info = NewLevel(0);

		wad->current_level = lump;

#   if DEBUG_DIR
		PrintDebug("Process dir... %s :\n", lump->name);
#   endif

		wad->dir.push_back(lump);
		return;
	}

	// --- LEVEL LUMPS ---

	if (wad->current_level)
	{
		if (CheckLevelLumpName(lump->name) || CheckGLLumpName(lump->name))
		{
			// check for duplicates
			if (FindLevelLump(lump->name))
			{
				PrintWarn("Duplicate entry `%s' ignored in %s\n",
						lump->name, wad->current_level->name);

				delete lump;

				wad->num_entries--;
				return;
			}

#     if DEBUG_DIR
			PrintDebug("Process dir... |--- %s\n", lump->name);
#     endif

			// mark it to be loaded
			lump->flags |= LUMP_READ_ME;

			// link it in
			wad->current_level->lev_info->children.push_back(lump);
			return;
		}

		// OK, non-level lump.  End the previous level.

		wad->current_level = NULL;
	}

	// --- ORDINARY LUMPS ---

# if DEBUG_DIR
	PrintDebug("Process dir... %s\n", lump->name);
# endif

	if (CheckLevelLumpName(lump->name))
		PrintWarn("Level lump `%s' found outside any level\n", lump->name);

	// maybe load data
	lump->flags |= LUMP_COPY_ME;

	// link it in
	wad->dir.push_back(lump);
}

//
// ReadDirectory
//
static void ReadDirectory(void)
{
	int i;
	int total_entries = wad->num_entries;

	fseek(in_file, wad->dir_start, SEEK_SET);

	for (i=0; i < total_entries; i++)
	{
		ReadDirEntry();
	}

	DetermineLevelNames();

	// finally, unlink all lumps and process each one in turn

	list_c temp(wad->dir);

	wad->dir.clear();

	for (lump_c *cur = (lump_c *) temp.pop_front(); cur;
	             cur = (lump_c *) temp.pop_front())
	{
		ProcessDirEntry(cur);
	}
}


//
// ReadLumpData
//
static void ReadLumpData(lump_c *lump)
{
	size_t len;

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
		if (wad->current_level)
			PrintWarn("Trouble reading lump `%s' in %s\n",
					lump->name, wad->current_level->name);
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
	lump_c *cur, *L;
	int count = 0;

	for (cur = (lump_c*)wad->dir.begin(); cur; cur = (lump_c*)cur->NodeNext())
	{
		count++;

		if (cur->flags & LUMP_READ_ME)
			ReadLumpData(cur);

		if (cur->lev_info && ! (cur->lev_info->flags & LEVEL_IS_GL))
		{
			for (L = (lump_c*)cur->lev_info->children.begin(); L; L = (lump_c*)L->NodeNext())
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
// FindLevel
//
bool FindLevel(const char *map_name)
{
	lump_c *cur;

	for (cur = (lump_c*)wad->dir.begin(); cur; cur = (lump_c*)cur->NodeNext())
	{
		if (! cur->lev_info)
			continue;

		if (cur->lev_info->flags & LEVEL_IS_GL)  // @@@@
			continue;

		if (UtilStrCaseCmp(cur->name, map_name) == 0)
			break;
	}

	wad->current_level = cur;

	return (cur != NULL);
}

//
// GetLevelName
//
const char *GetLevelName(void)
{
	if (!wad->current_level)
		InternalError("GetLevelName: no current level");

	return wad->current_level->name;
}

//
// FindLevelLump
//
lump_c *FindLevelLump(const char *name)
{
	lump_c *cur = (lump_c*)wad->current_level->lev_info->children.begin();

	while (cur && (strcmp(cur->name, name) != 0))
		cur = (lump_c*)cur->NodeNext();

	return cur;
}

//
// CheckLevelLumpZero
//
bool CheckLevelLumpZero(lump_c *lump)
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

///		SetErrorMsg(strbuf);
		return -1;
	}

	if (! ReadHeader(filename))
	{
		fclose(in_file);
		return -1;
	}

	PrintMsg("Opened %cWAD file : %s\n", (wad->kind == IWAD) ? 'I' : 'P', 
			filename); 
	PrintMsg("Reading %d dir entries at 0x%X\n", wad->num_entries, 
			wad->dir_start);

	// read directory
	ReadDirectory();

	sprintf(strbuf, "Reading: %s", filename);

	// now read lumps
	check = ReadAllLumps();

	if (check != wad->num_entries)
		InternalError("Read directory count consistency failure (%d,%d)",
				check, wad->num_entries);

	wad->current_level = NULL;

	return 0;
}



//
// CloseWads
//
void CloseWads(void)
{
	if (in_file)
	{
		fclose(in_file);
		in_file = NULL;
	}
}

