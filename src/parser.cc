#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <string>
#include <cstring> 

#undef DO_INIT
#include "modes.h"
#include "aplplot_menu.h"
#include "parser.hh"

#define cfg_ASSERT_LEVEL_WANTED 0
#define cfg_SHORT_VALUE_LENGTH_WANTED 1

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

extern Token eval_B (Value_P B);

string keyword;
vector<string> args;

int              plot_width	= DEFAULT_PLOT_WIDTH;
int              plot_height	= DEFAULT_PLOT_HEIGHT;
string           xlabel;
string           ylabel;
string           tlabel;
bool             xlog		= false;
bool             ylog		= false;
int              mode		= APL_MODE_XY;
int              angle_units	= APL_ANGLE_RADIANS;
bool             killem		= false;
int              xcol		= -1;
int              draw		= APL_DRAW_LINES;
string           filename;
bool             embed		= false;
bool             menu		= false;
int              target_idx	= DEF_SCREEN;
unsigned char    bgred		= 0;
unsigned char    bggreen 	= 0;
unsigned char    bgblue 	= 0;
double           xorigin	= 0.0;
double           xspan		= -1.0;
int              plot_pipe_fd 	= -1;
int              axis 		= -1;	// fixme add to menu

//Value_P global_B;

void *menu_handle = NULL;
aplplot_menu_t aplplot_menu = NULL;

typedef boost::unordered_map<std::string, int> si_map;
si_map  kwd_map;

static void reset_options (int arg) {
  plot_width	= DEFAULT_PLOT_WIDTH;
  plot_height	= DEFAULT_PLOT_HEIGHT;
  xlabel.clear ();
  ylabel.clear ();
  tlabel.clear ();
  xlog		= false;
  ylog		= false;
  mode		= APL_MODE_XY;
  angle_units	= APL_ANGLE_RADIANS;
  killem	= false;
  xcol		= -1;
  draw		= APL_DRAW_LINES;
  target_idx	= DEF_SCREEN;
  bgred		= bggreen = bgblue = 0;
  xorigin	= 0.0;
  xspan		= -1.0;
  embed		= false;
  menu		= false;
}


static void
verify_filename ()
{
  target_idx = DEF_SCREEN;
  if (!filename.empty () && filename.length () > 0) {
    int i;
    for (i = 0; i < MODES_MAX_ENTRY; i++) {
      if (i != DEF_SCREEN) {
	char *loc = strstr ((char *)filename.c_str (), mode_strings[i].ext);
	if (loc) {
	  target_idx = i;
	  break;
	}
      }
    }
  }
  if (target_idx == DEF_SCREEN) {
    UERR << "Unsupported file extension: " << filename << endl;
    UERR << "Redirecting to the screen.\n";
  }
}
    
value_u
aplplot_get_value (int which)
{
  value_u rc;
  rc.type = VALUE_ERR;
  switch(which) {
  case VALUE_WIDTH:
    rc.type = which;
    rc.val.i = plot_width;
    break;
  case VALUE_HEIGHT:
    rc.type = which;
    rc.val.i = plot_height;
    break;
  case VALUE_X_LABEL:
    rc.type = which;
    rc.val.s = (char *)(xlabel.c_str ());
    break;
  case VALUE_Y_LABEL:
    rc.type = which;
    rc.val.s = (char *)(ylabel.c_str ());
    break;
  case VALUE_T_LABEL:
    rc.type = which;
    rc.val.s = (char *)(tlabel.c_str ());
    break;
  case VALUE_COLOUR_RED:
    rc.type = which;
    rc.val.i = (int)bgred;
    break;
  case VALUE_COLOUR_GREEN:
    rc.type = which;
    rc.val.i = (int)bggreen;
    break;
  case VALUE_COLOUR_BLUE:
    rc.type = which;
    rc.val.i = (int)bgblue;
    break;
  case VALUE_X_COL:
    rc.type = which;
    rc.val.i = xcol;
    break;
  case VALUE_X_ORIGIN:
    rc.type = which;
    rc.val.f = xorigin;
    break;
  case VALUE_X_SPAN:
    rc.type = which;
    rc.val.f = xspan;
    break;
  case VALUE_X_LOG:
    rc.type = which;
    rc.val.b = xlog;
    break;
  case VALUE_Y_LOG:
    rc.type = which;
    rc.val.b = ylog;
    break;
  case VALUE_DRAW:
    rc.type = which;
    rc.val.i = draw;
    break;
  case VALUE_COORDS:
    rc.type = which;
    rc.val.i = mode;
    break;
  case VALUE_ANGLES:
    rc.type = which;
    rc.val.i = angle_units;
    break;
  case VALUE_EMBED:
    rc.type = which;
    rc.val.b = embed;
    break;
  case VALUE_FILE_NAME:
    rc.type = which;
    rc.val.s = (char *)filename.c_str ();
    break;
  }
  return rc;
}
    
