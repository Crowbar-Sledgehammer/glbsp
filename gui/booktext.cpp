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

#define GRN  "@C59"   // 60 is better, but selection looks bad


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
  "@r Updated: 18th July 2001   ",
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
  "#P00",
  "glBSP is a nodes builder specially designed to be used with OpenGL",
  "ports of the DOOM game engine. It adheres to the \"GL-Friendly Nodes\"",
  "specification, which means it adds some new special nodes to a WAD",
  "file that makes it very easy (and fast#-!) for an OpenGL DOOM engine to",
  "compute the polygons needed for drawing the levels.",
  "",
  "#P00",
  "There are many DOOM ports that understand the GL Nodes which glBSP",
  "creates, including: EDGE, the Doomsday engine (JDOOM), Doom3D, PrBoom,",
  "and Vavoom.#- See the Contact and Links page.",
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
  "@m@b  Build Modes",
  "",
  "#P00",
  "There are five different build modes in glbspX, which can be",
  "selected in the upper left box in the main window.",
  "Here is a description of them:",
  "",
  "@t    GWA Mode",
  "#P30",
  "this blah blah blas can fo ofoo tororooro and pdfsd qewrqwer",
  "adf qwerqwer adf aag asg asg foo asdf qr far asdfadsf pooh bar",
  "tribbly doo quad.",
  "",
  "@t    GL, Normal if missing",
  "#P30",
  "Blah.",
  "",
  "@t    GL and Normal nodes",
  "#P30",
  "Blah.",
  "",
  "@t    GL nodes only",
  "#P30",
  "Blah.",
  "",
  "@t    Normal nodes only",
  "#P30",
  "Blah.",
  
