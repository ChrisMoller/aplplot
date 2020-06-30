/*
    This file is part of GNU APL, a free implementation of the
    ISO/IEC Standard 13751, "Programming Language APL, Extended"

    Copyright (C) 2008-2013  Dr. Jürgen Sauermann

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

#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_map.hpp>
#include<iostream>
#include<string>

#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <signal.h>
#include <values.h>
#include <fcntl.h>
#include <plplot/plplot.h>

using namespace std;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_URL
#undef PACKAGE_VERSION
#undef VERSION

#include "aplplot.hh"

#define DEF_XCAIRO	"xcairo"
#define DEF_PNGCAIRO	"pngcairo"
#define DEF_PDFCAIRO	"pdfcairo"
#define DEF_PSCAIRO	"pscairo"
#define DEF_EPSCAIRO	"epscairo"
#define DEF_SVGCAIRO	"svgcairo"

#define DEF_SCREEN	"screen"
#define DEF_PNG		"png"
#define DEF_PDF		"pdf"
#define DEF_PS		"ps"
#define DEF_EPS		"eps"
#define DEF_SVG		"svg"

#define DEFAULT_PLOT_WIDTH	512
#define DEFAULT_PLOT_HEIGHT	480

enum {APL_MODE_SET, APL_MODE_XY, APL_MODE_POLAR };
enum {APL_ANGLE_SET, APL_ANGLE_DEGREES,
      APL_ANGLE_RADIANS, APL_ANGLE_PI_RADIANS };

#define APL_DRAW_SET		-1
#define APL_DRAW_NOTHING	0
#define APL_DRAW_LINES		(1 << 0)
#define APL_DRAW_POINTS		(2 << 1)
#define APL_DRAW_BOTH		(APL_DRAW_LINES | APL_DRAW_POINTS)

static int 	plot_width	= DEFAULT_PLOT_WIDTH;
static int	plot_height	= DEFAULT_PLOT_HEIGHT;
static string	xlabel;
static string	ylabel;
static string	tlabel;
static bool	xlog		= false;
static bool	ylog		= false;
static int	mode		= APL_MODE_XY;
static int      angle_units	= APL_ANGLE_RADIANS;
static bool	killem		= false;
static int	xcol		= -1;
static int	draw		= APL_DRAW_LINES;
static string 	filename;
static bool     embed		= false;
static string 	target (DEF_XCAIRO);
static unsigned char bgred = 0, bggreen = 0, bgblue = 0;
static double	xorigin		= 0.0;
static double	xspan		= -1.0;
static int      plot_pipe_fd = -1;

string keyword;
vector<string> args;

typedef boost::unordered_map<std::string, int> si_map;
si_map  kwd_map;

typedef void (*option_fcn)(int);
typedef struct { std::string kwdx; option_fcn fcnx; int argx;} kwd_s;
#define opt_kwd(i) kwds[i].kwdx
#define opt_fcn(i) kwds[i].fcnx
#define opt_arg(i) kwds[i].argx

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
  target	= DEF_XCAIRO;
  bgred		= bggreen = bgblue = 0;
  xorigin	= 0.0;
  xspan		= -1.0;
  embed		= false;
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
  if (embed) target = DEF_PNGCAIRO;
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
	cerr << "Unrecognised angle unit " << args[0] << endl;
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
      default:
	cerr << "Unrecognised draw option " << args[0] << endl;
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
  if (args.size () >= 1) filename = args[0];
  else filename.clear ();
  if (filename.empty () || filename.length () == 0) target = DEF_XCAIRO;
}

static void set_dest (int arg) {
  if (args.size () >= 1) {
    if      (0 == args[0].compare (DEF_SCREEN)) target = DEF_XCAIRO;
    else if (0 == args[0].compare (DEF_PNG))    target = DEF_PNGCAIRO;
    else if (0 == args[0].compare (DEF_PDF))    target = DEF_PDFCAIRO;
    else if (0 == args[0].compare (DEF_PS))     target = DEF_PSCAIRO;
    else if (0 == args[0].compare (DEF_EPS))    target = DEF_EPSCAIRO;
    else if (0 == args[0].compare (DEF_SVG))    target = DEF_SVGCAIRO;
    else {
      cerr << "option must be one of screen, png, pdf, svg\n";
      cerr << "eps, or ps.\n";
      cerr << "Setting destination to screen\n";
      target = DEF_XCAIRO;
    }
  }
  else target = DEF_XCAIRO;
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
  {"dest",		set_dest,	0},
  {"reset",		reset_options,	0},
  {"xdomain",		set_domain,	0},
  {"bgwhite",		set_background,	APL_BG_WHITE},
  {"bgblack",		set_background,	APL_BG_BLACK},
  {"background",	set_background,	APL_BG_SET}
};

class LineClass {
public:
  LineClass (PLINT c, PLFLT *x,  PLFLT *y) {
    count = c; xvec = x; yvec = y;
  }
  ~LineClass () { delete [] xvec; delete [] yvec; }
  PLINT count;
  PLFLT *xvec;
  PLFLT *yvec;
};

void *
get_function_mux(const char * function_name)
{
   if (!strcmp(function_name, "get_signature"))   return (void *)&get_signature;
   if (!strcmp(function_name, "eval_B"))          return (void *)&eval_B;
   if (!strcmp(function_name, "eval_AB"))         return (void *)&eval_AB;
   if (!strcmp(function_name, "eval_XB"))         return (void *)&eval_XB;
   if (!strcmp(function_name, "eval_AXB"))        return (void *)&eval_AXB;
   if (!strcmp(function_name, "eval_fill_B"))     return (void *)&eval_fill_B;
   if (!strcmp(function_name, "eval_fill_AB"))    return (void *)&eval_fill_AB;
   if (!strcmp(function_name, "eval_ident_Bx"))   return (void *)&eval_ident_Bx;
   return 0;
}

static void
render_z (PLINT count,
	  APL_Float min_xv,
	  APL_Float max_xv,
	  APL_Float min_yv,
	  APL_Float max_yv,
	  APL_Float min_zv,
	  APL_Float max_zv,
	  PLFLT *xvec,		// fixme switch to class
	  PLFLT *yvec,
	  PLFLT *zvec)
{

    /*
         z
         |
         |
         |
         +----------- x
        /
       /
      /
     y
      
     */
    

  plscolbg (bgred, bggreen, bgblue);
  plinit ();

  pladv (0);
  plvpor( 0.0, 1.0, 0.0, 1.0 );
  plwind( -512.00, 512.0, -200.0, 600.0);

  pllab (xlabel.empty () ? "" : xlabel.c_str (),
	 ylabel.empty () ? "" : ylabel.c_str (),
	 tlabel.empty () ? "" : tlabel.c_str ());
    
  plw3d (512.0,	// basex,
	 512.0,	// basey,
	 512.0,	// height,
	 min_xv,	// xmin,
	 max_xv,	// xmax,
	 min_yv,	// ymin,
	 max_yv,	// ymax,
	 min_zv,	// zmin,
	 max_zv,	// zmax,
	 20.0,	// alt,
	 -30.0);	// az: pos cw from top, neg ccw from top

  plbox3 ("bnstu",	// xopt,
	  "x label",	// xlabel,	
	  0.0,	// xtick,
	  0,		// nxsub,
	  "bnstu",	// xopt,
	  "y label",	// xlabel,	
	  0.0,	// xtick,
	  0,		// nxsub,
	  "bcdmnstuv",	// xopt,
	  "z label",	// xlabel,	
	  0.0,	// xtick,
	  0);		// nxsub

  plline3 (count, xvec, yvec, zvec);
}

