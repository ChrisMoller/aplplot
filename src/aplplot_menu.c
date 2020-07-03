#include <gtk/gtk.h>
#include <math.h>

#include "aplplot_menu.h"

#define INITIAL_WIDTH  640
#define INITIAL_HEIGHT 480

static gint window_width  = INITIAL_WIDTH;
static gint window_height = INITIAL_HEIGHT;
static GtkWidget *window;

static GtkWidget *cart;
static GtkWidget *polar;
static GtkWidget *deg;
static GtkWidget *rad;
static GtkWidget *pirad;
static GtkWidget *screen;
static GtkWidget *png;
static GtkWidget *pdf;
static GtkWidget *ps;
static GtkWidget *eps;
static GtkWidget *svg;
static GtkWidget *xlabel;
static GtkWidget *ylabel;
static GtkWidget *tlabel;
static GtkWidget *file_name;
static GtkWidget *colour;
static GtkWidget *width = NULL;
static GtkWidget *height = NULL;

enum {MODE_CART, MODE_POLAR};

double
aplplot_menu_get_double (int which)
{
  double rv = NAN;
  switch(which) {
  case VALUE_WIDTH:
    fprintf (stderr, "width = %p\n", width);
    rv = gtk_spin_button_get_value (GTK_SPIN_BUTTON (width));
    break;
  case VALUE_HEIGHT:
    rv = gtk_spin_button_get_value (GTK_SPIN_BUTTON (height));
    break;
  case VALUE_X_MIN:
    break;
  case VALUE_X_MAX:
    break;
  }
  return rv;
}

char *
aplplot_menu_get_string (int which)
{
  gchar *rv = NULL;
  switch(which) {
  case VALUE_X_LABEL:
    rv = (gchar *)gtk_entry_get_text (GTK_ENTRY (xlabel));
    break;
  case VALUE_Y_LABEL:
    rv =  (gchar *)gtk_entry_get_text (GTK_ENTRY (ylabel));
    break;
  case VALUE_T_LABEL:
    rv =  (gchar *)gtk_entry_get_text (GTK_ENTRY (tlabel));
    break;
  case VALUE_FILE_NAME:
    rv =  (gchar *)gtk_entry_get_text (GTK_ENTRY (file_name));
    break;
  case VALUE_COLOUR:
    {
      GdkRGBA colour_val;
      static gchar *colour_gchar = NULL;
      gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (colour),
				  &colour_val);
      if (colour_gchar) g_free (colour_gchar);
      rv = colour_gchar = gdk_rgba_to_string (&colour_val);
    }
    break;
  }
  return rv;
}

static void
hitit_clicked_cb (GtkButton *button,
		  gpointer   user_data)
{
  g_print ("hit it\n");
}

static void
embed_toggled_cb (GtkButton *button,
		  gpointer   user_data)
{
  gboolean active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gtk_widget_set_sensitive (screen,	!active);
  gtk_widget_set_sensitive (png,	!active);
  gtk_widget_set_sensitive (pdf,	!active);
  gtk_widget_set_sensitive (ps,		!active);
  gtk_widget_set_sensitive (eps,	!active);
  gtk_widget_set_sensitive (svg,	!active);
}

