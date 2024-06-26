#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "colours.hh"

colours_s colours[] = {
  { "white"           , 255 , 255 , 255 },
  { "black"           ,   0 ,   0 ,   0 },
  { "dark-grey"       , 160 , 160 , 160 },
  { "red"             , 255 ,   0 ,   0 },
  { "web-green"       ,   0 , 192 ,   0 },
  { "web-blue"        ,   0 , 128 , 255 },
  { "dark-magenta"    , 192 ,   0 , 255 },
  { "dark-cyan"       ,   0 , 238 , 238 },
  { "dark-orange"     , 192 ,  64 ,   0 },
  { "dark-yellow"     , 200 , 200 ,   0 },
  { "royalblue"       ,  65 , 105 , 225 },
  { "goldenrod"       , 255 , 192 ,  32 },
  { "dark-spring-green",  0 , 128 ,  64 },
  { "purple"          , 192 , 128 , 255 },
  { "steelblue"       ,  48 ,  96 , 128 },
  { "dark-red"        , 139 ,   0 ,   0 },
  { "dark-chartreuse" ,  64 , 128 ,   0 },
  { "orchid"          , 255 , 128 , 255 },
  { "aquamarine"      , 127 , 255 , 212 },
  { "brown"           , 165 ,  42 ,  42 },
  { "yellow"          , 255 , 255 ,   0 },
  { "turquoise"       ,  64 , 224 , 208 },

  /* greyscale gradient */

  { "grey0"           ,   0 ,   0 ,   0 },
  { "grey10"          ,  26 ,  26 ,  26 },
  { "grey20"          ,  51 ,  51 ,  51 },
  { "grey30"          ,  77 ,  77 ,  77 },
  { "grey40"          , 102 , 102 , 102 },
  { "grey50"          , 127 , 127 , 127 },
  { "grey60"          , 153 , 153 , 153 },
  { "grey70"          , 179 , 179 , 179 },
  { "grey"            , 192 , 192 , 192 },
  { "grey80"          , 204 , 204 , 204 },
  { "grey90"          , 229 , 229 , 229 },
  { "grey100"         , 255 , 255 , 255 },

  /* random other colors */

  { "light-red"       , 240 ,  50 ,  50 },
  { "light-green"     , 144 , 238 , 144 },
  { "light-blue"      , 173 , 216 , 230 },
  { "light-magenta"   , 240 ,  85 , 240 },
  { "light-cyan"      , 224 , 255 , 255 },
  { "light-goldenrod" , 238 , 221 , 130 },
  { "light-pink"      , 255 , 182 , 193 },
  { "light-turquoise" , 175 , 238 , 238 },
  { "gold"            , 255 , 215 ,   0 },
  { "green"           ,   0 , 255 ,   0 },
  { "dark-green"      ,   0 , 100 ,   0 },
  { "spring-green"    ,   0 , 255 , 127 },
  { "forest-green"    ,  34 , 139 ,  34 },
  { "sea-green"       ,  46 , 139 ,  87 },
  { "blue"            ,   0 ,   0 , 255 },
  { "dark-blue"       ,   0 ,   0 , 139 },
  { "midnight-blue"   ,  25 ,  25 , 112 },
  { "navy"            ,   0 ,   0 , 128 },
  { "medium-blue"     ,   0 ,   0 , 205 },
  { "skyblue"         , 135 , 206 , 235 },
  { "cyan"            ,   0 , 255 , 255 },
  { "magenta"         , 255 ,   0 , 255 },
  { "dark-turquoise"  ,   0 , 206 , 209 },
  { "dark-pink"       , 255 ,  20 , 147 },
  { "coral"           , 255 , 127 ,  80 },
  { "light-coral"     , 240 , 128 , 128 },
  { "orange-red"      , 255 ,  69 ,   0 },
  { "salmon"          , 250 , 128 , 114 },
  { "dark-salmon"     , 233 , 150 , 122 },
  { "khaki"           , 240 , 230 , 140 },
  { "dark-khaki"      , 189 , 183 , 107 },
  { "dark-goldenrod"  , 184 , 134 ,  11 },
  { "beige"           , 245 , 245 , 220 },
  { "olive"           , 160 , 128 ,  32 },
  { "orange"          , 255 , 165 ,   0 },
  { "violet"          , 238 , 130 , 238 },
  { "dark-violet"     , 148 ,   0 , 211 },
  { "plum"            , 221 , 160 , 221 },
  { "dark-plum"       , 144 ,  80 ,  64 },
  { "dark-olivegreen" ,  85 , 107 ,  47 },

  { "orangered4"      , 128 ,  20 ,   0 },
  { "brown4"          , 128 ,  20 ,  20 },
  { "sienna4"         , 128 ,  64 ,  20 },
  { "orchid4"         , 128 ,  64 , 128 },
  { "mediumpurple3"   , 128 ,  96 , 192 },
  { "slateblue"       , 128 ,  96 , 255 },
  { "yellow4"         , 128 , 128 ,   0 },
  { "sienna1"         , 255 , 128 ,  64 },
  { "tan1"            , 255 , 160 ,  64 },
  { "sandybrown"      , 255 , 160 ,  96 },
  { "light-salmon"    , 255 , 160 , 112 },
  { "pink"            , 255 , 192 , 192 },
  { "khaki1"          , 255 , 255 , 128 },
  { "lemonchiffon"    , 255 , 255 , 192 },
  { "bisque"          , 205 , 183 , 158 },
  { "honeydew"        , 240 , 255 , 240 },
  { "slategrey"       , 160 , 182 , 205 },
  { "seagreen"        , 193 , 255 , 193 },
  { "antiquewhite"    , 205 , 192 , 176 },
  { "chartreuse"      , 124 , 255 ,  64 },
  { "greenyellow"     , 160 , 255 ,  32 },

  /* Synonyms */

  { "gray"            , 190 , 190 , 190 },
  { "light-gray"      , 211 , 211 , 211 },
  { "light-grey"      , 211 , 211 , 211 },
  { "dark-gray"       , 160 , 160 , 160 },
  { "slategray"       , 160 , 182 , 205 },
  { "gray0"           ,   0 ,   0 ,   0 },
  { "gray10"          ,  26 ,  26 ,  26 },
  { "gray20"          ,  51 ,  51 ,  51 },
  { "gray30"          ,  77 ,  77 ,  77 },
  { "gray40"          , 102 , 102 , 102 },
  { "gray50"          , 127 , 127 , 127 },
  { "gray60"          , 153 , 153 , 153 },
  { "gray70"          , 179 , 179 , 179 },
  { "gray80"          , 204 , 204 , 204 },
  { "gray90"          , 229 , 229 , 229 },
  { "gray100"         , 255 , 255 , 255 },
};

