//------------------------------------------------------------------------
// DIALOG : Unix/FLTK Pop-up dialogs
//------------------------------------------------------------------------
//
//  GL-Friendly Node Builder (C) 2000-2002 Andrew Apted
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


Fl_Image  *about_image;
Fl_Pixmap *pldie_image;
Fl_Pixmap *skull_image;

static unsigned char *about_image_rgb;


static Fl_Window *cur_diag;
static int cur_diag_result;
static boolean_g cur_diag_done;
static const char *cur_diag_guess_name;


static void UncompressAboutImage(unsigned char *& rgb)
{
  int rgb_size = ABOUT_IMG_W * ABOUT_IMG_H * 3;

  rgb = new unsigned char [rgb_size];
  assert(rgb);

  const unsigned char *src = about_image_data;
  unsigned char *dest = rgb;

  while (dest < (rgb + rgb_size))
  {
    int val = *src++;
    int num = 1;

    if (val >= 0xF8)
    {
      static int num_set[8] = { 3,4,5,6, 8,10,12,16 };

      num = num_set[val - 0xF8];
      val = *src++;

      assert(val < 0xF8);
    }

    for (; num > 0; num--)
    {
      *dest++ = about_image_pal[val*3 + 2];  // red
      *dest++ = about_image_pal[val*3 + 1];  // green
      *dest++ = about_image_pal[val*3 + 0];  // blue
    }
  }
}


//
// DialogLoadImages
//
// Should be called after WindowStartup.
//
void DialogLoadImages(void)
{
  pldie_image = new Fl_Pixmap(pldie_image_data);
  skull_image = new Fl_Pixmap(skull_image_data);

  // create about image

  UncompressAboutImage(about_image_rgb);
  
  about_image = new Fl_RGB_Image(about_image_rgb, 
      ABOUT_IMG_W, ABOUT_IMG_H, 3, (ABOUT_IMG_W * 3));
}

//
// DialogFreeImages
//
void DialogFreeImages(void)
{
  if (about_image_rgb)
    delete[] about_image_rgb;

  delete about_image;
  delete pldie_image;
  delete skull_image;
}


//------------------------------------------------------------------------

static void dialog_closed_CB(Fl_Widget *w, void *data)
{
  cur_diag_result = -1;
  cur_diag_done = TRUE;
}

static void dialog_right_button_CB(Fl_Widget *w, void *data)
{
  cur_diag_result = 0;
  cur_diag_done = TRUE;
}

static void dialog_middle_button_CB(Fl_Widget *w, void *data)
{
  cur_diag_result = 1;
  cur_diag_done = TRUE;
}

static void dialog_left_button_CB(Fl_Widget *w, void *data)
{
  cur_diag_result = 2;
  cur_diag_done = TRUE;
}

static void dialog_file_browse_CB(Fl_Widget *w, void *data)
{
  Fl_Input *inp_box = (Fl_Input *) data;
  const char *new_name; 
  
  new_name = fl_file_chooser("Select the log file", "*.log",
      inp_box->value());

  // cancelled ?
  if (! new_name)
    return;

  inp_box->value(new_name);
}

static void dialog_file_guess_CB(Fl_Widget *w, void *data)
{
  Fl_Input *inp_box = (Fl_Input *) data;

  if (cur_diag_guess_name)
  {
    inp_box->value(cur_diag_guess_name);
  }
  
}


//------------------------------------------------------------------------

static void DialogRun()
{
  cur_diag->set_modal();
  cur_diag->show();

  // read initial pos (same logic as in Guix_MainWin)
  WindowSmallDelay();
  int init_x = cur_diag->x(); 
  int init_y = cur_diag->y();

  // run the GUI and let user make their choice
  while (! cur_diag_done)
  {
    Fl::wait();
  }

  // check if the user moved/resized the window
  if (cur_diag->x() != init_x || cur_diag->y() != init_y)
  {
    guix_prefs.dialog_x = cur_diag->x();
    guix_prefs.dialog_y = cur_diag->y();
  }
}


//
// DialogShowAndGetChoice
//
// The `pic' parameter is the picture to show on the left, or NULL for
// none.  The message can contain newlines.  The right/middle/left
// parameters allow up to three buttons.
//
// Returns the button number pressed (0 for right, 1 for middle, 2 for
// left) or -1 if escape was pressed or window manually closed.
// 
int DialogShowAndGetChoice(const char *title, Fl_Pixmap *pic, 
    const char *message, const char *right = "OK", 
    const char *middle = NULL, const char *left = NULL)
{
  cur_diag_result = -1;
  cur_diag_done = FALSE;

  int but_width = left ? (120*3) : middle ? (120*2) : (120*1);

  // determine required size
  int width = 120 * 3;
  int height;

  // set current font for fl_measure()
  fl_font(FL_HELVETICA, FL_NORMAL_SIZE);

  fl_measure(message, width, height);

  if (width < but_width)
    width = but_width;

  if (height < 16)
    height = 16;
 
  width  += 60 + 20 + 16;  // 16 extra, just in case
  height += 10 + 40 + 16;  // 
 
  // create window
  cur_diag = new Fl_Window(0, 0, width, height, title);
  cur_diag->end();
  cur_diag->size_range(width, height, width, height);
  cur_diag->callback((Fl_Callback *) dialog_closed_CB);
  
  cur_diag->position(guix_prefs.dialog_x, guix_prefs.dialog_y);

  // set the resizable
  Fl_Box *box = new Fl_Box(60, 0, width - 3*120, height);
  cur_diag->add(box);
  cur_diag->resizable(box); 

  // create the image, if any
  if (pic)
  {
    box = new Fl_Box(5, 10, 50, 50);
    pic->label(box);
    cur_diag->add(box);
  }
 
  // create the message area
  box = new Fl_Box(60, 10, width-60 - 20, height-10 - 40, message);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
  cur_diag->add(box);
  
  // create buttons
  Fl_Button *button;
  
  if (right)
  {
    button = new Fl_Return_Button(width - 120*1, height-40, 104, 30, right);
    button->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    button->callback((Fl_Callback *) dialog_right_button_CB);
    cur_diag->add(button);
  }

  if (middle)
  {
    button = new Fl_Button(width - 120*2, height-40, 104, 30, middle);
    button->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    button->callback((Fl_Callback *) dialog_middle_button_CB);
    cur_diag->add(button);
  }

  if (left)
  {
    button = new Fl_Button(width - 120*3, height-40, 104, 30, left);
    button->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    button->callback((Fl_Callback *) dialog_left_button_CB);
    cur_diag->add(button);
  }

  // show time !
  DialogRun();
 
  // delete window (automatically deletes child widgets)
  delete cur_diag;
  cur_diag = NULL;

  return cur_diag_result;
}