static void
angle_clicked_cb (GtkButton *button,
		  gpointer   user_data)
{
  int mode = GPOINTER_TO_INT (user_data);
  gboolean active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (polar));
  gtk_widget_set_sensitive (deg,   active);
  gtk_widget_set_sensitive (rad,   active);
  gtk_widget_set_sensitive (pirad, active);
}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Aplplot");
  GtkWidget *frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (window), frame);

  GtkWidget *outer_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add (GTK_CONTAINER (frame), outer_vbox);

  {	// top hbox
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start (GTK_BOX (outer_vbox), hbox, FALSE, FALSE, 4);

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);

      GtkWidget *linear =
	gtk_radio_button_new_with_label (NULL, "Linear");

      GtkWidget *log =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (linear),
						     "Logarithmic");

      gtk_box_pack_start (GTK_BOX (vbox), linear, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), log, FALSE, FALSE, 4);
    }

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);

      GtkWidget *lines  = gtk_check_button_new_with_label ("Lines");
      GtkWidget *points = gtk_check_button_new_with_label ("Points");

      gtk_box_pack_start (GTK_BOX (vbox), lines, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), points, FALSE, FALSE, 4);
    }

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);
    
      cart =
	gtk_radio_button_new_with_label (NULL, "Cartesian");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cart), TRUE);

      polar =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (cart),
						     "Polar");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (polar), FALSE);

      g_signal_connect (cart, "clicked",
			G_CALLBACK (angle_clicked_cb),
			GINT_TO_POINTER (MODE_CART));
      g_signal_connect (polar, "clicked",
			G_CALLBACK (angle_clicked_cb),
			GINT_TO_POINTER (MODE_POLAR));
      gtk_box_pack_start (GTK_BOX (vbox), cart, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), polar, FALSE, FALSE, 4);
    }

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);
    
      deg = gtk_radio_button_new_with_label (NULL, "Degrees");
      gtk_widget_set_sensitive (deg, FALSE);

      rad =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (deg),
						     "Radians");
      gtk_widget_set_sensitive (rad, FALSE);

      pirad =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (deg),
						     "Pi Radians");
      gtk_widget_set_sensitive (pirad, FALSE);

      
      gtk_box_pack_start (GTK_BOX (vbox), deg, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), rad, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), pirad, FALSE, FALSE, 4);
    }
  }

  {	// second hbox
    GtkWidget *frame = gtk_frame_new ("Output");
    gtk_container_add (GTK_CONTAINER (outer_vbox), frame);

    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    {
      GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);

      GtkWidget *embed  = gtk_check_button_new_with_label ("Embed");
      gtk_box_pack_start (GTK_BOX (hbox), embed, FALSE, FALSE, 4);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (embed), FALSE);
      g_signal_connect (embed, "toggled", G_CALLBACK (embed_toggled_cb), NULL);
      
      colour = gtk_color_button_new ();
      gtk_box_pack_end (GTK_BOX (hbox), colour, FALSE, FALSE, 4);

      GtkWidget *colour_label = gtk_label_new ("Background colour");
      gtk_box_pack_end (GTK_BOX (hbox), colour_label, FALSE, FALSE, 4);
    }

    {
      GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);

      screen =
	gtk_radio_button_new_with_label (NULL, "Screen");
      png =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (screen),
						     "PNG");
      pdf =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (screen),
						     "Pdf");
      ps =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (screen),
						     "PS");
      eps =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (screen),
						     "EPS");
      svg =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (screen),
						   "SVG");
      gtk_box_pack_start (GTK_BOX (hbox), screen, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (hbox), png, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (hbox), pdf, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (hbox), ps, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (hbox), eps, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (hbox), svg, FALSE, FALSE, 4);
    }

    {
      GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);

      GtkWidget *file_label = gtk_label_new ("File name");
      gtk_box_pack_start (GTK_BOX (hbox), file_label, FALSE, FALSE, 4);

      GtkWidget *file_name;
      file_name = gtk_entry_new ();
      gtk_entry_set_placeholder_text (GTK_ENTRY (file_name),
				      "Output file name");
      gtk_box_pack_start (GTK_BOX (hbox), file_name, TRUE, TRUE, 4);
    } 
    

  }

  {	// third hbox
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_add (GTK_CONTAINER (outer_vbox), hbox);

    {
      GtkWidget *vbox_dims = gtk_grid_new ();
      gtk_box_pack_start (GTK_BOX (hbox), vbox_dims, FALSE, FALSE, 4);

      GtkWidget *width_label = gtk_label_new ("Width");
      gtk_grid_attach (GTK_GRID (vbox_dims), width_label, 0, 0, 1, 1);
      
      width = gtk_spin_button_new_with_range (100.0, 10000.0, 1.0);
      gtk_grid_attach (GTK_GRID (vbox_dims), width, 1, 0, 1, 1);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (width),
				 (gdouble)INITIAL_WIDTH);

      GtkWidget *height_label = gtk_label_new ("Height");
      gtk_grid_attach (GTK_GRID (vbox_dims), height_label, 0, 1, 1, 1);
      
      height = gtk_spin_button_new_with_range (100.0, 10000.0, 1.0);
      gtk_grid_attach (GTK_GRID (vbox_dims), height, 1, 1, 1, 1);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (height),
				 (gdouble)INITIAL_HEIGHT);
    }
    {
      GtkWidget *range_dims = gtk_grid_new ();
      gtk_box_pack_start (GTK_BOX (hbox), range_dims, FALSE, FALSE, 4);

      GtkWidget *min_label = gtk_label_new ("Min X");
      gtk_grid_attach (GTK_GRID (range_dims), min_label, 0, 0, 1, 1);

      GtkAdjustment *min_x_adj = gtk_adjustment_new (0.0,
						     -G_MAXDOUBLE,
						     G_MAXDOUBLE,
						     1.0,
						     100.0,
						     100.0);
      GtkWidget *min_x = gtk_spin_button_new (min_x_adj, 1.0, 8);
      gtk_grid_attach (GTK_GRID (range_dims), min_x, 1, 0, 1, 1);

      GtkWidget *max_label = gtk_label_new ("Max X");
      gtk_grid_attach (GTK_GRID (range_dims), max_label, 0, 1, 1, 1);
      
      GtkAdjustment *max_x_adj = gtk_adjustment_new (0.0,
						     -G_MAXDOUBLE,
						     G_MAXDOUBLE,
						     1.0,
						     100.0,
						     100.0);
      GtkWidget *max_x = gtk_spin_button_new (max_x_adj, 1.0, 8);
      gtk_grid_attach (GTK_GRID (range_dims), max_x, 1, 1, 1, 1);

      GtkWidget *x_col_lbl = gtk_label_new ("X Column");
      gtk_grid_attach (GTK_GRID (range_dims), x_col_lbl, 0, 2, 1, 1);
      GtkWidget *x_col =
	gtk_spin_button_new_with_range (00, 1000.0, 1.0);
      gtk_grid_attach (GTK_GRID (range_dims), x_col, 1, 2, 1, 1);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (x_col), 0.0);
    }
    
  }

  {	// fourth hbox
    GtkWidget *labels = gtk_grid_new ();
    gtk_box_pack_start (GTK_BOX (outer_vbox), labels, FALSE, FALSE, 4);

    GtkWidget *xlabel_lbl = gtk_label_new ("X Label");
    GtkWidget *ylabel_lbl = gtk_label_new ("Y Label");
    GtkWidget *tlabel_lbl = gtk_label_new ("Top Label");
    gtk_grid_attach (GTK_GRID (labels), xlabel_lbl, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (labels), ylabel_lbl, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (labels), tlabel_lbl, 0, 2, 1, 1);
    
    xlabel = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (xlabel), "X label");
    ylabel = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (ylabel), "Y label");
    tlabel = gtk_entry_new ();
    gtk_entry_set_placeholder_text (GTK_ENTRY (tlabel), "Top label");
    gtk_grid_attach (GTK_GRID (labels), xlabel, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (labels), ylabel, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (labels), tlabel, 1, 2, 1, 1);
  }

  GtkWidget *hitit = gtk_button_new_with_label ("Go");
  g_signal_connect (cart, "clicked", G_CALLBACK (hitit_clicked_cb), NULL);
  gtk_box_pack_start (GTK_BOX (outer_vbox), hitit, FALSE, FALSE, 4);
  
  gtk_widget_show_all (window);
}