size_t colours_cnt = sizeof(colours) / sizeof(colours_s);

static colours_s **cols = NULL;

static int
cols_find (const void *a, const void *b)
{
  char *av = (char *)a;
  colours_s *bv = *(colours_s **)b;
  return strcasecmp (av, bv->name);
}

static int
cols_cmp (const void *a, const void *b)
{
  colours_s *av = *(colours_s **)a;
  colours_s *bv = *(colours_s **)b;
  return strcmp (av->name, bv->name);
}

static void
colours_init ()
{
  cols = (colours_s **)malloc (colours_cnt * sizeof(colours_s *));
  int i;
  for (i = 0; i < colours_cnt; i++)
    cols[i] = &colours[i];
  qsort (cols, colours_cnt, sizeof(colours_s *), cols_cmp);
}

colours_s *
colour_lookup (const char *col)
{
  if (!cols) colours_init ();

  void *t = bsearch (col, cols, colours_cnt, sizeof(colours_s *), cols_find);
  if (t)
    return *(colours_s **)t;
  else
    return nullptr;
  
}

#if 0
int
main (int ac, char *av[])
{
  colours_init ();
  void *t = bsearch (av[1], cols, colours_cnt, sizeof(colours_s *), cols_find);
  if (t) {
    colours_s *r = *(colours_s **)t;
    fprintf (stderr, "%d %d %d\n",
	     (int)r->red,
	     (int)r->green,
	     (int)r->blue);
  }
  else
    fprintf (stderr, "%s not found\n", av[1]);
  return 0;
}
#endif
