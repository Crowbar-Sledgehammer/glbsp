//------------------------------------------------------------------------
// FILES : Unix/FLTK File boxes
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

#include <unistd.h>


#define ALERT_TXT  "glBSP Alert"
#define MISSING_COMMS  "(Not Specified)"

#define MY_GWA_COLOR  \
    fl_color_cube((FL_NUM_RED-1)*3/4, 0, 0)


static void file_box_inout_CB(Fl_Widget *w, void *data)
{
  guix_win->files->WriteInfo();

  // update output box in GWA mode
  guix_win->files->InFileChanged();
}


static void file_in_browse_CB(Fl_Widget *w, void *data)
{
  const char *name = fl_file_chooser("Select an input file",
      "*.wad", guix_win->files->in_file->value());

  // cancelled ?
  if (! name)
    return;

  guix_win->files->in_file->value(name);
  guix_win->files->in_file->redraw();
  guix_win->files->WriteInfo();

  guix_win->files->InFileChanged();
}

#if 0  // not needed (but might be)
static void file_out_browse_CB(Fl_Widget *w, void *data)
{
  const char *name = fl_file_chooser("Select an output file",
      NULL, guix_win->files->out_file->value());

  if (! name)
    return;

  guix_win->files->out_file->value(name);
  guix_win->files->out_file->redraw();
  guix_win->files->WriteInfo();
}
#endif

static void file_out_guess_CB(Fl_Widget *w, void *data)
{
  guix_win->files->out_file->value(
      HelperGuessOutput(guix_win->files->in_file->value()));
   
  guix_win->files->out_file->redraw();
  guix_win->files->WriteInfo();
}


//
// FileBox Constructor
//
Guix_FileBox::Guix_FileBox(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h, "Files")
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
  
  box(FL_THIN_UP_BOX);

  labelfont(FL_HELVETICA | FL_BOLD);
  labeltype(FL_NORMAL_LABEL);
  align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);

  // create the file group -- serves as the resizable
  int len = w - 62 - 100;

  file_group = new Fl_Group(x+62, y, len, h);
  file_group->end();
  add(file_group);

  resizable(file_group);

  // create input and output file widgets
    
  in_label = new Fl_Box(x, y+18+30*0, 62, 26, "Input");
  in_label->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
  add(in_label);
   
  out_label = new Fl_Box(x, y+18+30*1, 62, 26, "Output");
  out_label->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
  add(out_label);
   
  in_file = new Fl_Input(x + 62, y+18+30*0, len, 26);
  in_file->callback((Fl_Callback *) file_box_inout_CB);
  in_file->when(FL_WHEN_CHANGED);
  file_group->add(in_file);

  out_file = new Fl_Input(x + 62, y+18+30*1, len, 26);
  out_file->callback((Fl_Callback *) file_box_inout_CB);
  out_file->when(FL_WHEN_CHANGED);
  file_group->add(out_file);

  // this widget is normally hidden.  It occupies the same space as
  // the out_file widget.  When GWA mode is selected, the normal
  // output box is hidden and this one is shown instead.

  out_gwa_file = new Fl_Output(x + 62, y+18+30*1, len, 26);
  out_gwa_file->textcolor(MY_GWA_COLOR);
  out_gwa_file->selection_color(FL_CYAN);
  out_gwa_file->hide();
  file_group->add(out_gwa_file);

  in_browse = new Fl_Button(x+70+len, y+18+30*0, 80, 26, "Browse");
  in_browse->align(FL_ALIGN_INSIDE);
  in_browse->callback((Fl_Callback *) file_in_browse_CB, in_file);
  add(in_browse);

  out_browse = new Fl_Button(x+70+len, y+18+30*1, 70, 26, "Guess");
  out_browse->align(FL_ALIGN_INSIDE);
  out_browse->callback((Fl_Callback *) file_out_guess_CB, out_file);
  add(out_browse);

  ReadInfo();
}

//
// FileBox Destructor
//
Guix_FileBox::~Guix_FileBox()
{
  WriteInfo();

  GlbspFree(gwa_filename);
}


void Guix_FileBox::ReadInfo()
{
  in_file->value(guix_info.input_file);
  in_file->redraw();

  out_file->value(guix_info.output_file);
  out_file->redraw();

  InFileChanged();
  GWA_Changed();
}


