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
#define cfg_ASSERT_LEVEL_WANTED 0
#define cfg_SHORT_VALUE_LENGTH_WANTED 1
#include <complex> 
#include <vector> 

#include <sys/types.h>
#include <sys/wait.h>
#include <alloca.h>
#include <math.h>
#include <signal.h>
#include <values.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <plplot/plplot.h>

#include "parser.hh"
#define DO_INIT
#include "modes.h"
#include "aplplot.hh"
#include "aplplot_menu.h"

using namespace std;

class ComplexVec
{
public:
  ComplexVec (vector<PLFLT> rv, vector<PLFLT> iv)
  {
    rvec = rv;
    ivec = iv;
  }
  vector<PLFLT> rvec;
  vector<PLFLT> ivec;
};

class ComplexVecVec
{
public:
  ComplexVecVec (vector<PLFLT> nv,
		 APL_Float min_r,
		 APL_Float max_r,
		 APL_Float min_i,
		 APL_Float max_i)
  {
    nvec = nv;
    min_rv = min_r;
    max_rv = max_r;
    min_iv = min_i;
    max_iv = max_i;
  }
  void push (vector<PLFLT> rvec, vector<PLFLT> ivec)
  {
    fprintf (stderr, "pushing %d %d\n",
	     (int)(rvec.size ()),
	     (int)(ivec.size ()));
    ComplexVec cv (rvec, ivec);
    cvec.push_back (cv);
  }
  vector<PLFLT> nvec;
  vector<ComplexVec> cvec;
  APL_Float min_rv;
  APL_Float max_rv;
  APL_Float min_iv;
  APL_Float max_iv;
};

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_URL
#undef PACKAGE_VERSION
#undef VERSION

Value_P global_B;

// plot *0j.1×○⍳700

static void
#ifdef OLD_STYLE
render_z (vector<PLFLT> rvec,
	  vector<PLFLT> ivec,
	  vector<PLFLT> nvec,
	  APL_Float min_rv,
	  APL_Float max_rv,
	  APL_Float min_iv,
	  APL_Float max_iv)
#else
render_z (ComplexVecVec cvv)
#endif
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
	 cvv.min_rv,	// xmin,
	 cvv.max_rv,	// xmax,
	 cvv.min_iv,	// ymin,
	 cvv.max_iv,	// ymax,
	 0.0,	// zmin,
	 cvv.nvec.back (),	// zmax,
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

#ifdef OLD_STYLE
  plline3 (nvec.size (), rvec.data (), ivec.data (), nvec.data ());
#else
  fprintf (stderr, "ns = %d, cs = %d\n",
	   (int)(cvv.nvec.size ()),
	   (int)(cvv.cvec.size ())
	   );
  loop (c, cvv.cvec.size ()) {
    ComplexVec vc = cvv.cvec[c];
    fprintf (stderr, "cs = %d, rs is = %d %d\n",
	     (int)cvv.cvec.size (),
	     (int)(vc.rvec.size ()),
	     (int)(vc.ivec.size ()));
#if 1
    plline3 (cvv.nvec.size (),
	     vc.rvec.data (),
	     vc.ivec.data (),
	     cvv.nvec.data ());
#endif
  }
#endif
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
    else UERR << "missing filename\n";
  }
  return rf;
}

static int
#ifdef OLD_STYLE
run_plot_z (vector<PLFLT> rvec,
	    vector<PLFLT> ivec,
	    vector<PLFLT> nvec,
	    APL_Float min_rv,
	    APL_Float max_rv,
	    APL_Float min_iv,
	    APL_Float max_iv)
#else
  run_plot_z (ComplexVecVec cvv)
