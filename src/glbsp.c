//------------------------------------------------------------------------
// MAIN : Main program for glBSP
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

#include "blockmap.h"
#include "level.h"
#include "node.h"
#include "seg.h"
#include "structs.h"
#include "util.h"
#include "wad.h"


const nodebuildinfo_t *cur_info = NULL;
const nodebuildfuncs_t *cur_funcs = NULL;
volatile nodebuildcomms_t *cur_comms = NULL;

int cur_build_pos;
int cur_file_pos;

static char glbsp_message_buf[1024];


const nodebuildinfo_t default_buildinfo =
{
  NULL,    // input_file
  NULL,    // output_file

  DEFAULT_FACTOR,  // factor

  FALSE,   // no_reject
  FALSE,   // no_progress
  FALSE,   // mini_warnings
  FALSE,   // hexen_mode
  FALSE,   // pack_sides
  FALSE,   // v1_vert

  FALSE,   // load_all
  FALSE,   // no_gl
  FALSE,   // no_normal
  FALSE,   // force_normal
  FALSE,   // gwa_mode
  FALSE,   // keep_sect
  FALSE,   // no_prune

  DEFAULT_BLOCK_LIMIT   // block_limit
};

const nodebuildcomms_t default_buildcomms =
{
  NULL,   // message
  FALSE   // cancelled
};


/* ----- option parsing ----------------------------- */

#define HANDLE_BOOLEAN(name, field)  \
    if (StrCaseCmp(opt_str, name) == 0)  \
    {  \
      info->field = TRUE;  \
      argv++; argc--;  \
      continue;  \
    }

glbsp_ret_e GlbspParseArgs(nodebuildinfo_t *info, 
    volatile nodebuildcomms_t *comms,
    const char ** argv, int argc)
{
  const char *opt_str;

  int num_files = 0;

  while (argc > 0)
  {
    if (argv[0][0] != '-')
    {
      // --- ORDINARY FILENAME ---

      if (num_files >= 1)
      {
        comms->message = "Too many filenames.  Use the -o option";
        return GLBSP_E_BadArgs;
      }

      info->input_file = argv[0];
      num_files++;

      argv++; argc--;
      continue;
    }

    // --- AN OPTION ---

    opt_str = &argv[0][1];

    // handle GNU style options beginning with `--'
    if (opt_str[0] == '-')
      opt_str++;

    if (StrCaseCmp(opt_str, "o") == 0)
    {
      if (argc < 2)
      {
        comms->message = "Missing filename for the -o option";
        return GLBSP_E_BadArgs;
      }

      info->output_file = argv[1];

      argv += 2; argc -= 2;
      continue;
    }

    if (StrCaseCmp(opt_str, "factor") == 0)
    {
      if (argc < 2)
      {
        comms->message = "Missing factor value";
        return GLBSP_E_BadArgs;
      }

      info->factor = (int) strtol(argv[1], NULL, 10);

      argv += 2; argc -= 2;
      continue;
    }

    if (StrCaseCmp(opt_str, "maxblock") == 0)
    {
      if (argc < 2)
      {
        comms->message = "Missing maxblock value";
        return GLBSP_E_BadArgs;
      }

      info->block_limit = (int) strtol(argv[1], NULL, 10);

      argv += 2; argc -= 2;
      continue;
    }

    HANDLE_BOOLEAN("noreject",    no_reject)
    HANDLE_BOOLEAN("noprog",      no_progress)
    HANDLE_BOOLEAN("warn",        mini_warnings)
    HANDLE_BOOLEAN("hexen",       hexen_mode)
    HANDLE_BOOLEAN("packsides",   pack_sides)
    HANDLE_BOOLEAN("v1",          v1_vert)

    HANDLE_BOOLEAN("loadall",     load_all)
    HANDLE_BOOLEAN("nogl",        no_gl)
    HANDLE_BOOLEAN("nonormal",    no_normal)
    HANDLE_BOOLEAN("forcenormal", force_normal)
    HANDLE_BOOLEAN("forcegwa",    gwa_mode)
    HANDLE_BOOLEAN("keepsect",    keep_sect)
    HANDLE_BOOLEAN("noprune",     no_prune)

    sprintf(glbsp_message_buf, "Unknown option: %s", argv[0]);
    comms->message = (const char *) glbsp_message_buf;
    return GLBSP_E_BadArgs;
  }

  return GLBSP_E_OK;
}