void Guix_FileBox::WriteInfo()
{
  GlbspFree(guix_info.input_file);
  guix_info.input_file = NULL;

  GlbspFree(guix_info.output_file);
  guix_info.output_file = NULL;

  // treat "" the same as a NULL string ptr
  if (in_file->value() && strlen(in_file->value()) > 0)
    guix_info.input_file = GlbspStrDup(in_file->value());

  if (out_file->value() && strlen(out_file->value()) > 0)
    guix_info.output_file = GlbspStrDup(out_file->value());
}


void Guix_FileBox::GWA_Changed()
{
  if (guix_info.gwa_mode)
  {
    out_file->hide();
    out_gwa_file->show();
    out_browse->deactivate();
  }
  else
  {
    out_gwa_file->hide();
    out_file->show();
    out_browse->activate();
  }
}


void Guix_FileBox::InFileChanged(void)
{
  GlbspFree(gwa_filename);

  gwa_filename = GlbspStrDup(
      HelperReplaceExt(guix_info.input_file, "gwa"));
   
  out_gwa_file->value(gwa_filename);
  out_gwa_file->redraw();
}


//------------------------------------------------------------------------


//
// FactorBox Constructor
//
Guix_FactorBox::Guix_FactorBox(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h)
{
  // cancel the automatic `begin' in Fl_Group constructor
  end();
 
  box(FL_THIN_UP_BOX);
  resizable(0);  // no resizing the kiddies, please
  
  // create factor input box

  factor = new Fl_Counter(x+60, y+7, 100, 26, "Factor");
  factor->align(FL_ALIGN_LEFT);
  factor->type(FL_SIMPLE_COUNTER);
  factor->range(1, 99);
  factor->step(1, 1);
  add(factor);

  ReadInfo();
}

//
// FactorBox Destructor
//
Guix_FactorBox::~Guix_FactorBox()
{
  WriteInfo();
}


void Guix_FactorBox::ReadInfo()
{
  factor->value(guix_info.factor);
}


void Guix_FactorBox::WriteInfo()
{
  guix_info.factor = (int) factor->value();
}


//------------------------------------------------------------------------

