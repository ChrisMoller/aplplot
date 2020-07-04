#ifndef APLPLOT_MENU_H
#define APLPLOT_MENU_H

typedef void (*aplplot_menu_t) (void *fcn);  

typedef enum {
	      DEST_SCREEN,
	      DEST_PNG,
	      DEST_PDF,
	      DEST_PS,
	      DEST_EPS,
	      DEST_SVG
} dest_e;

typedef enum {
	      COORDS_DEGREES,
	      COORDS_RADIANS,
	      COORDS_PI_RRADIANS
} coords_e;

enum {
      VALUE_WIDTH,		// int
      VALUE_HEIGHT,		// int
      VALUE_X_LABEL,		// string
      VALUE_Y_LABEL,		// string
      VALUE_T_LABEL,		// string
      VALUE_X_COL,		// int
      VALUE_FILE_NAME,		// string
      VALUE_EMBED,		// boolean
      VALUE_DEST,		// dest_e
      VALUE_X_LOG,		// boolean
      VALUE_Y_LOG,		// boolean
      VALUE_LINES,		// boolean
      VALUE_POINTS,		// boolean
      VALUE_POLAR,		// boolean
      VALUE_ANGLES,		// coords_e
      VALUE_X_MIN,		// double
      VALUE_X_MAX,		// double
      VALUE_COLOUR		// string
};

typedef struct {
  int type;
  union {
    char    *s;
    double   f;
    int      i;
    int      b;
    dest_e   d;
    coords_e c;
  } val;
} value_u;

typedef void (*aplplot_set_value_t) (value_u val);  

#endif // APLPLOT_MENU_H
