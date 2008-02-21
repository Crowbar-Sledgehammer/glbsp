//------------------------------------------------------------------------
//  TEXTURE handling
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

// this includes everything we need
#include "defs.h"


typedef std::map<std::string, std::string> texture_map_t;

static texture_map_t tex_DB;

static const char *default_tex_name;


typedef std::map<int, std::string> entity_map_t;

static entity_map_t ent_DB;


static void ParseTexture(char *pos)
{
  char *old_name = strtok(pos, " \t");

  if (! old_name)
  {
    fprintf(stderr, "Bad texture line in convdefs\n");
    return;
  }

  char *new_name = strtok(NULL, " \t");

  if (! new_name)
  {
    fprintf(stderr, "Bad texture line in convdefs\n");
    return;
  }

  // make source texture name uppercase
  for (char *s = old_name; *s; s++)
    *s = toupper(*s);

  if (strcmp(old_name, "DEFAULT") == 0)
  {
    default_tex_name = strdup(new_name);
    return;
  }

  tex_DB[old_name] = std::string(new_name);
}


static void ParseEntity(char *pos)
{
  const char *old_name = strtok(pos, " \t");

  if (! old_name)
  {
    fprintf(stderr, "Bad entity line in convdefs\n");
    return;
  }

  const char *new_name = strtok(NULL, " \t");

  if (! new_name)
  {
    fprintf(stderr, "Bad entity line in convdefs\n");
    return;
  }

  int id_number = atoi(old_name);

  if (id_number <= 0)
  {
    fprintf(stderr, "Bad entity number '%s' in convdefs\n", old_name);
    return;
  }

  ent_DB[id_number] = std::string(new_name);
}


void Texture_Load(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (! fp)
    FatalError("Cannot open definitions file: %s\n", filename);

  char line_buf[1024];

  while (fgets(line_buf, sizeof(line_buf)-10, fp) != NULL)
  {
    char *pos = line_buf;

    while (*pos && isspace(*pos))
      pos++;

    // skip blank lines and comments
    if (*pos == 0 || *pos == '/' || *pos == '#')
      continue;

    if (strncmp(pos, "texture", 7) == 0)
    {
      ParseTexture(pos+7);
    }
    else if (strncmp(pos, "entity", 6) == 0)
    {
      ParseEntity(pos+6);
    }
    else
    {
      fprintf(stderr, "Warning: unrecognised line in %s:\n%s\n\n",
              filename, line_buf);
    }
  }

  fclose(fp);

  if (! default_tex_name)
    FatalError("No 'DEFAULT' texture entry!\n");
}


const char * Texture_Convert(const char *old_name, bool is_flat)
{
  if (tex_DB.find(old_name) == tex_DB.end())
  {
    fprintf(stderr, "Warning: unspecified texture '%s'\n", old_name);

    SYS_ASSERT(default_tex_name);

    tex_DB[old_name] = std::string(default_tex_name);
  }

  return tex_DB[old_name].c_str();
}


const char * Entity_Convert(int id_number)
{
  if (ent_DB.find(id_number) != ent_DB.end())
  {
    return ent_DB[id_number].c_str();
  }

  // built-in values
  if (id_number == 1)
    return "info_player_start";

  if (id_number == 11)
    return "info_player_deathmatch";

  return NULL; // not found
}

