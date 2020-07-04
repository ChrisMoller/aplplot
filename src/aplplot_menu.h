#ifndef APLPLOT_MENU_H
#define APLPLOT_MENU_H

//#include "modes.h"

#define DEFAULT_PLOT_WIDTH	512
#define DEFAULT_PLOT_HEIGHT	480

typedef void (*aplplot_menu_t) (void *fcn);  

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
      VALUE_GO
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

#endif // APLPLOT_MENU_H