glbsp_ret_e GlbspCheckInfo(nodebuildinfo_t *info,
    volatile nodebuildcomms_t *comms)
{
  glbsp_ret_e retval = GLBSP_E_OK;

  comms->message = NULL;

  if (!info->input_file)
  {
    comms->message = "INTERNAL ERROR: Missing input filename !";
    return GLBSP_E_BadArgs;
  }

  if (!info->output_file)
  {
    info->output_file = ReplaceExtension(info->input_file, "gwa");
    info->gwa_mode = 1;

    //!!! PrintMsg("* No output file specified. Using: %s\n\n", info->output_file);
  }
  else if (CheckExtension(info->output_file, "gwa"))
    info->gwa_mode = 1;

  if (strcmp(info->input_file, info->output_file) == 0)
  {
    //!!! PrintMsg("* Output file is same as input file. Using -loadall\n\n");
    info->load_all = 1;
  }

  if (info->no_prune && info->pack_sides)
  {
    info->pack_sides = FALSE;
    comms->message = "-noprune and -packsides cannot be used together";
    retval = GLBSP_E_BadArgs;
  }

  if (info->gwa_mode && info->no_gl)
  {
    info->no_gl = FALSE;
    comms->message = "-nogl with GWA file: nothing to do !";
    retval = GLBSP_E_BadArgs;
  }
 
  if (info->gwa_mode && info->force_normal)
  {
    info->force_normal = FALSE;
    comms->message = "-forcenormal used, but GWA files don't have normal nodes";
    retval = GLBSP_E_BadArgs;
  }
 
  if (info->no_normal && info->force_normal)
  {
    info->force_normal = FALSE;
    comms->message = "-forcenormal and -nonormal cannot be used together";
    retval = GLBSP_E_BadArgs;
  }
  
  if (info->factor <= 0)
  {
    info->factor = DEFAULT_FACTOR;
    comms->message = "Bad factor value !";
    retval = GLBSP_E_BadArgs;
  }

  if (info->block_limit < 1000 || info->block_limit > 64000)
  {
    info->block_limit = DEFAULT_BLOCK_LIMIT;
    comms->message = "Bad blocklimit value !";
    retval = GLBSP_E_BadArgs;
  }

  return retval;
}


/* ----- build nodes for a single level --------------------------- */

static glbsp_ret_e HandleLevel(void)
{
  superblock_t *seg_list;
  node_t *root_node;
  subsec_t *root_sub;

  glbsp_ret_e ret;

  if (cur_comms->cancelled)
    return GLBSP_E_Cancelled;

  DisplaySetBarLimit(1, 100);
  DisplaySetBar(1, 0);

  cur_build_pos = 0;

  LoadLevel();

  InitBlockmap();

  // create initial segs
  seg_list = CreateSegs();

  // recursively create nodes
  ret = BuildNodes(seg_list, &root_node, &root_sub, 0);
  FreeSuper(seg_list);

  if (ret == GLBSP_E_OK)
  {
    ClockwiseBspTree(root_node);

    PrintMsg("Built %d NODES, %d SSECTORS, %d SEGS, %d VERTEXES\n",
        num_nodes, num_subsecs, num_segs, num_normal_vert + num_gl_vert);

    if (root_node)
      PrintMsg("Heights of left and right subtrees = (%d,%d)\n",
          ComputeHeight(root_node->r.node), ComputeHeight(root_node->l.node));

    SaveLevel(root_node);
  }

  FreeLevel();
  FreeQuickAllocCuts();
  FreeQuickAllocSupers();

  return ret;
}


/* ----- main routine -------------------------------------- */

glbsp_ret_e GlbspBuildNodes(const nodebuildinfo_t *info,
    const nodebuildfuncs_t *funcs, volatile nodebuildcomms_t *comms)
{
  char strbuf[256];

  glbsp_ret_e ret = GLBSP_E_OK;

  cur_info  = info;
  cur_funcs = funcs;
  cur_comms = comms;

  total_big_warn = total_small_warn = 0;

  // sanity check
  if (!cur_info->input_file || !cur_info->output_file)
  {
    comms->message = "INTERNAL ERROR: Missing in/out filename !";
    return GLBSP_E_BadArgs;
  }

  if (cur_info->no_normal && cur_info->no_gl)
  {
    comms->message = "-nonormal and -nogl specified: nothing to do !";
    return GLBSP_E_BadArgs;
  }
  
  InitDebug();
  
  // opens and reads directory from the input wad
  ReadWadFile(cur_info->input_file);

  if (CountLevels() <= 0)
  {
    //!!!! Error("No levels found in wad")
    CloseWads();
    TermDebug();

    return GLBSP_E_Unknown;
  }
   
  PrintMsg("\nCreating nodes using tunable factor of %d\n", info->factor);

  DisplayOpen(DIS_BUILDPROGRESS);
  DisplaySetTitle("glBSP Progress");

  sprintf(strbuf, "File: %s", cur_info->input_file);
  
  DisplaySetBarText(2, strbuf);
  DisplaySetBarLimit(2, CountLevels());
  DisplaySetBar(2, 0);

  cur_file_pos = 0;
  
  // loop over each level in the wad
  while (FindNextLevel())
  {
    ret = HandleLevel();

    if (ret != GLBSP_E_OK)
      break;

    cur_file_pos++;
    DisplaySetBar(2, cur_file_pos);
  }

  DisplayClose();

  // writes all the lumps to the output wad
  if (ret == GLBSP_E_OK)
    WriteWadFile(cur_info->output_file);

  // close wads and free memory
  CloseWads();

  PrintMsg("\nTotal serious warnings: %d\n", total_big_warn);
  PrintMsg("Total minor warnings: %d\n", total_small_warn);

  TermDebug();

  return ret;
}