static boolean_g BuildValidateOptions(void)
{
  // This routine checks for all manners of nasty input/output file
  // problems.  Belt up, it's a bumpy ride !!

  int choice;
  char buffer[1024];

  // a) Empty or invalid filenames

  if (!guix_info.input_file || guix_info.input_file[0] == 0)
  {
    DialogShowAndGetChoice(ALERT_TXT, skull_image, 
        "Please choose an Input filename.");
    return FALSE;
  }
  
  if (!guix_info.output_file || guix_info.output_file[0] == 0)
  {
    DialogShowAndGetChoice(ALERT_TXT, skull_image, 
        "Please choose an Output filename.");
    return FALSE;
  }
 
  
  // b) Missing extensions
 
  if (! HelperHasExt(guix_info.input_file))
  {
    sprintf(buffer,
        "The input file you selected has no extension.\n"
        "\n"
        "Do you want to add \".WAD\" and continue ?");

    choice = DialogShowAndGetChoice(ALERT_TXT, skull_image,
        buffer, "OK", "Cancel");

    if (choice != 0)
      return FALSE;

    char *new_input = HelperReplaceExt(guix_info.input_file, "wad");

    GlbspFree(guix_info.input_file);
    guix_info.input_file = GlbspStrDup(new_input);
  }

  if (! HelperHasExt(guix_info.output_file))
  {
    sprintf(buffer,
        "The output file you selected has no extension.\n"
        "\n"
        "Do you want to add \".%s\" and continue ?",
        guix_info.gwa_mode ? "GWA" : "WAD");

    choice = DialogShowAndGetChoice(ALERT_TXT, skull_image,
        buffer, "OK", "Cancel");

    if (choice != 0)
      return FALSE;

    char *new_output = HelperReplaceExt(guix_info.output_file, 
        guix_info.gwa_mode ? "gwa" : "wad");

    GlbspFree(guix_info.output_file);
    guix_info.output_file = GlbspStrDup(new_output);
  }

 
  // c) No such input file
  
  if (! HelperFileExists(guix_info.input_file))
  {
    sprintf(buffer,
        "Could not open the input file:\n"
        "    %s\n\n"
        "Please check the filename and try again.",
        guix_info.input_file);

    DialogShowAndGetChoice(ALERT_TXT, skull_image, buffer);
    return FALSE;
  }

  
  // d) Use of the ".gwa" extension

  if (HelperCheckExt(guix_info.input_file, "gwa"))
  {
    DialogShowAndGetChoice(ALERT_TXT, skull_image, 
        "The input file you selected has the GWA extension.\n"
        "GWA files do not contain any level data, so there\n"
        "wouldn't be anything to build nodes for.\n"
        "\n"
        "Please choose another Input filename.");
    return FALSE;
  }

  if (HelperCheckExt(guix_info.output_file, "gwa") &&
      ! guix_info.gwa_mode)
  {
    choice = DialogShowAndGetChoice(ALERT_TXT, skull_image,
        "The output file you selected has the GWA extension,\n"
        "but the GWA Mode option is not enabled.\n"
        "\n"
        "Do you want to enable GWA Mode and continue ?\n".
        "OK", "Cancel");

    if (choice != 0)
      return FALSE;

    guix_info.gwa_mode = TRUE;
    guix_info.no_gl = FALSE;
    guix_info.no_normal = FALSE;
    guix_info.force_normal = FALSE;

    guix_win->build_mode->ReadInfo();
    guix_win->files->GWA_Changed();
  }


  // e) Input == Output
  // f) Output file already exists

  guix_info.load_all = FALSE;

  if (HelperCaseCmp(guix_info.input_file, guix_info.output_file) == 0)
  {
    if (guix_prefs.same_file_warn)
    {
      sprintf(buffer,
          "Warning: the input and output files are the same.\n"
          "\n"
          "This will use a lot more memory than normal, since the\n"
          "whole input wad must be loaded in.  On a low memory\n"
          "machine, the node building may fail (especially if the\n"
          "WAD is very large, e.g. DOOM2.WAD).  It is also slightly\n"
          "risky: if something goes wrong during saving, your WAD\n"
          "file will be toast.\n"
          "\n"
          "Do you want to continue ?\n"
          "Choose Always to disable this warning in the future.");

      choice = DialogShowAndGetChoice(ALERT_TXT, skull_image,
          buffer, "OK", "Always", "Cancel");
       
      if (choice < 0 || choice == 2)
        return FALSE;

      if (choice == 1)
        guix_prefs.same_file_warn = FALSE;
    }
    
    guix_info.load_all = TRUE; 
  }
  else if (HelperFileExists(guix_info.output_file) &&
      guix_prefs.overwrite_warn)
  {
    sprintf(buffer,
        "Warning: the chosen output file already exists:\n\n"
        "    %s\n\n"
        "Do you want to overwrite it ?\n"
        "Choose Always to disable this warning in the future.",
        guix_info.output_file);

    choice = DialogShowAndGetChoice(ALERT_TXT, skull_image,
        buffer, "Overwrite", "Always", "Cancel");

    if (choice < 0 || choice == 2)
      return FALSE;

    if (choice == 1)
      guix_prefs.overwrite_warn = FALSE;
  }

  return TRUE;  
}

static boolean_g BuildCheckInfo(void)
{
  char buffer[1024];

  for (;;)
  {
    glbsp_ret_e ret = GlbspCheckInfo(&guix_info, &guix_comms); 

    if (ret == GLBSP_E_OK)
      return TRUE;

    if (ret != GLBSP_E_BadInfoFixed)
    {
      sprintf(buffer, 
          "The following problem was detected with the current\n"
          "options:\n\n"
          "    %s\n\n"
          "Please fix the problem and try again.",
          guix_comms.message ? guix_comms.message : MISSING_COMMS);

      // user doesn't have any real choice here
      DialogShowAndGetChoice(ALERT_TXT, skull_image, buffer);

      guix_win->ReadAllInfo();
      break;
    }

    sprintf(buffer,
        "The following problem was detected with the current\n"
        "options:\n\n"
        "    %s\n\n"
        "However, the option causing the problem has been changed.\n"
        "Do you want to continue ?",
        guix_comms.message ? guix_comms.message : MISSING_COMMS);

    int choice = DialogShowAndGetChoice(ALERT_TXT, skull_image, 
        buffer, "OK", "Cancel");

    if (choice != 0)
      break;
  }

  return FALSE;
}

