//------------------------------------------------------------------------
// GLBSP.H : Interface to Node Builder
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

#ifndef __GLBSP_GLBSP_H__
#define __GLBSP_GLBSP_H__


#define GLBSP_VER  "1.94"


// certain GCC attributes can be useful
#ifndef GCCATTR
#define GCCATTR(attr)  /* nothing */
#endif


/* ----- basic types --------------------------- */

typedef char  sint8_g;
typedef short sint16_g;
typedef int   sint32_g;
   
typedef unsigned char  uint8_g;
typedef unsigned short uint16_g;
typedef unsigned int   uint32_g;

typedef double float_g;
typedef double angle_g;  // degrees, 0 is E, 90 is N

// boolean type
typedef int boolean_g;

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif


/* ----- complex types --------------------------- */

// FIXME: need semantics for memory usage !

// Node Build Information Structure 
typedef struct nodebuildinfo_s
{
  const char *input_file;
  const char *output_file;

  int factor;

  boolean_g no_reject;
  boolean_g no_progress;
  boolean_g mini_warnings;
  boolean_g hexen_mode;
  boolean_g pack_sides;
  boolean_g v1_vert;

  boolean_g load_all;
  boolean_g no_gl;
  boolean_g no_normal;
  boolean_g force_normal;
  boolean_g gwa_mode;  
  boolean_g keep_sect;
  boolean_g no_prune;

  int block_limit;

  // private stuff -- values computed in GlbspParseArgs or
  // GlbspCheckInfo that need to be passed to GlbspBuildNodes.

  boolean_g missing_output;
  boolean_g same_filenames;
}
nodebuildinfo_t;

// This is for two-way communication (esp. with the GUI).
// Should be flagged `volatile' since multiple threads (real or
// otherwise, e.g. signals) may read or change the values.
//
typedef struct nodebuildcomms_s
{
  // if the node builder failed, this will contain the error
  const char *message;

  // the GUI can set this to tell the node builder to stop
  boolean_g cancelled;
}
nodebuildcomms_t;


// Display Prototypes
typedef enum
{
  DIS_INVALID,        // Nonsense value is always useful
  DIS_BUILDPROGRESS,  // Build Box, has 2 bars
  DIS_FILEPROGRESS,   // File Box, has 1 bar
  NUMOFGUITYPES
}
displaytype_e;

// Callback functions
typedef struct nodebuildfuncs_s
{
  // FIXME: COMMENT ALL OF THESE
  void (* fatal_error)(const char *str, ...);
  void (* print_msg)(const char *str, ...);
  void (* ticker)(void);

  boolean_g (* display_open)(displaytype_e type);
  void (* display_setTitle)(const char *str);
  void (* display_setText)(const char *str);
  void (* display_setBar)(int barnum, int count);
  void (* display_setBarLimit)(int barnum, int limit);
  void (* display_setBarText)(int barnum, const char *str);
  void (* display_close)(void);
}
nodebuildfuncs_t;

// Default build info and comms
extern const nodebuildinfo_t default_buildinfo;
extern const nodebuildcomms_t default_buildcomms;


/* -------- engine prototypes ----------------------- */

typedef enum
{
  // everything went peachy keen
  GLBSP_E_OK = 0,

  // an unknown error occurred (this is the catch-all value)
  GLBSP_E_Unknown,

  // the arguments were bad/inconsistent.
  GLBSP_E_BadArgs,

  // file errors
  GLBSP_E_ReadError,
  GLBSP_E_WriteError,

  // building was cancelled
  GLBSP_E_Cancelled
}
glbsp_ret_e;

// parses the arguments, modifying the `info' structure accordingly.
// Returns GLBSP_E_OK if all went well, otherwise another error code.
// Upon error, comms->message may be set to an string describing the
// error.  Typical errors are unrecognised options and invalid option
// values.  Calling this routine is not compulsory.  Note that the set
// of arguments does not include the program's name.
//
glbsp_ret_e GlbspParseArgs(nodebuildinfo_t *info,
    volatile nodebuildcomms_t *comms,
    const char ** argv, int argc);

// checks the node building parameters in `info'.  If they are valid,
// returns GLBSP_E_OK, otherwise an error code is returned and the
// parameters are updated to something that is valid.  Note: when
// `output_file' is NULL, this routine will update it to the correct
// GWA filename (and set the gwa_mode flag).  This routine should
// always be called shortly before GlbspBuildNodes().
//
glbsp_ret_e GlbspCheckInfo(nodebuildinfo_t *info,
    volatile nodebuildcomms_t *comms);

// main routine, this will build the nodes (GL and/or normal) for the
// given input wad file out to the given output file.  Returns
// GLBSP_E_OK if everything went well, otherwise another error code.
// Typical errors are fubar parameters (like input_file == NULL),
// problems reading/writing, or cancellation by another thread (esp.
// the GUI) using the comms->cancelled flag.  Upon errors, the
// comms->message field may contain a string describing the error.
//
glbsp_ret_e GlbspBuildNodes(const nodebuildinfo_t *info,
    const nodebuildfuncs_t *funcs, 
    volatile nodebuildcomms_t *comms);


#endif /* __GLBSP_GLBSP_H__ */
