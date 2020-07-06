/*
    This file is part of GNU APL, a free implementation of the
    ISO/IEC Standard 13751, "Programming Language APL, Extended"

    Copyright (C) 2020 Chris Moller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef APLPLOT_MENU_H
#define APLPLOT_MENU_H

//#include "modes.h"

#define DEFAULT_PLOT_WIDTH	512
#define DEFAULT_PLOT_HEIGHT	480

typedef void (*aplplot_menu_t) (void *set_fcn, void *get_fcn);  

enum {
      VALUE_WIDTH,		// int
      VALUE_HEIGHT,		// int
      VALUE_X_LABEL,		// string
      VALUE_Y_LABEL,		// string
      VALUE_T_LABEL,		// string
      VALUE_X_COL,		// int
      VALUE_FILE_NAME,		// string
      VALUE_EMBED,		// boolean
      VALUE_X_LOG,		// boolean
      VALUE_Y_LOG,		// boolean
      VALUE_DRAW,		// int
      VALUE_COORDS,		// int
      VALUE_ANGLES,		// int
      VALUE_X_ORIGIN,		// double
      VALUE_X_SPAN,		// double
      VALUE_COLOUR_RED,		// int
      VALUE_COLOUR_GREEN,	// int
      VALUE_COLOUR_BLUE,	// int
      VALUE_GO,
      VALUE_ERR
};

enum {APL_MODE_SET, APL_MODE_XY, APL_MODE_POLAR };

enum {APL_ANGLE_SET, APL_ANGLE_DEGREES,
      APL_ANGLE_RADIANS, APL_ANGLE_PI_RADIANS };

#define APL_DRAW_SET		-1
#define APL_DRAW_NOTHING	0
#define APL_DRAW_LINES		(1 << 0)
#define APL_DRAW_POINTS		(1 << 1)
#define APL_DRAW_BOTH		(APL_DRAW_LINES | APL_DRAW_POINTS)

typedef struct {
  int type;
  union {
    char    *s;
    double   f;
    int      i;
    int      b;
    int      d;
  } val;
} value_u;

typedef void (*aplplot_set_value_t) (value_u val);  
typedef value_u (*aplplot_get_value_t) (int whichl);  

#endif // APLPLOT_MENU_H