void
aplplot_set_value (value_u val)
{
  switch(val.type) {
  case VALUE_WIDTH:
    plot_width =  val.val.i;
    break;
  case VALUE_HEIGHT:
    plot_height =  val.val.i;
    break;
  case VALUE_X_LABEL:
    xlabel =  val.val.s;
    break;
  case VALUE_Y_LABEL:
    ylabel =  val.val.s;
    break;
  case VALUE_T_LABEL:
    tlabel =  val.val.s;
    break;
  case VALUE_X_LOG:
    xlog = val.val.b;
    break;
  case VALUE_Y_LOG:
    ylog = val.val.b;
    break;
  case VALUE_COORDS:
    mode = val.val.i;
    break;
  case VALUE_ANGLES:
    angle_units =  val.val.i;
    break;
  case VALUE_X_COL:
    xcol =  val.val.b;
    break;
  case VALUE_DRAW:
    draw =  val.val.i;
    break;
  case VALUE_FILE_NAME:
    filename = val.val.s;
    verify_filename ();
    break;
  case VALUE_X_ORIGIN:
    xorigin = val.val.f;
    break;
  case VALUE_X_SPAN:
    xspan = val.val.f;
    break;
  case VALUE_EMBED:
    embed = val.val.b;
    if (embed) target_idx = DEF_PNG;
    break;
  case VALUE_COLOUR_RED:
    bgred   = (unsigned char)val.val.i;
    break;
  case VALUE_COLOUR_GREEN:
    bggreen   = (unsigned char)val.val.i;
    break;
  case VALUE_COLOUR_BLUE:
    bgblue   = (unsigned char)val.val.i;
    break;
  case VALUE_GO:
    eval_B (global_B);
  default:		// do nothing
    break;
  }
}

static void set_width (int arg) {
  if (args.size () >= 1) istringstream (args[0]) >> plot_width;
}

static void set_height (int arg) {
  if (args.size () >= 1) istringstream (args[0]) >> plot_height;
}

static void set_dims (int arg) {
  if (args.size () >= 2) {
    istringstream (args[0]) >> plot_width;
    istringstream (args[1]) >> plot_height;
  }
}

static void set_xlabel (int arg) {
  if (args.size () >= 1) xlabel = args[0];
  else xlabel.clear ();
}

static void set_ylabel (int arg) {
  if (args.size () >= 1) ylabel = args[0];
  else ylabel.clear ();
}

static void set_tlabel (int arg) {
  if (args.size () >= 1) tlabel = args[0];
  else tlabel.clear ();
}

static void set_killem (int arg) { killem = true; }

static void set_embed (int arg) {
  embed = (args.size () >= 1 && 0 == args[0].compare ("off")) ? false : true;
  if (embed) target_idx = DEF_PNG;
}

static void set_menu (int arg) {
  if (aplplot_menu) {
    (*aplplot_menu) ((void *)aplplot_set_value, (void *)aplplot_get_value);
  }
  menu = (args.size () >= 1 && 0 == args[0].compare ("off")) ? false : true;
}

static void set_xlog (int arg) {
  xlog = (args.size () >= 1 && 0 == args[0].compare ("off")) ? false : true;
}

static void set_ylog (int arg) {
  ylog = (args.size () >= 1 && 0 == args[0].compare ("off")) ? false : true;
}

static void set_mode (int arg) {
  if (arg == APL_MODE_SET)
    mode = (args.size () >= 1 && 0 == args[0].compare ("polar"))
      ? APL_MODE_POLAR : APL_MODE_XY;
  else 
    mode = arg;
}

static void set_angle (int arg) {
  if (arg== APL_ANGLE_SET) {
    if (args.size () >= 1) {
      switch (*(args[0].c_str ())) {
      case 'd':
      case 'D':
	angle_units = APL_ANGLE_DEGREES;
	break;
      case 'r':
      case 'R':
	angle_units = APL_ANGLE_RADIANS;
	break;
      case 'p':
      case 'P':
	angle_units = APL_ANGLE_PI_RADIANS;
	break;
      default:
	UERR << "Unrecognised angle unit " << args[0] << endl;
	break;
      }
    } else angle_units = APL_ANGLE_RADIANS;
  } else angle_units = arg;
}