static FILE *
open_file (char **tfile_p)
{
  FILE *rf;
  *tfile_p = NULL;
  if (embed) {
    if (plot_pipe_fd != -1) {
#define TEMPORARY_IMAGE_FILE "/tmp/aplplot-image.XXXXXX"
      *tfile_p = strdup (TEMPORARY_IMAGE_FILE);
      if (*tfile_p) {
	int tfd = mkstemp (*tfile_p);
	rf = fdopen (tfd, "w");
      }
    }
  }
  else {
    if (!filename.empty ()) rf = fopen (filename.c_str (), "w");
    else cerr << "missing filename\n";
  }
  return rf;
}

static int
run_plot_z (PLINT count,
	    APL_Float min_xv,
	    APL_Float max_xv,
	    APL_Float min_yv,
	    APL_Float max_yv,
	    APL_Float min_zv,
	    APL_Float max_zv,
	    PLFLT *xvec,
	    PLFLT *yvec,
	    PLFLT *zvec)
{
  plspage (0.0,  0.0, plot_width, plot_height, 0.0, 0.0);
  plsdev (target.c_str ());
  if (0 == target.compare (DEF_XCAIRO))  {
    pid_t pid = fork ();
    if (pid == 0) {		// child
      setsid ();
    
      render_z (count,
		min_xv, max_xv,
		min_yv, max_yv,
		min_zv, max_zv,
		xvec, yvec, zvec);

      plend ();
      wait (NULL);
      exit (0);
    }
    return (int)pid;
  }
  else {
    char *tfile = NULL;
    FILE *po = open_file (&tfile);

    if (po) {
      plsfile (po);
      render_z (count,
		min_xv, max_xv,
		min_yv, max_yv,
		min_zv, max_zv,
		xvec, yvec, zvec);
      plend ();
      if (embed) {
	write (plot_pipe_fd, tfile, strlen (tfile));
	syncfs (plot_pipe_fd);
	usleep (50000);
      }
      if (tfile) free (tfile);
    }
    else cerr << "output file could not be opened.\n";
    return 0;
  }
}

