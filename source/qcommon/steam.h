/*
Copyright (C) 2014 Victor Luchits

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _STEAM_LIB_H_
#define _STEAM_LIB_H_
#include "../steamshim/src/steamshim_types.h"

#include <stdint.h>
#define STEAMID_CHARS 18
void Steam_Init( void );
void Steam_Shutdown( void );

extern cvar_t *steam_debug;

#endif // _STEAM_LIB_H_
