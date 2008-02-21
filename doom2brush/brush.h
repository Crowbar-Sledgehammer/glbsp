//------------------------------------------------------------------------
//  BRUSH Creation
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

#ifndef __DM2BR_BRUSH_H__
#define __DM2BR_BRUSH_H__


void Brush_WriteField(const char *field, const char *val_str, ...);

void Brush_ConvertSectors(void);
void Brush_ConvertExtraFloors(void);
void Brush_ConvertLiquids(void);
void Brush_ConvertWalls(void);
void Brush_ConvertThings(void);


#endif /* __DM2BR_BRUSH_H__ */