static void
draw_polar_grid (APL_Float min_xv,
		 APL_Float max_xv,
		 APL_Float min_yv,
		 APL_Float max_yv)
{
  double i;
  double lv;
  double fp, ep;
  APL_Float radx = fmax (fabs (min_xv), fabs (max_xv));
  APL_Float rady = fmax (fabs (min_yv), fabs (max_yv));
  APL_Float rad  = fmax (radx, rady);
  char buffer[64];

  lv = log10 (rad);
  ep = trunc (lv);
  fp = lv - ep;
  fp = exp10 (fp);
  fp = trunc (10.0 * fp) / 10.0;
  fp = round (fp);
  rad = fp * exp10 (ep);

  plwidth (0.65);		// fixme make settable

  snprintf(buffer, sizeof(buffer), "x 10#u%d#d", (int)ep);
  plmtex ("lv", 1.0, 0.9, 1.00, buffer);
  for(i = 0.1; i <= 1.1; i += 0.1) {
    plarc(0.0, 0.0, i * rad, i * rad, 0.0, 360.0, 0.0, 0);

    snprintf(buffer, sizeof(buffer), "%3.1f", i * fp);
    plptex( i * rad, rad / 50.0, 0.0, 10.0, 0, buffer);
    plptex(-i * rad, rad / 50.0, 0.0, 10.0, 0, buffer);
  }

  for(i = 0.0; i < 8.0; i += 1.0) {
    double ang = (i / 8.0) * 2.0 * M_PI;
    pljoin (0.0, 0.0, 1.1 * rad * cos (ang), 1.1 * rad * sin (ang));
  }
  plwidth (1.0);
}

static void
render_xy (APL_Float min_xv,
	   APL_Float max_xv,
	   APL_Float min_yv,
	   APL_Float max_yv,
	   vector<LineClass *> lines)
{
  plspage (0.0,  0.0, plot_width, plot_height, 0.0, 0.0);
  plscolbg (bgred, bggreen, bgblue);
  plinit ();

  int axis_val = (mode == APL_MODE_XY) 
    ? ((xlog ? 10 : 0) + (ylog ? 20 : 0)) : -2;
  plenv (min_xv, max_xv, min_yv, max_yv, 0, axis_val);
  if (mode != APL_MODE_XY) draw_polar_grid (min_xv, max_xv, min_yv, max_yv);

  pllab (xlabel.empty () ? "" : xlabel.c_str (),
	 ylabel.empty () ? "" : ylabel.c_str (),
	 tlabel.empty () ? "" : tlabel.c_str ());

  plssym (0.0, 0.75);
  for (int i = 0; i < lines.size (); i++) {
    plcol0 (2 + i % 14);
    if (draw & APL_DRAW_LINES)
      plline (lines[i]->count, lines[i]->xvec, lines[i]->yvec);
    if (draw & APL_DRAW_POINTS)
      plpoin (lines[i]->count, lines[i]->xvec, lines[i]->yvec, 2 + i % 30);
  }
  plcol0 (1);
}

