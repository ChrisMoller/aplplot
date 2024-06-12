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

static FILE *
open_pipe ()
{
  int pipefd[2];
  pipe(pipefd);
  pid_t pc = fork ();

  if (pc == 0) {				// child
    close(pipefd[1]);
    int drc = dup2 (pipefd[0], STDIN_FILENO);
    execlp ("gnuplot", "-p", (char  *) NULL);	// returns only on failure
    return nullptr;
  }
  else {					// parent
    close(pipefd[0]);
    FILE *gp = fdopen (pipefd[1], "w");
    if (!gp) {
      INTERNAL_ERROR;	// doesn't return;
      return nullptr;
    }
    else
      return gp;
  }
}

#define GP_CLEAR "set object rectangle from screen 0,0 to screen 1,1 \
behind fillcolor rgb 'white' fillstyle solid noborder\n"

static void
plot_simple_y (vector<double> *rvec, vector<double> *ivec, int extr)
{
  fprintf (stderr, "plot_simple_y\n");
  FILE *gp = open_pipe ();

  fprintf (gp, "$Mydata << EOD\n");
  switch(extr) {
  case APL_EXTRACT_REAL:
    loop (i, rvec->size ())
      fprintf (gp, "%g\n", (*rvec)[i]);
    break;
  case APL_EXTRACT_IMAGINARY:
    loop (i, rvec->size ())
      fprintf (gp, "%g\n", (*ivec)[i]);
    break;
  case APL_EXTRACT_MAGNITUDE:
    loop (i, rvec->size ())
      fprintf (gp, "%g\n", hypot ((*ivec)[i], (*rvec)[i]));
    break;
  case APL_EXTRACT_PHASE:
    loop (i, rvec->size ())
      fprintf (gp, "%g\n", atan2 ((*ivec)[i], (*rvec)[i]));
    break;
  }
  fprintf (gp, "EOD\n");

  fprintf (gp, GP_CLEAR);
  fprintf (gp, "plot $Mydata with linespoints t \"whatever\"\n");

  fflush (gp);
  //  fclose (gp);
}


static Token
eval_XB(Value_P X, Value_P B)
{
  const ShapeItem count    = B->element_count();
  const auto      rank     = B->get_rank();
  const CellType  celltype = B->deep_cell_types();

  if (!(celltype & ~CT_NUMERIC)) {
    if (celltype &  CT_COMPLEX) {		// complex
      switch(rank) {
      case 1:
	{
	  vector<double> rvec (count);
	  vector<double> ivec (count);
	  loop(p, count) {
	    const Cell & cell_B = B->get_cravel(p);
	    rvec[p] = (double)(cell_B.get_real_value ());
	    ivec[p] = (double)(cell_B.get_imag_value ());
	  }
	  plot_simple_y (&rvec, &ivec, extract);
	}
	break;
      case 2:
	break;
      case 3:
	break;
      default:
        RANK_ERROR;
	break;
      }
    }
    else {
      switch(rank) {				// real
      case 1:
	{
	  vector<double> rvec (count);
	  loop(p, count) {
	    const Cell & cell_B = B->get_cravel(p);
	    rvec[p] = (double)(cell_B.get_real_value ());
	  }
	  plot_simple_y (&rvec, nullptr, APL_EXTRACT_REAL);
	}
	break;
      case 2:
	break;
      case 3:
	break;
      default:
        RANK_ERROR;
	break;
      }
    }
  }
  return Token(TOK_APL_VALUE1, Str0_0 (LOC));
}


static Token
eval_AB(Value_P A, Value_P B)
{
  if (A->is_char_string()) {
    const UCS_string  ustr     = A->get_UCS_ravel();           // unicode
    const UTF8_string utf (ustr);
    parse_opts (utf.c_str ());
  }

  global_B = B;
  
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