static void set_draw (int arg) {
  if (arg== APL_DRAW_SET) {
    if (args.size () >= 1) {
      switch (*(args[0].c_str ())) {
      case 'l':
      case 'L':
	draw = APL_DRAW_LINES;
	break;
      case 'p':
      case 'P':
	draw = APL_DRAW_POINTS;
	break;
      case 'b':
      case 'B':
	draw = APL_DRAW_BOTH;
	break;
      case 's':
      case 'S':
	draw = APL_DRAW_SCATTER;
	break;
      default:
	UERR << "Unrecognised draw option " << args[0] << endl;
	break;
      }
    } else draw = APL_DRAW_LINES;
  } else draw = arg;
}

static void set_xcol (int arg) {
  if (args.size () >= 1) istringstream (args[0]) >> xcol;
  else xcol = -1;
}

static void set_file (int arg) {
  if (args.size () >= 1) {
    filename = args[0];
    verify_filename ();
  }
  else filename.clear ();
  if (filename.empty () || filename.length () == 0)
    target_idx = DEF_SCREEN;
}

static void set_dest (int arg) {
  if (args.size () >= 1) {
    int i;
    for (i = 0; i < MODES_MAX_ENTRY; i++) {
      string src (mode_strings[i].mode);
      if (0 == args[0].compare (src)) {
	target_idx = i;
	break;
      }
    }
    if (i == MODES_MAX_ENTRY) {
      UERR << "option must be one of screen, png, pdf, svg\n";
      UERR << "eps, or ps.\n";
      UERR << "Setting destination to screen\n";
      target_idx = DEF_SCREEN;
    }
  }
  else target_idx = DEF_SCREEN;
}

enum {APL_BG_SET, APL_BG_BLACK, APL_BG_WHITE};

static void set_background (int arg) {
  switch (arg) {
  case APL_BG_WHITE:
    bgred = 255; bggreen = 255; bgblue = 255;
    break;
  case APL_BG_BLACK:
    bgred = 0; bggreen = 0, bgblue = 0;
    break;
  case APL_BG_SET:
     if (args.size () >= 3) {
      int r,g,b;
      istringstream (args[0]) >> r;
      istringstream (args[1]) >> g;
      istringstream (args[2]) >> b;
      bgred   = (unsigned char)r;
      bggreen = (unsigned char)g;
      bgblue  = (unsigned char)b;
    }
    break;
  }
}

static void set_domain (int arg) {
  if (args.size () <= 0) {
    xorigin	= 0.0;
    xspan	= -1.0;
  }
  else {
    char *rem = NULL;
    xorigin	= strtod (args[0].c_str (), &rem);
    if (rem && (*rem == 'p' || *rem == 'P')) xorigin *= M_PI;
    if (args.size () > 1) {
      rem = NULL;
      xspan	= strtod (args[1].c_str (), &rem);
      if (rem && (*rem == 'p' || *rem == 'P')) xspan *= M_PI;
    }
    else xspan	= -1.0;
  }
}

kwd_s kwds[] = {
  {"width",		set_width,	0},
  {"height",		set_height,	0},
  {"dims",		set_dims,	0},
  {"xlabel",		set_xlabel,	0},
  {"ylabel",		set_ylabel,	0},
  {"tlabel",		set_tlabel,	0},
  {"kill",		set_killem,	0},
  {"xlog",		set_xlog,	0},
  {"logx",		set_xlog,	0},
  {"ylog",		set_ylog,	0},
  {"logy",		set_ylog,	0},
  {"mode",		set_mode,	APL_MODE_SET},
  {"polar",		set_mode,	APL_MODE_POLAR},
  {"xy",		set_mode,	APL_MODE_XY},
  {"angle",		set_angle,	APL_ANGLE_SET},
  {"angle_unit",	set_angle,	APL_ANGLE_SET},
  {"degrees",		set_angle,	APL_ANGLE_DEGREES},
  {"radians",		set_angle,	APL_ANGLE_RADIANS},
  {"piradians",		set_angle,	APL_ANGLE_PI_RADIANS},
  {"xcol",		set_xcol,	0},
  {"draw",		set_draw,	APL_DRAW_SET},
  {"lines",		set_draw,	APL_DRAW_LINES},
  {"points",		set_draw,	APL_DRAW_POINTS},
  {"both",		set_draw,	APL_DRAW_BOTH},
  {"file",		set_file,	0},
  {"embed",		set_embed,	0},
  {"menu",		set_menu,	0},
  {"dest",		set_dest,	0},
  {"reset",		reset_options,	0},
  {"xdomain",		set_domain,	0},
  {"bgwhite",		set_background,	APL_BG_WHITE},
  {"bgblack",		set_background,	APL_BG_BLACK},
  {"background",	set_background,	APL_BG_SET},
  {"bg",		set_background,	APL_BG_SET}
};

int nr_kwds = sizeof (kwds) / sizeof (kwd_s);