static int
run_plot (APL_Float min_xv,
	  APL_Float max_xv,
	  APL_Float min_yv,
	  APL_Float max_yv,
	  vector<LineClass *> lines)
{
  plsdev (target.c_str ());
  if (0 == target.compare (DEF_XCAIRO))  {
    pid_t pid = fork ();
    if (pid == 0) {		// child
      setsid ();
    
	render_xy (min_xv, max_xv, min_yv, max_yv, lines);
    
	plend ();
	wait (NULL);
	exit (0);
      }
      return (int)pid;
    }
  else {
    char *tfile = NULL;
    FILE *po = open_file (&tfile);
    if (po) {
      plsfile (po);
      render_xy (min_xv, max_xv, min_yv, max_yv, lines);
      plend ();
    }
    if (embed) {
      write (plot_pipe_fd, tfile, strlen (tfile));
      syncfs (plot_pipe_fd);
      usleep (50000);
    }
    if (tfile) free (tfile);
    return 0;
  }
}

static void
killAllChildProcess(int ppid)
{
  /*****
	Thanks to sundq on stackoverflow for this.  (Maybe someday I'll get
	around to writing getcpid() to compliment getpid() and getppid().
   *****/

  char *command = NULL;

  asprintf(&command, "ps -ef|awk '$3==%d {print $2}'", ppid);
  if (command) {
    FILE *fp = popen(command,"r");
    if (fp) {
      char *buff = NULL;
      size_t len = 255;
      while (getline (&buff, &len, fp) >= 0) {
	long pid = atol (buff);
	kill ((pid_t)pid, SIGTERM);
      }
      fclose(fp);
      if (buff) free(buff);
    }
    free(command);
  }
  return;
}

static void
polarise (APL_Float *xv, APL_Float *yv)
{
  if (!xv || !yv) return;
  
  APL_Float tx = *xv; 
  APL_Float ty = *yv;
  
  switch (angle_units) {
  case APL_ANGLE_DEGREES:
    *xv = ty * cos (tx * M_PI / 180.0);
    *yv = ty * sin (tx * M_PI / 180.0);
    break;
  case APL_ANGLE_RADIANS:
    *xv = ty * cos (tx);
    *yv = ty * sin (tx);
    break;
  case APL_ANGLE_PI_RADIANS:
    *xv = ty * cos (tx * M_PI);
    *yv = ty * sin (tx * M_PI);
    break;
  }
}

static int
plot_xy (ShapeItem pxcol, Value_P B)
{
  int pid = -1;
  APL_Float xv, yv;
  vector<LineClass *> lines;

  ShapeItem rows = B->get_rows();
  ShapeItem cols = B->get_cols();
	
  APL_Float min_xv =  MAXDOUBLE;
  APL_Float max_xv = -MAXDOUBLE;
  APL_Float min_yv =  MAXDOUBLE;
  APL_Float max_yv = -MAXDOUBLE;

  loop (q, cols) {
    if (q == pxcol) continue;

    PLFLT *xvec = new PLFLT[(int)rows];
    PLFLT *yvec = new PLFLT[(int)rows];

    loop(p, rows) {
      if (pxcol != -1) {
	const Cell & cell_Bx = B->get_ravel (pxcol + p * cols);
	xv = cell_Bx.get_real_value ();
      }
      else {
	//	xv = (APL_Float)p;
	xv = xorigin + (xspan - xorigin) * (double)p / (double)rows;
      }
      if (xlog) xv = log (xv);
      
      const Cell & cell_By = B->get_ravel (q + p * cols);
      yv = cell_By.get_real_value ();
      if (ylog) yv = log (yv);

      if (mode != APL_MODE_XY) polarise (&xv, &yv);

      if (min_xv > xv) min_xv = xv;
      if (max_xv < xv) max_xv = xv;
      if (min_yv > yv) min_yv = yv;
      if (max_yv < yv) max_yv = yv;
	  
      xvec[p] = xv;
      yvec[p] = yv;
    }

    LineClass *line = new LineClass (rows, xvec, yvec);
    lines.push_back (line);
  }

  pid = run_plot (min_xv, max_xv, min_yv, max_yv, lines);

  
  for (int i = 0; i < lines.size (); i++)
    delete lines[i];
  
  return pid;
}