//The following options are recognised:
//
//  -factor <num> 
//               Changes the cost assigned to seg splits.  Factor can
//               be any number greater than 0, larger values make seg
//               splits more costly (and thus glBSP tries harder to
//               avoid them), but smaller values produce better BSP
//               trees.  See the section below for more info.  The
//               default value (5) is known to be a good compromise.
//
//  -noreject    Normally glBSP will create an simple REJECT map for
//               each level.  This options prevents any existing
//               REJECT map from being clobbered.
//
//  -noprog      Does not show the progress indicator.
//
//  -warn        Shows extra warning messages, which detail various
//               non-serious problems that glBSP has while analysing the
//               level structure.  Often these warnings show a real
//               problem in the level (e.g. a non-closed sector or
//               invalid sidedef), so they are worth checking now and
//               then.
//
//  -packsides   Pack sidedefs, by detecting which sidedefs are
//               identical and removing the duplicates, producing a
//               smaller PWAD.  Note: this may make your level a lot
//               harder to edit !  Therefore this option is most useful
//               when producing a final WAD for public release.
// 
//  -v1          Backwards compatibility.  The current version of the
//               "GL Nodes" spec (V2.0) introduces a new format for the
//               GL_VERT lump designed to prevent slime trails.  Some
//               engines may not support this at first, so use this
//               option to produce the old (V1.0) format.
//
//These next set of options normally don't need to be used, but they are
//provided to allow complete control or for special circumstances:
//
//  -loadall     glBSP will normally try to copy lumps from the input
//               WAD file to the output file instead of loading them
//               into memory.  This allows you to run glBSP on very
//               large WADS (e.g. 15 MB for DOOM II) on a low memory
//               machine.
//
//               This option causes everything from the input file to be
//               loaded into memory.  This allows you to run glBSP using
//               the same file for both input and output, but I strongly
//               recommend _against_ it: you could lose your original
//               WAD if something goes wrong (and you know Murphy...).
//
//  -nogl        Forces glBSP to not create the GL node info.
//
//  -nonormal    Forces glBSP to not create the normal node information
//               when it detects that it is absent.
// 
//  -forcenormal
//               glBSP will normally detect if the normal node info (i.e. 
//               the non-GL variety) is present, and leave it untouched
//               when it is, otherwise create it.  This option forces
//               glBSP to recreate the normal node info.
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
  "#P00",
  "As far as I know,",
  "none of the various WAD tools that exist (such as DSHRINK, CLEANWAD,",
  "DEUTEX, etc..) are `glBSP aware', they will rearrange or even remove",
  "some of the special GL entries.  If you are preparing your final WAD",
  "file that is either a compilation of just multiple maps, or multiple",
  "maps and other data (music, textures, sprites, graphics, etc..), run",
  "the following process to properly compile everything:",
  "",
  "#P14",
  "1. For your final compile of your maps (e.g. you are completely done",
  "   making your maps), compile them using glbsp and use the `Normal Nodes",
  "   only' build mode to *not* build the GL nodes.",
  "",
  "#P14",
  "2) Run rmb (reject map building) at this point on your WAD files.",
  "",
  "@t        e.g.: rmb map1.wad map2.wad -perfect -nomap",
  "",
  "#P14",
  "3) Merge all your WAD files into your main WAD using NWT or",
  "   Wintex/DEUSF.",
  "",
  "#P14",
  "4) Run cleanwad on your main WAD",
  "",
  "@t        e.g.: cleanwad main.wad main1.wad +rr +al +tw +rp +pp +is +rb +pb",
  "",
  "#P14",
  "5) Recompile the maps in your main WAD with the GL nodes.  Be sure",
  "   to use the `No Clobber REJECT' misc option, which prevents the",
  "   reject entries created by rmb from being overwritten.",
  "",
  "#P00",
  "Your final output, main2.wad, will be a clean and compact WAD, with no",
  "wasted space, and all the GL nodes in the proper order. All of your",
  "data will be in this WAD file as well, as glbsp can compile the maps",
  "in a WAD without destroying non-map data.",
  "",
  "@b *** DO NOT: ***",
  "",
  "#P13",
  "+ Run dshrink on your map WADs at any time!",
  "",
  "#P13",
  "+ Run cleanwad on your map WADs *after* you have compiled your GL",
  "  friendly nodes!  (The GL nodes will be removed).",
  "",
  "#P13",
  "+ Use Wintex/DEUSF to merge map WADs with GL friendly nodes in them!",
  "  (The GL node entries will be rearranged, causing problems).",
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
  "#P00",
  "A node builder works like this: say you are looking at your level in",
  "the automap or in the level editor.  The node builder needs to pick a",
  "line (stretching to infinity) that divides the whole map in two halves",
  "(can be rough).  Now cover up one half with your hand, and repeat the",
  "process on the remaining half.  The nodes builder keeps doing this",
  "until the areas that remain are convex (i.e. none of the walls can",
  "block the view of any other walls when you are inside that area).",
  "",
  "#P00",
  "Those infinite lines are called `partition lines', and when they cross",
  "a linedef, the linedef has to be split.  Each split piece is called a",
  "`seg', and every split causes more segs to be created.  Having fewer",
  "segs is good: less to draw & smaller files, so partition lines are",
  "chosen so that they minimise splits.  The `Factor' value controls how",
  "costly these splits are.  Higher values cause the node builder to try",
  "harder to avoid splits.",
  "",
  "#P00",
  "So, each `partition' line splits an area (or `space') of the level",
  "into *two* smaller spaces.  This is where the term `Binary Space",
  "Partition' (BSP) comes from.",
  "",
  "#P00",
  "Another thing: having a good BSP tree is also important, and helps for",
  "faster rendering & smaller files.  Thus the node builder also tries to",
  "make the BSP tree good (by making it balanced, and keeping it small).",
  "If the Factor value is too high, it will care too much about the",
  "splits, and probably make a bad BSP tree.  How good the BSP tree is",
  "can be gauged by the output line that reads:",
  "",
  "@c@t Heights of left and right subtrees = (12,24)",
  "",
  "#P00",
  "Lower values are better (the BSP tree is smaller), and values that are",
  "closer together are also better (the BSP is more balanced).",
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
  "#P00",
  "As mentioned in the readme file, glBSP is based on BSP 2.3.  Most of",
  "the code has been rewritten, however, and some features of BSP were",
  "changed or abandoned.  Features that are different:",
  "",
  "#P13",
  "+ This GUI version, glbspX, is completely new !",
  "",
  "#P13",
  "+ When the output file is not specified (i.e. no -o option), then",
  "  the default output file will be a GWA file with the same name.",
  "  Under BSP 2.3, the default output file would be \"tmp.wad\".",
  "  (This only applies to the command-line version).",
  "",
  "#P13",
  "+ All code for doing visplane checks has been removed.  It was very",
  "  complex stuff, and for OpenGL DOOM ports this checking is not",
  "  needed.  Thus glBSP does not accept the following options that",
  "  BSP 2.3 supports: -thold, -vp, -vpmark, -vpwarn.",
  "",
  "#P13",
  "+ glBSP should work on big-endian platforms (like the Mac).  The",
  "  code was rewritten with this in mind, though it has not been",
  "  tested yet (a report on this would be appreciated).",
  "",
  "#P13",
  "+ The `just for a grin' special feature where a linedef with tag",
  "  999 would cause an angle adjustment was removed.",
  "",
  "#P13",
  "+ glBSP has Hexen support.",
  "",
  "#P13",
  "+ glBSP packs the blockmap, and can merge identical sidedefs.",
  "",
  "#P13",
  "+ glBSP has a much more modular architecture.",
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

