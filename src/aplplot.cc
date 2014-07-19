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
#include<iostream>
#include<string>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <values.h>
#include <plplot/plplot.h>

#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_URL
#undef PACKAGE_VERSION
#undef VERSION

#include "aplplot.hh"

using namespace std;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

#define DEFAULT_PLOT_WIDTH	512
#define DEFAULT_PLOT_HEIGHT	480

static int plot_width  = DEFAULT_PLOT_WIDTH;
static int plot_height = DEFAULT_PLOT_HEIGHT;
static char *xlabel    = NULL;
static char *ylabel    = NULL;
static char *tlabel    = NULL;
static int hdlr_set    = 0;

static void
sigchld_handler (int sig)
{
  int status;
  waitpid (-1, &status, WNOHANG);
}

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
  pid_t pid = fork ();
  if (pid == 0) {		// child
    setsid ();
    FILE *po = popen ("display /dev/stdin", "w");
    plsfile (po);
    plsdev ("pngcairo");


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
    

    plinit ();

    pladv (0);
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    plwind( -512.00, 512.0, -200.0, 600.0);
    
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
    plend ();
    exit (0);
  }
  return (int)pid;
}

static int
run_plot (PLINT count,
	  APL_Float min_xv,
	  APL_Float max_xv,
	  APL_Float min_yv,
	  APL_Float max_yv,
	  PLFLT *xvec,
	  PLFLT *yvec)
{
  pid_t pid = fork ();
  if (pid == 0) {		// child
    setsid ();
    FILE *po = popen ("display /dev/stdin", "w");
    plsfile (po);
    plsdev ("pngcairo");

    //       xdpi  ydpi xlen   ylen   xoff yoff
    plspage (0.0,  0.0, plot_width, plot_height, 0.0, 0.0);
    plinit ();
    plenv (min_xv, max_xv, min_yv, max_yv, 0, 0 );

    pllab (xlabel, ylabel, tlabel);
    
    plline (count, xvec, yvec);
    plend ();
    exit (0);
  }
  return (int)pid;
}

static Token
eval_B(Value_P B)
{
  int pid = -1;
  
  const ShapeItem count    = B->element_count();
  const Rank      rank     = B->get_rank();
  const CellType  celltype = B->compute_cell_types();

  if (!hdlr_set) {
    signal (SIGCHLD, sigchld_handler);
    hdlr_set = 1;
  }
  
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
    }else {			// real nrs
      if (rank == 1) {		// simple xy graph w/ 0-based indices
	APL_Float xv, yv;
	
	APL_Float min_xv =  MAXDOUBLE;
	APL_Float max_xv = -MAXDOUBLE;
	APL_Float min_yv =  MAXDOUBLE;
	APL_Float max_yv = -MAXDOUBLE;

	PLFLT *xvec = new PLFLT[count];
	PLFLT *yvec = new PLFLT[count];
	  
	loop(p, count) {
	  const Cell & cell_B = B->get_ravel(p);
	  xv = (APL_Float)p;
	  yv = cell_B.get_real_value ();

	  if (min_xv > xv) min_xv = xv;
	  if (max_xv < xv) max_xv = xv;
	  if (min_yv > yv) min_yv = yv;
	  if (max_yv < yv) max_yv = yv;
	  
	  xvec[p] = xv;
	  yvec[p] = yv;
	}

	pid = run_plot ((PLINT)count, min_xv, max_xv, min_yv, max_yv,
			xvec, yvec);

	delete [] xvec;
	delete [] yvec;
      } 
      else if (rank == 2) {	// simple xy graph
	// fixme -- try get_last_shape_item()
	if (B->get_shape_item(1) == 2) {  // [m 2]: x y, x y, ...
	  APL_Float xv, yv;
	
	  APL_Float min_xv =  MAXDOUBLE;
	  APL_Float max_xv = -MAXDOUBLE;
	  APL_Float min_yv =  MAXDOUBLE;
	  APL_Float max_yv = -MAXDOUBLE;

	  const ShapeItem halfcount = count / 2;
	  
	  PLFLT *xvec = new PLFLT[halfcount];
	  PLFLT *yvec = new PLFLT[halfcount];

	  loop(p, halfcount) {
	    // xvecs will be even indices
	    // yvecs will be odd  indices
	  
	    const Cell & cell_Bx = B->get_ravel(2 * p);
	    const Cell & cell_By = B->get_ravel(2 * p + 1);
	    xv = cell_Bx.get_real_value ();
	    yv = cell_By.get_real_value ();

	    if (min_xv > xv) min_xv = xv;
	    if (max_xv < xv) max_xv = xv;
	    if (min_yv > yv) min_yv = yv;
	    if (max_yv < yv) max_yv = yv;
	  
	    xvec[p] = xv;
	    yvec[p] = yv;
	  }

	  pid = run_plot ((PLINT)halfcount, min_xv, max_xv, min_yv, max_yv,
			  xvec, yvec);

	  delete [] xvec;
	  delete [] yvec;
	}
	// fixme -- try get_last_shape_item()
	else if (B->get_shape_item(1) == 3) {  // [m 3]:  x y z, x y z, ...
	}
	else {			// [m n]: don't know yet
	// rank error?		// see Value.hh for rows cols
	}
      }
      else {			// [l m n...]:  don't know yet
	// rank error?
      }
    }
  }
  else {
    // non numeric error
  }
  
  const IntCell apid ((APL_Integer)pid);
  Value_P Z(new Value(apid, LOC));
  Z->set_default(*Value::Zero_P);
  Z->check_value(LOC);
  return Token(TOK_APL_VALUE1, Z);
}