static int
plot_y (Value_P B)
{
  int pid = -1;
  APL_Float xv, yv;
  vector<LineClass *> lines;
	
  APL_Float min_xv =  MAXDOUBLE;
  APL_Float max_xv = -MAXDOUBLE;
  APL_Float min_yv =  MAXDOUBLE;
  APL_Float max_yv = -MAXDOUBLE;

  PLFLT *xvec = new PLFLT[B->element_count ()];
  PLFLT *yvec = new PLFLT[B->element_count ()];
	  
  loop(p, B->element_count ()) {
    const Cell & cell_B = B->get_ravel(p);
    xv = (APL_Float)p;
    yv = cell_B.get_real_value ();
    if (ylog) yv = log (yv);
    
    if (mode != APL_MODE_XY) polarise (&xv, &yv);
    
    if (min_xv > xv) min_xv = xv;
    if (max_xv < xv) max_xv = xv;
    if (min_yv > yv) min_yv = yv;
    if (max_yv < yv) max_yv = yv;
    
    xvec[p] = xv;
    yvec[p] = yv;
  }


  LineClass *line = new LineClass (B->element_count (), xvec, yvec);
  lines.push_back (line);

  pid = run_plot (min_xv, max_xv, min_yv, max_yv, lines);
  
  delete line;
  
  return pid;
}

static void
handle_opts ()
{
  if (keyword.empty ()) return;

  if (kwd_map.empty ())
    for (int i = 0; i < sizeof (kwds) / sizeof (kwd_s); i++)
      kwd_map [opt_kwd (i)] = i;

  if (kwd_map.find (keyword) != kwd_map.end ()) {
    int idx = -1;
    idx = kwd_map.at (keyword);
    if (idx >= 0) (*opt_fcn (idx))(opt_arg (idx));
  }
  else {
    cerr << "invalid option " << keyword << endl;
    // fixme -- complain abt bad kwd
  }
  
  keyword.clear ();
  args.clear ();
}

static void
set_kwd (string const& str)
{
  keyword = str;
}

static void
append_arg (string const& str)
{
  args.push_back (str);
}

template <typename Iterator>
struct option_parser : qi::grammar<Iterator, ascii::space_type> {
  option_parser() : option_parser::base_type(start) {
    using qi::int_;
    using qi::lit;
    using qi::lexeme;
    using ascii::char_;

    keyword_string  %= lexeme[+char_("0-9a-zA-Z_")];
    unquoted_string %= lexeme[+(char_ - ' ' - ';')];
    quoted_string   %= lexeme['"' >> +(char_ - '"') >> '"'];
    any_string      %= quoted_string | unquoted_string;

    start %= keyword_string[&set_kwd]
      >> *any_string[&append_arg]
      >> *( ',' >> any_string[&append_arg] )
      >> ';' ;
  }

  qi::rule<Iterator, string(), ascii::space_type> any_string;
  qi::rule<Iterator, string(), ascii::space_type> keyword_string;
  qi::rule<Iterator, string(), ascii::space_type> unquoted_string;
  qi::rule<Iterator, string(), ascii::space_type> quoted_string;
  qi::rule<Iterator,           ascii::space_type> start;
};