#endif
{
  plspage (0.0,  0.0, plot_width, plot_height, 0.0, 0.0);
  plsdev (mode_strings[target_idx].target);
  if (target_idx == DEF_SCREEN)  {
    pid_t pid = fork ();
    if (pid == 0) {		// child
      setsid ();

#ifdef OLD_STYLE
      render_z (rvec, ivec, nvec,
		min_rv, max_rv,
		min_iv, max_iv);
#else
      render_z (cvv);
#endif

      plend ();
      wait (NULL);
      exit (0);
    }
    return (int)pid;
  }
  else {		// to file
    char *tfile = NULL;
    FILE *po = open_file (&tfile);

    if (po) {
      plsfile (po);

#ifdef OLD_STYLE
      render_z (rvec, ivec, nvec,
		min_rv, max_rv,
		min_iv, max_iv);
#else
      render_z (cvv);
#endif

      plend ();
      if (embed) {
	write (plot_pipe_fd, tfile, strlen (tfile));
	syncfs (plot_pipe_fd);
	usleep (50000);
      }
      if (tfile) free (tfile);
    }
    else UERR << "output file could not be opened.\n";
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

#ifdef OLD_STYLE
static void
render_xy (APL_Float min_xv,
	   APL_Float max_xv,
	   APL_Float min_yv,
	   APL_Float max_yv,
	   vector<LineClass *> lines)
{
  fprintf (stderr, "render xy %g %g %g %g\n",
	   min_xv, max_xv, min_yv, max_yv);
  plspage (0.0,  0.0, plot_width, plot_height, 0.0, 0.0);
  plscolbg (bgred, bggreen, bgblue);
  plinit ();

  if ((draw & APL_DRAW_LINES) || (draw & APL_DRAW_POINTS)) {

    int axis_val = (mode == APL_MODE_XY) 
      ? ((xlog ? 10 : 0) + (ylog ? 20 : 0)) : -2;
    plenv (min_xv, max_xv, min_yv, max_yv, 0, axis_val);
    if (mode != APL_MODE_XY) draw_polar_grid (min_xv, max_xv, min_yv, max_yv);

    for (int i = 0; i < lines.size (); i++) {
      plcol0 (2 + i % 14);
      if (draw & APL_DRAW_LINES)
	plline (lines[i]->count, lines[i]->xvec, lines[i]->yvec);
      if (draw & APL_DRAW_POINTS)
	plpoin (lines[i]->count, lines[i]->xvec, lines[i]->yvec, 2 + i % 30);
    }
    plcol0 (1);
  }
  else if (draw & draw & APL_DRAW_SCATTER) {
    APL_Float amin_xv =  MAXDOUBLE;
    APL_Float amax_xv = -MAXDOUBLE;
    APL_Float amin_yv =  MAXDOUBLE;
    APL_Float amax_yv = -MAXDOUBLE;

    fprintf (stderr, "ls = %d\n", lines.size ());
    if (lines.size () >= 2) {
      for (int i = 0; i < lines[0]->count; i++) {
	double xv =  lines[0]->yvec[i];
	double yv =  lines[1]->yvec[i];
	
	if (amin_xv > xv) amin_xv = xv;
	if (amax_xv < xv) amax_xv = xv;
	if (amin_yv > yv) amin_yv = yv;
	if (amax_yv < yv) amax_yv = yv;
      }
    
      plenv (amin_xv, amax_xv, amin_yv, amax_yv, 0, 2);
      pllab (xlabel.empty () ? "" : xlabel.c_str (),
	     ylabel.empty () ? "" : ylabel.c_str (),
	     tlabel.empty () ? "" : tlabel.c_str ());
      plssym (0.0, 0.75);
    
      plpoin (lines[0]->count, lines[0]->yvec, lines[1]->yvec, 2);
    }
      //    else {
      //      UERR << "Not enough information.\n";
      //    }
  }
}
#endif

#ifdef OLD_STYLE
static int
run_plot (APL_Float min_xv,
	  APL_Float max_xv,
	  APL_Float min_yv,
	  APL_Float max_yv,
	  vector<LineClass *> lines)
{
  plsdev (mode_strings[target_idx].target);
  if (target_idx == DEF_SCREEN)  {
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
#endif

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
#if 0
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
	const Cell & cell_Bx = B->get_cravel (pxcol + p * cols);
	xv = cell_Bx.get_real_value ();
      }
      else {
	//	xv = (APL_Float)p;
	xv = xorigin + (xspan - xorigin) * (double)p / (double)rows;
      }
      if (xlog) xv = log (xv);
      
      const Cell & cell_By = B->get_cravel (q + p * cols);
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
#endif
  return pid;
}

static int
plot_y (Value_P B)
{
  int pid = -1;
#if 0
  APL_Float xv, yv;
  vector<LineClass *> lines;
	
  APL_Float min_xv =  MAXDOUBLE;
  APL_Float max_xv = -MAXDOUBLE;
  APL_Float min_yv =  MAXDOUBLE;
  APL_Float max_yv = -MAXDOUBLE;

  PLFLT *xvec = new PLFLT[B->element_count ()];
  PLFLT *yvec = new PLFLT[B->element_count ()];
	  
  loop(p, B->element_count ()) {
    const Cell & cell_B = B->get_cravel(p);
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
#endif
  return pid;
}

static void
handle_opts ()
{
  if (keyword.empty ()) return;

  if (kwd_map.empty ())
    for (int i = 0; i < nr_kwds; i++)
      kwd_map [opt_kwd (i)] = i;

  if (kwd_map.find (keyword) != kwd_map.end ()) {
    int idx = -1;
    idx = kwd_map.at (keyword);
    if (idx >= 0) (*opt_fcn (idx))(opt_arg (idx));
  }
  else {
    UERR << "invalid option " << keyword << endl;
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
eval_XB(Value_P X, Value_P B)
{
  int pid = -1;

  if (X->is_numeric_scalar()) {
    int v = (int)(X->get_sole_integer ());
    if (v != -2) axis = v;
  }
  
  const ShapeItem count    = B->element_count();
  const auto      rank     = B->get_rank();
  const CellType  celltype = B->deep_cell_types();

  if (!(celltype & ~CT_NUMERIC)) {
    if (celltype &  CT_COMPLEX) {

      APL_Float rv, iv;
      APL_Float min_rv =  MAXDOUBLE;
      APL_Float max_rv = -MAXDOUBLE;
      APL_Float min_iv =  MAXDOUBLE;
      APL_Float max_iv = -MAXDOUBLE;

      if (rank == 1) {  // complex vector: xy complex plane, z indep vbl
	vector<PLFLT> rvec (count);
	vector<PLFLT> ivec (count);
	vector<PLFLT> nvec (count);
	
	loop(p, count) {
	  const Cell & cell_B = B->get_cravel(p);
	  rv = rvec[p] = (PLFLT)(cell_B.get_real_value ());
	  iv = ivec[p] = (PLFLT)(cell_B.get_imag_value ());
	  nvec[p] = (PLFLT)p;

	  if (min_rv > rv) min_rv = rv;
	  if (max_rv < rv) max_rv = rv;
	  if (min_iv > iv) min_iv = iv;
	  if (max_iv < iv) max_iv = iv;
	}

#ifdef OLD_STYLE
	pid = run_plot_z (rvec, ivec, nvec,
			  min_rv, max_rv,
			  min_iv, max_iv);
#else
	ComplexVecVec cvv (nvec,  min_rv, max_rv, min_iv, max_iv);
	cvv.push (rvec, ivec);
	pid = run_plot_z (cvv);
#endif

      }  
      else {	// complex matrix: n vectors as abov
	ShapeItem rows = B->get_rows();
	ShapeItem cols = B->get_cols();

	vector<PLFLT> rvec (cols);
	vector<PLFLT> ivec (cols);
	vector<PLFLT> nvec (cols);	

	loop(p, count) {
	  const Cell & cell_B = B->get_cravel(p);
	  rv = (PLFLT)(cell_B.get_real_value ());
	  iv = (PLFLT)(cell_B.get_imag_value ());
	  if (min_rv > rv) min_rv = rv;
	  if (max_rv < rv) max_rv = rv;
	  if (min_iv > iv) min_iv = iv;
	  if (max_iv < iv) max_iv = iv;
	}

	loop (c, cols)
	  nvec[c] = (PLFLT)c;

	loop (r, rows) {
	  int p = r * (int)cols;
	  loop (c, cols) {
	    const Cell & cell_B = B->get_cravel(p + c);
	    rvec[p] = (PLFLT)(cell_B.get_real_value ());
	    ivec[p] = (PLFLT)(cell_B.get_imag_value ());
	  }
	  
	  
#ifdef OLD_STYLE
	  pid = run_plot_z (rvec, ivec, nvec,
			    min_rv, max_rv,
			    min_iv, max_iv);
#else
	ComplexVecVec cvv (nvec,  min_rv, max_rv, min_iv, max_iv);
	pid = run_plot_z (cvv);
#endif
	}
	
      }
    } else {			// real nrs
      if (killem) {
	/***
	    fixme -- this is dangerous.  check to make sure it's one of
	    our pids being killed.  (killed ppid == our ppid?)
	 ***/
	loop(p, count) {
	  const Cell & cell_B = B->get_cravel(p);
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
	RANK_ERROR;
	break;
      }
    }
  }
  else if (B->is_char_string()) {
    fprintf (stderr, "GGGGGGGGGGGGGGGG\n");
    return Token(TOK_APL_VALUE1, Str0_0 (LOC));
  }
  else {
    DOMAIN_ERROR;
    // non numeric error
  }

  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
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

     global_B = B;
     
     while (iter != end) {
       bool rc = phrase_parse(iter, end, g, space);

       handle_opts ();
       if (!rc) break;
     }
   }

   Token rt = (menu || B->is_empty ()) ?
     Token(TOK_APL_VALUE1, Str0_0 (LOC)) : eval_B (B);
   return rt;
}

Token
eval_B(Value_P B)
{
  Value_P X = IntScalar (-2, LOC);
  return eval_XB (X, B);
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
eval_ident_Bx(Value_P B, sAxis x)
{
  return Token(TOK_APL_VALUE1, Str0(LOC));
}


void *
get_function_mux(const char * function_name)
{
  if (!menu_handle) {
    menu_handle = dlopen ("/usr/local/lib/apl/libaplplot_menu.so", RTLD_NOW);
    aplplot_menu = (aplplot_menu_t)dlsym (menu_handle, "aplplot_menu");
  }
  if (!strcmp(function_name, "get_signature"))
    return reinterpret_cast<void *>(&get_signature);
  if (!strcmp(function_name, "eval_B"))
    return reinterpret_cast<void *>(&eval_B);
  if (!strcmp(function_name, "eval_AB"))
    return reinterpret_cast<void *>(&eval_AB);
  if (!strcmp(function_name, "eval_XB"))
    return reinterpret_cast<void *>(&eval_XB);
  if (!strcmp(function_name, "eval_AXB"))
    return reinterpret_cast<void *>(&eval_AXB);
  if (!strcmp(function_name, "eval_fill_B"))
    return reinterpret_cast<void *>(&eval_fill_B);
  if (!strcmp(function_name, "eval_fill_AB"))
    return reinterpret_cast<void *>(&eval_fill_AB);
  if (!strcmp(function_name, "eval_ident_Bx"))
    return reinterpret_cast<void *>(&eval_ident_Bx);
  return 0;
}


