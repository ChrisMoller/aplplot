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
extern unsigned char	bgalpha;
extern unsigned char	bgred;
extern unsigned char	bggreen;
extern unsigned char	bgblue;
extern double		xorigin;
extern double		xspan;
extern int		plot_pipe_fd;
extern int		axis;
extern int		extract;

extern int nr_kwds;

extern void parse_opts (const string sstr);