static Token
eval_B(Value_P B)
{
  int pid = -1;
  
  const ShapeItem count    = B->element_count();
  const Rank      rank     = B->get_rank();
  const CellType  celltype = B->deep_cell_types();

  if (!(celltype & ~CT_NUMERIC)) {
    if (celltype &  CT_COMPLEX) {
      if (rank == 1) {  // simple polar graph
	APL_Float xv, yv, zv;
	
	APL_Float min_xv =  MAXDOUBLE;
	APL_Float max_xv = -MAXDOUBLE;
	APL_Float min_yv =  MAXDOUBLE;
	APL_Float max_yv = -MAXDOUBLE;
	APL_Float min_zv =  MAXDOUBLE;
	APL_Float max_zv = -MAXDOUBLE;
	
	PLFLT *xvec = new PLFLT[count];
	PLFLT *yvec = new PLFLT[count];
	PLFLT *zvec = new PLFLT[count];
	
	loop(p, count) {
	  const Cell & cell_B = B->get_ravel(p);
	  xv = (APL_Float)p;
	  yv = cell_B.get_real_value ();
	  zv = cell_B.get_imag_value ();

	  if (min_xv > xv) min_xv = xv;
	  if (max_xv < xv) max_xv = xv;
	  if (min_yv > yv) min_yv = yv;
	  if (max_yv < yv) max_yv = yv;
	  if (min_zv > zv) min_zv = zv;
	  if (max_zv < zv) max_zv = zv;
	  
	  xvec[p] = xv;
	  yvec[p] = yv;
	  zvec[p] = zv;
	}

	pid = run_plot_z ((PLINT)count,
			  min_xv, max_xv,
			  min_yv, max_yv,
			  min_zv, max_zv,
			  xvec, yvec, zvec);

	delete [] xvec;
	delete [] yvec;
	delete [] zvec;
      }  
      else {	// don't know yet
	// rank error?
      }
    } else {			// real nrs
      if (killem) {
	/***
	    fixme -- this is dangerous.  check to make sure it's one of
	    our pids being killed.  (killed ppid == our ppid?)
	 ***/
	loop(p, count) {
	  const Cell & cell_B = B->get_ravel(p);
	  APL_Integer vv = cell_B.get_int_value ();
	  killAllChildProcess ((int)vv);
	}
	killem = false;
	return Token(TOK_APL_VALUE1, Str0_0 (LOC));
      }
      switch (rank) {
      case 1:			// simple xy graph w/ 0-based indices
	pid = plot_y (B);
	break;
      case 2:			// xy graph
	if (B->get_cols() == 1) pid = plot_y (B);
	if (B->get_cols() >= 2) pid = plot_xy (xcol, B);
	break;
      default:			// multi dimensional 
	break;
      }
    }
  }
  else {
    // non numeric error
  }

  return Token(TOK_APL_VALUE1, IntScalar (pid, LOC));
}

static Token
eval_AB(Value_P A, Value_P B)
{
   using namespace std;
   using namespace boost;

   if (plot_pipe_fd == -1) {
     char *plot_pipe_name = getenv ("APLPLOT");
     if (plot_pipe_name) 
       plot_pipe_fd = open (plot_pipe_name, O_WRONLY);
   }

   if (A->is_char_string()) {
     //const ShapeItem count    = A->element_count();
     const UCS_string  ustr	= A->get_UCS_ravel();		// unicode
     const UTF8_string utf (ustr);
     const string sstr (utf.c_str ());
     //     const string sstr ((const char *)(utf.get_items()), utf.size());
     //const Rank       rank    = A->get_rank();
     
     //     const string     sstr    = A->get_UCS_ravel().to_string (); // utf8

     using boost::spirit::ascii::space;
     typedef string::const_iterator iterator_type;
     typedef option_parser<iterator_type> option_parser;

     option_parser g;

     string::const_iterator iter = sstr.begin();
     string::const_iterator end  = sstr.end();

     while (iter != end) {
       bool rc = phrase_parse(iter, end, g, space);

       handle_opts ();
       if (!rc) break;
     }
   }

   Token rt = B->is_empty () ?
     Token(TOK_APL_VALUE1, Str0_0 (LOC)) : eval_B (B);
   return rt;
}

static Token
eval_XB(Value_P X, Value_P B)
{
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}

static Token
eval_AXB(Value_P A, Value_P X, Value_P B)
{
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}


static Token
eval_fill_B(Value_P B)
{
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}

static Token
eval_fill_AB(Value_P A, Value_P B)
{
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}

static Token
eval_ident_Bx(Value_P B, Axis x)
{
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}



