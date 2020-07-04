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
