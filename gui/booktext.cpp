//------------------------------------------------------------------------
// BOOKTEXT : Unix/FLTK Manual Text
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

#include "local.h"

// forward decls
static const char * contents_text[];
static const char * intro_text[];
static const char * using_text[];
static const char * option_text[];
static const char * note_tc_text[];
static const char * howwork_text[];
static const char * diff_text[];
static const char * contact_text[];
static const char * acknow_text[];

#define GRN  "@C60"


const book_page_t book_pages[] =
{
  { contents_text },   // #00
  { intro_text },      // #01
  { using_text },      // #02
  { option_text },     // #03
  { note_tc_text },    // #04
  { howwork_text },    // #05
  { diff_text },       // #06
  { contact_text },    // #07
  { acknow_text },     // #08
  { NULL }
};


//------------------------------------------------------------------------

static const char *contents_text[] =
{
  "",
  "@c@l@b glbspX Manual",
  "",
  "@-",
  "",
  "@r by Andrew Apted   ",
  "@r Updated: 14th July 2001   ",
  "",
  "@c@m Table of Contents",
  "",
  "#L01@c" GRN " 1. Introduction",
  "#L02@c" GRN " 2. Using glbspX",
  "#L03@c" GRN " 3. Options and Menus",
  "#L04@c" GRN " 4. Notes for TC authors",
  "#L05@c" GRN " 5. How glBSP works",
  "#L06@c" GRN " 6. Differences to BSP 2.3",
  "#L07@c" GRN " 7. Contact and Links",
  "#L08@c" GRN " 8. Acknowledgements",
  NULL
};


//------------------------------------------------------------------------

static const char *intro_text[] =
{
  "",
  "@c@l 1. Introduction",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *using_text[] =
{
  "",
  "@c@l 2. Using glbspX",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *option_text[] =
{
  "",
  "@c@l 3. Options and Menus",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *note_tc_text[] =
{
  "",
  "@c@l 4. Notes for TC authors",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *howwork_text[] =
{
  "",
  "@c@l 5. How glBSP works",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *diff_text[] =
{
  "",
  "@c@l 6. Differences to BSP 2.3",
  "",
  "@-",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *contact_text[] =
{
  "",
  "@c@l 7. Contact and Links",
  "",
  "@-",
  "",
  "  The homepage for glBSP can be found here:",
  "",
  "@t     http://glbsp.sourceforge.net/",
  "",
  "  Questions, bug reports, suggestions, etc... can be sent to the",
  "  following email address:",
  "",
  "@t     glbsp-public@lists.sourceforge.net",
  "",
  "",
  "@m Compatible Engines",
  "",
  "@t   EDGE     http://edge.sourceforge.net/",
  "@t   JDOOM    http://www.newdoom.com/jdoom",
  "@t   Doom3D   http://www.redrival.com/fudgefactor/doom3d.html",
  "@t   PrBOOM   http://prboom.sourceforge.net/",
  "@t   Vavoom   http://www.raven-games.com/hosted/vavoom",
  "",
  NULL
};


//------------------------------------------------------------------------

static const char *acknow_text[] =
{
  "",
  "@c@l 8. Acknowledgements",
  "",
  "@-",
  "",
  "  Andy Baker, for making binaries, writing code and other help.",
  "",
  "  Marc A. Pullen, for testing and helping with the documentation.",
  "",
  "  Lee Killough and André Majorel, for giving their permission to put",
  "  glBSP under the GNU GPL.",
  "",
  "  Janis Legzdinsh for fixing many problems with Hexen wads.",
  "",
  "  Jaakko Keränen, who gave some useful feedback on the GL-Friendly",
  "  Nodes specification.",
  "",
  "  The authors of FLTK (Fast Light Tool Kit), for a nice LGPL C++ GUI",
  "  toolkit that even I can get working on both Linux and Win32.",
  "",
  "  Marc Rousseau (author of ZenNode 1.0), Robert Fenske Jr (author of",
  "  Warm 1.6), L.M. Witek (author of Reject 1.1), and others, for",
  "  releasing the source code to their WAD utilities, and giving me lots",
  "  of ideas to \"borrow\" :), like blockmap packing.",
  "",
  "  Colin Reed and Lee Killough (and others), who wrote the original BSP",
  "  2.3 which glBSP is based on.",
  "",
  "  Matt Fell, for the Doom Specs v1.666.",
  "",
  "  Raphael Quinet, for DEU and the original idea.",
  "",
  "  id Software, for not only creating such an irresistable game, but",
  "  releasing the source code for so much of their stuff.",
  "",
  "  . . . and everyone else who deserves it ! ",
  "",
  NULL
};

