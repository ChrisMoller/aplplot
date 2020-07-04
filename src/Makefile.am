
lib_LTLIBRARIES = libaplplot.la libaplplot_menu.la


libaplplot_la_SOURCES = aplplot.cc aplplot.hh
libaplplot_menu_la_SOURCES = aplplot_menu.c

BUILT_SOURCES = modes.h

#EXTRA_libaplplot_la_DEPENDENCIES = modes.h
#EXTRA_libaplplot_menu_la_DEPENDENCIES = modes.h


libaplplot_la_CPPFLAGS = -I$(APL_SOURCES) $(PLPLOT_CFLAGS) \
                          $(GTK_CFLAGS) $(CAIRO_CFLAGS)
libaplplot_menu_la_CFLAGS = $(GTK_CFLAGS) $(CAIRO_CFLAGS)

libaplplot_la_LIBADD = $(PLPLOT_LIBS)  $(GTK_CFLAGS) $(CAIRO_CFLAGS) \
                      -L/usr/local/lib/apl -laplplot_menu
libaplplot_la_LDFLAGS = -Wl,-R/usr/local/lib/apl

libaplplot_menu_la_LDFLAGS = $(GTK_LIBS) $(CAIRO_LIBS)

modes.h : modes.m4
	m4 < $< > $@

#lib_LTLIBRARIES = libaplplot.la
#noinst_LTLIBRARIES =

#libaplplot_la_SOURCES = aplplot.cc aplplot.hha \
#                        aplplot_menu.c aplplot_menu.h
#libaplplot_menu_la_SOURCES = aplplot_menu.c aplplot_menu.h

#libaplplot_la_CPPFLAGS = -I$(APL_SOURCES) $(PLPLOT_CFLAGS) \
#                          $(GTK_CFLAGS) $(CAIRO_CFLAGS)
#libaplplot_menu_la_CFLAGS = $(GTK_CFLAGS) $(CAIRO_CFLAGS)

#libaplplot_la_LIBADD = $(PLPLOT_LIBS)  $(GTK_CFLAGS) $(CAIRO_CFLAGS)

#libaplplot_la_LIBADD = $(PLPLOT_LIBS) -L/usr/local/lib/apl libaplplot_menu.a
#libaplplot_la_LDFLAGS = -Wl,-Bstatic

#libaplplot_la_LIBADD = $(PLPLOT_LIBS)
#libaplplot_la_LDFLAGS = -L/usr/local/lib/apl -laplplot_menu -Wl,-R/usr/local/lib/apl
#libaplplot_la_LDFLAGS = -L/usr/local/lib/apl -laplplot_menu -Wl,-Bstatic
#libaplplot_menu_la_LDFLAGS = $(GTK_LIBS) $(CAIRO_LIBS)

