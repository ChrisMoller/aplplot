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
define(`count',0)dnl
define(`bump',`define(`count',incr(count))')dnl
define(`upcase', `translit($1, `a-z', `A-Z')')dnl
define(`entry',`  `DEF_'upcase($1)`,'divert(1)
  {"$1", "$2", "$3"},divert(0)')dnl
divert(1)`
typedef struct {
  const char *mode;
  const char *target;
  const char *ext;
  } mode_strings_s;
#ifndef DO_INIT
extern
#endif
mode_strings_s mode_strings[MODES_MAX_ENTRY]
#ifdef DO_INIT
= {
'divert(0)dnl
#ifndef APL_PLOT_MODES_H
#define APL_PLOT_MODES_H
`typedef enum {'
entry(`screen', `xcairo',     `png')
entry(`png',    `pngcairo',   `png')
entry(`pdf',    `pdfcairo',   `pdf')
entry(`ps',     `pscairo',    `ps')
entry(`eps',    `epscairo',   `eps')
entry(`svg',    `svgcairo',   `svg')
`  MODES_MAX_ENTRY'
`} modes_e;'
divert(1)`
}

#endif
#endif /* APLPLOT_MODES_H */
;
'
