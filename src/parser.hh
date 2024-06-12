#pragma once
#include <boost/spirit/include/qi.hpp>
#include <boost/unordered_map.hpp>
#include "aplplot_menu.h"
#define cfg_ASSERT_LEVEL_WANTED 0
#define cfg_SHORT_VALUE_LENGTH_WANTED 1
#include <Native_interface.hh>

using namespace std;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

extern string keyword;
extern vector<string> args;

extern Value_P global_B;

typedef boost::unordered_map<std::string, int> si_map;
extern si_map  kwd_map;

extern aplplot_menu_t aplplot_menu;
extern void *menu_handle;

typedef void (*option_fcn)(int);
typedef struct { std::string kwdx; option_fcn fcnx; int argx;} kwd_s;
extern kwd_s kwds[];
#define opt_kwd(i) kwds[i].kwdx
#define opt_fcn(i) kwds[i].fcnx
#define opt_arg(i) kwds[i].argx

extern int		plot_width;
extern int		plot_height;
extern string		xlabel;
extern string		ylabel;
extern string		tlabel;
extern bool		xlog;
extern bool		ylog;
extern int		mode;
extern int		angle_units;
extern bool		killem;
extern int		xcol;
extern int		draw;
extern string		filename;
extern bool		embed;
extern bool		menu;
extern int		target_idx;
extern unsigned char	bgred;
extern unsigned char	bggreen;
extern unsigned char	bgblue;
extern double		xorigin;
extern double		xspan;
extern int		plot_pipe_fd;
extern int		axis;

extern int nr_kwds;