//
// DialogQueryFilename
// 
// Shows the current filename (name_ptr) in an input box, and provides
// a browse button to choose a new filename, and an optional button to
// guess the new filename (if `guess_name' is NULL, then the button is
// disabled).
//
// This routine does NOT ensure that the filename is valid (or any
// other requirement, e.g. has a certain extension).
// 
// Returns 0 if "OK" was pressed, 1 if "Cancel" was pressed, or -1 if
// escape was pressed or the window was manually closed.
// 
int DialogQueryFilename(const char *title, const char *message,
        const char ** name_ptr, const char *guess_name)
{
  cur_diag_result = -1;
  cur_diag_done = FALSE;
  cur_diag_guess_name = guess_name;

  // determine required size
  int width = 400;
  int height;

  // set current font for fl_measure()
  fl_font(FL_HELVETICA, FL_NORMAL_SIZE);

  fl_measure(message, width, height);

  if (width < 400)
    width = 400;

  if (height < 16)
    height = 16;
 
  width  += 60 + 20 + 16;  // 16 extra, just in case
  height += 60 + 40 + 16;  // 

  // create window
  cur_diag = new Fl_Window(0, 0, width, height, title);
  cur_diag->end();
  cur_diag->size_range(width, height, width, height);
  cur_diag->callback((Fl_Callback *) dialog_closed_CB);
  
  cur_diag->position(guix_prefs.dialog_x, guix_prefs.dialog_y);

  // set the resizable
  Fl_Box *box = new Fl_Box(0, height-1, width, 1);
  cur_diag->add(box);
  cur_diag->resizable(box); 

  // create the message area
  box = new Fl_Box(14, 10, width-20 - 20, height-10 - 100, message);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
  cur_diag->add(box);
  
  // create buttons
  
  Fl_Button *b_ok;
  Fl_Button *b_cancel;
  
  b_ok = new Fl_Return_Button(width - 120*1, height-40, 104, 30, "OK");
  b_ok->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  b_ok->callback((Fl_Callback *) dialog_right_button_CB);
  cur_diag->add(b_ok);

  b_cancel = new Fl_Button(width - 120*2, height-40, 104, 30, "Cancel");
  b_cancel->align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  b_cancel->callback((Fl_Callback *) dialog_middle_button_CB);
  cur_diag->add(b_cancel);

  // create input box
  Fl_Input *inp_box;
  
  inp_box = new Fl_Input(20, height-90, width - 230, 26);
  inp_box->value(*name_ptr);
  cur_diag->add(inp_box);
 
  // create the browse and guess button
  Fl_Button *b_browse;
  Fl_Button *b_guess;

  b_guess = new Fl_Button(width - 120, inp_box->y(), 70, 26, "Guess");
  b_guess->align(FL_ALIGN_INSIDE);
  b_guess->callback((Fl_Callback *) dialog_file_guess_CB, inp_box);
  cur_diag->add(b_guess);

  b_browse = new Fl_Button(width - 205, inp_box->y(), 80, 
      b_guess->h(), "Browse");
  b_browse->align(FL_ALIGN_INSIDE);
  b_browse->callback((Fl_Callback *) dialog_file_browse_CB, inp_box);
  cur_diag->add(b_browse);

  // show time !
  DialogRun();

  if (cur_diag_result == 0)
  {
    GlbspFree(*name_ptr);

    *name_ptr = GlbspStrDup(inp_box->value());
  }
 
  // delete window (automatically deletes child widgets)
  delete cur_diag;
  cur_diag = NULL;

  return cur_diag_result;
}


//
// GUI_FatalError
//
// Terminates the program reporting an error.
//
void GUI_FatalError(const char *str, ...)
{
  char buffer[2048];

  // create message
  va_list args;

  va_start(args, str);
  vsprintf(buffer, str, args);
  va_end(args);

  if (buffer[0] == 0 || buffer[strlen(buffer)-1] != '\n')
    strcat(buffer, "\n");

  strcat(buffer, "\nglBSPX will now shut down.");
    
  DialogShowAndGetChoice("glBSP Fatal Error", pldie_image, buffer);

  // Q/ save cookies ?  
  // A/ no, we save them before each build begins.

  exit(5);
}