static gint
command_line_cb (GApplication            *application,
                 GApplicationCommandLine *command_line,
                 gpointer                 user_data)
{
  GError *error = NULL;
  gint ac;

  gchar **av = g_application_command_line_get_arguments (command_line, &ac);

  GOptionEntry entries[] =
    {
     { NULL }
    };

  GOptionContext *context = g_option_context_new ("file1 file2...");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

  if (!g_option_context_parse (context, &ac, &av, &error)) {
    g_warning ("option parsing failed: %s\n", error->message);
    g_clear_error (&error);
  }

#if 0
   g_print ("test = %d\n", test);
   for (int i = 0; i < ac; i++) {
     g_print ("arg %d = %s\n", i, av[i]);
   }
#endif

#if 1
   if (ac > 0) {
     //     fn = g_strdup (av[1]);
   }
#else
   else {
     g_print ("No image specified.\n");
     exit (1);
   }
#endif

  g_application_activate (application);

  return 1;
}

static gchar **apl_argv      = NULL;
static int     apl_argv_next = 0;
static int     apl_argv_max  = 0;
#define APL_ARGV_INCR   16

static void
append_argv (gchar *av)
{
  if (apl_argv_max <= apl_argv_next) {
    apl_argv_max += APL_ARGV_INCR;
    apl_argv = g_realloc (apl_argv, apl_argv_max * sizeof(gchar *));
  }
  apl_argv[apl_argv_next++] = av;
}

void (*talkback)() = NULL;

void
aplplot_menu (void *fcn)
{
  GtkApplication *app;
  talkback = (void (*)())fcn;
  (*talkback)();

  append_argv ("aplplot");
  append_argv (NULL);

  app = gtk_application_new ("org.gtk.example",
                             G_APPLICATION_HANDLES_COMMAND_LINE);
  g_signal_connect (app, "command-line",
                    G_CALLBACK (command_line_cb), NULL);
  g_signal_connect (app, "activate",
                    G_CALLBACK (activate), NULL);
  g_application_run (G_APPLICATION (app), apl_argv_next, apl_argv);
  //  g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
}