static void BuildDoBuild(void)
{
  glbsp_ret_e ret = GlbspBuildNodes(&guix_info, &guix_funcs, &guix_comms);

  if (ret == GLBSP_E_OK)
    return;

  if (ret == GLBSP_E_Cancelled)
  {
    guix_win->text_box->AddMsg("\n*** Cancelled ***\n", FL_BLUE, TRUE);
    return;
  }

  // something went wrong :(
 
  const char *err_short;

  switch (ret)
  {
    case GLBSP_E_ReadError:
      guix_win->text_box->AddMsg("\n*** Read Error ***\n", FL_BLUE, TRUE);
      err_short = "Read";
      break;

    case GLBSP_E_WriteError:
      guix_win->text_box->AddMsg("\n*** Write Error ***\n", FL_BLUE, TRUE);
      err_short = "Write";
      break;

    // these two shouldn't happen
    case GLBSP_E_BadArgs:
    case GLBSP_E_BadInfoFixed:
      guix_win->text_box->AddMsg("\n*** Option Error ***\n", FL_BLUE, TRUE);
      err_short = "Option";
      break;

    case GLBSP_E_Unknown:
    default:
      guix_win->text_box->AddMsg("\n*** Unknown Error ***\n", FL_BLUE, TRUE);
      err_short = "Unknown";
      break;
  }

  char buffer[1024];

  sprintf(buffer, 
      "The following %s Error occurred when trying to build\n"
      "the nodes:\n"
      "\n"
      "   %s\n"
      "\n"
      "For Read errors, check that the input file is a valid\n"
      "WAD file, and is not corrupt.  For Write errors, check\n"
      "that your hard disk (or floppy, etc) has not run out\n"
      "of storage space."
      comms->message ? comms->message : MISSING_COMMS);

  DialogShowAndGetChoice(ALERT_TXT, skull_image, buffer);
}

static void build_button_CB(Fl_Widget *w, void *data)
{
  // make sure info is up-to-date
  guix_win->WriteAllInfo();

  // save cookies, in case the build crashes or calls the fatal-error
  // function.
  CookieWriteAll();

  // sleight of hand for GWA mode: we remember the old output name in
  // the nodebuildinfo and replace it with the gwa name.  The memory
  // stuff is messy -- we can't be 100% sure that `output_file' field
  // won't be freed and assigned a new value.
 
  const char *old_output = guix_info.output_file;
  boolean_g gwa_hack = FALSE;

  if (guix_info.gwa_mode)
  {
    guix_info.output_file = GlbspStrDup(guix_win->files->gwa_filename);
    gwa_hack = TRUE;
  }
 
//!!!
fprintf(stderr, "BUILD\n  INPUT = [%s]\n  OUTPUT = [%s]\n\n",
    guix_info.input_file, guix_info.output_file);

  if (BuildValidateOptions())
  {
    if (BuildCheckInfo())
    {
      BuildDoBuild();

      guix_win->text_box->AddHorizBar();
    }
  }

  if (gwa_hack)
  {
    GlbspFree(guix_info.output_file);
    guix_info.output_file = old_output;
  }

  // if the build info changed, make sure widgets are in sync.
  // This is not the ideal place, it would be better to call this as
  // soon as any change could've happened -- but the GWA hack
  // interferes with that approach.
  // 
  guix_win->ReadAllInfo();

  GlbspFree(guix_comms.message);
  guix_comms.message = NULL;
}

//
// BuildButton Constructor
//
Guix_BuildButton::Guix_BuildButton(int x, int y, int w, int h) :
    Fl_Group(x, y, w, h)
{
  end();  // turn off auto-add-widgets
  
  resizable(0);  // no resizing the kiddies, please

  // create button sitting in a space of its own

  button = new Fl_Button(x+60, y+5, 120, 30, "Build Nodes");
  button->box(FL_ROUND_UP_BOX);
  button->align(FL_ALIGN_INSIDE);
  button->callback((Fl_Callback *) build_button_CB);
  add(button);
}

//
// BuildButton Destructor
//
Guix_BuildButton::~Guix_BuildButton()
{
  // nothing to do
}