string keyword;
vector<string> args;

static void
handle_opts ()
{
#if 0
  cerr << "keyword = \"" << keyword << "\"" << endl;
  for (int i = 0; i < args.size (); i++)
    cerr << "\targ[" << i << "] = \"" << args[i] << "\"" << endl;
#endif

  if (keyword.empty ()) return;
  
  if (0 == keyword.compare ("width")) {
    if (args.size () >= 1) {
      istringstream (args[0]) >> plot_width;
    }
  }
  else if (0 == keyword.compare ("height")) {
    if (args.size () >= 1) {
      istringstream (args[0]) >> plot_height;
      cerr << "h set to " << plot_height << endl;
    }
  }
  else if (0 == keyword.compare ("dims")) {
    if (args.size () >= 2) {
      istringstream (args[0]) >> plot_width;
      istringstream (args[1]) >> plot_height;
    }
  }
  else {
    // fixme -- complain abt bad kwd
  }
  
  keyword.erase ();
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

    unquoted_string %= lexeme[+char_("0-9a-zA-Z_")];
    quoted_string   %= lexeme['"' >> +(char_ - '"') >> '"'];
    any_string      %= quoted_string | unquoted_string;

    start %= unquoted_string[&set_kwd]
      >> *any_string[&append_arg]
      >> *( ',' >> any_string[&append_arg] )
      >> ';' ;
  }

  qi::rule<Iterator, string(), ascii::space_type> any_string;
  qi::rule<Iterator, string(), ascii::space_type> unquoted_string;
  qi::rule<Iterator, string(), ascii::space_type> quoted_string;
  qi::rule<Iterator,           ascii::space_type> start;
};

static Token
eval_AB(Value_P A, Value_P B)
{
   using namespace std;
   using namespace boost;

   if (A->is_char_string()) {
     //const ShapeItem count    = A->element_count();
     //const UCS_string ustr    = A->get_UCS_ravel();		// unicode
     /***
	 A->remove_trailing_padchars()
	 A->remove_copy_black()			// strip leading/trailing ws
	 A->drop(int drop_count)			// drop first
	 A->remove_pad()
	 A->remove_lt_spaces()
	 A->atoi()
	 A->compare("mmm", ?)
     ***/
     const string     sstr    = A->get_UCS_ravel().to_string (); // utf8
     //const char      *cstr    = sstr.c_str ();
     //const Rank	     rank    = A->get_rank();

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
     Token(TOK_APL_VALUE1, Value::Str0_0_P) : eval_B (B);
   return rt;
}

static Token
eval_XB(Value_P X, Value_P B)
{
UCS_string ucs("aplplot eval_XB() called");
Value_P Z(new Value(ucs, LOC));
   Z->check_value(LOC);
   return Token(TOK_APL_VALUE1, Z);
}

static Token
eval_AXB(Value_P A, Value_P X, Value_P B)
{
UCS_string ucs("aplplot eval_AXB() called");
Value_P Z(new Value(ucs, LOC));
   Z->check_value(LOC);
   return Token(TOK_APL_VALUE1, Z);
}


static Token
eval_fill_B(Value_P B)
{
UCS_string ucs("aplplot eval_fill_B() called");
Value_P Z(new Value(ucs, LOC));
   Z->check_value(LOC);
   return Token(TOK_APL_VALUE1, Z);
}

static Token
eval_fill_AB(Value_P A, Value_P B)
{
UCS_string ucs("aplplot eval_fill_B() called");
Value_P Z(new Value(ucs, LOC));
   Z->check_value(LOC);
   return Token(TOK_APL_VALUE1, Z);
}

static Token
eval_ident_Bx(Value_P B, Axis x)
{
UCS_string ucs("aplplot eval_ident_Bx() called");
Value_P Z(new Value(ucs, LOC));
   Z->check_value(LOC);
   return Token(TOK_APL_VALUE1, Z);
}



