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

#ifndef __DM2BR_TEXTURE_H__
#define __DM2BR_TEXTURE_H__


void Texture_Load(const char *filename);

const char * Texture_Convert(const char *old_name, bool is_flat);
const char *  Entity_Convert(int id_number);

#endif /* __DM2BR_TEXTURE_H__ */
