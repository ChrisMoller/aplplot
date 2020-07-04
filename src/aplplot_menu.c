#include <gtk/gtk.h>
#include <math.h>

#include "aplplot_menu.h"

aplplot_set_value_t aplplot_set_value = NULL;

static gint window_width  = DEFAULT_PLOT_WIDTH;
static gint window_height = DEFAULT_PLOT_HEIGHT;
static GtkWidget *window;
static GtkApplication *app;

static GtkWidget *cart;
static GtkWidget *polar;
static GtkWidget *deg;
static GtkWidget *rad;
static GtkWidget *pirad;
static GtkWidget *xlabel;
static GtkWidget *ylabel;
static GtkWidget *tlabel;
static GtkWidget *file_name;
static GtkWidget *colour;
static GtkWidget *width;
static GtkWidget *height;
static GtkWidget *x_col;
static GtkWidget *file_name;
static GtkWidget *file_button;
static GtkWidget *xlog;
static GtkWidget *ylog;
static GtkWidget *lines;
static GtkWidget *points;

static gboolean fn_valid = FALSE;
static char *filename = NULL;
static int angle_mode = APL_ANGLE_RADIANS;

static void
file_clicked_cb (GtkButton *button,
		 gpointer   user_data)
{
  GtkWidget *dialog =
    gtk_file_chooser_dialog_new ("File name",
				 GTK_WINDOW (window),
				 GTK_FILE_CHOOSER_ACTION_SAVE,
				 "Cancel",
				 GTK_RESPONSE_CANCEL,
				 "Select",
				 GTK_RESPONSE_ACCEPT,
				 NULL);
  gint res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    if (filename) g_free (filename);
    filename = gtk_file_chooser_get_filename (chooser);
    if (filename && *filename) fn_valid = TRUE;
    gtk_label_set_text (GTK_LABEL (file_name), filename);
    g_free (filename);
  }

  gtk_widget_destroy (dialog);
}

static void
hitit_clicked_cb (GtkButton *button,
		  gpointer   user_data)
{
  static char *xlbl = NULL;
  static char *ylbl = NULL;
  static char *tlbl = NULL;
  static char *fn = NULL;
  value_u val;

  val.type = VALUE_WIDTH;
  val.val.i = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (width));
  aplplot_set_value (val);
  
  val.type = VALUE_HEIGHT;
  val.val.i = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (height));
  aplplot_set_value (val);

  if (xlbl) g_free (xlbl);
  val.type = VALUE_X_LABEL;
  val.val.s = xlbl = g_strdup (gtk_entry_get_text (GTK_ENTRY (xlabel)));
  aplplot_set_value (val);

  if (ylbl) g_free (ylbl);
  val.type = VALUE_Y_LABEL;
  val.val.s = ylbl = g_strdup (gtk_entry_get_text (GTK_ENTRY (ylabel)));
  aplplot_set_value (val);

  if (tlbl) g_free (tlbl);
  val.type = VALUE_T_LABEL;
  val.val.s = tlbl = g_strdup (gtk_entry_get_text (GTK_ENTRY (tlabel)));
  aplplot_set_value (val);
  
  val.type = VALUE_X_LOG;
  val.val.b =  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (xlog));
  aplplot_set_value (val);
  
  val.type = VALUE_Y_LOG;
  val.val.b =  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ylog));
  aplplot_set_value (val);
  
  val.type = VALUE_COORDS;
  int pp =  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (polar));
  val.val.i =  pp ? APL_MODE_POLAR : APL_MODE_XY;
  aplplot_set_value (val);
  
  val.type = VALUE_ANGLES;
  val.val.i = angle_mode;
  aplplot_set_value (val);
  
  val.type = VALUE_X_COL;
  val.val.i = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (x_col));
  aplplot_set_value (val);
  
  val.type = VALUE_DRAW;
  gboolean la = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lines));
  gboolean pa = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (points));
  gint am = 0;
  am |= la ? APL_DRAW_LINES : 0;
  am |= pa ? APL_DRAW_POINTS : 0;
  val.val.i =  am;
  aplplot_set_value (val);

  if (fn_valid) {
    if (fn) g_free (fn);
    val.type = VALUE_FILE_NAME;
    val.val.s = fn = g_strdup (gtk_label_get_text (GTK_LABEL (file_name)));
    aplplot_set_value (val);
  }
  
#if 0
  value_u valb = { VALUE_EMBED, .val.b = 1};
  aplplot_set_value (valb);
  value_u vald = { VALUE_DEST, .val.d = DEST_SVG};
  aplplot_set_value (vald);
  value_u valc = { VALUE_ANGLES, .val.c = COORDS_RADIANS};
  aplplot_set_value (valc);
#endif
#if 0
  exit (0);
  gtk_main_quit ();
  g_application_release (G_APPLICATION (app));
  gtk_application_remove_window (GTK_APPLICATION (app),
				 GTK_WINDOW (window));
  g_application_quit (G_APPLICATION (app));
#endif
}

static void
embed_toggled_cb (GtkButton *button,
		  gpointer   user_data)
{
  gboolean active =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gtk_widget_set_sensitive (file_name,	 !active);
  gtk_widget_set_sensitive (file_button, !active);
}

static void
angle_clicked_cb (GtkButton *button,
		  gpointer   user_data)
{
  angle_mode = GPOINTER_TO_INT (user_data);
}

static void
coords_clicked_cb (GtkButton *button,
		  gpointer   user_data)
{
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
  g_signal_connect (G_OBJECT (window), "destroy",
                    G_CALLBACK (hitit_clicked_cb), NULL);
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

      GtkWidget *xlinear =
	gtk_radio_button_new_with_label (NULL, "X Linear");

      xlog =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (xlinear),
						     "X Logarithmic");

      GtkWidget *ylinear =
	gtk_radio_button_new_with_label (NULL, "Y Linear");

      ylog =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (ylinear),
						     "Y Logarithmic");

      gtk_box_pack_start (GTK_BOX (vbox), xlinear, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), xlog, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), ylinear, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), ylog, FALSE, FALSE, 4);
    }

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);

      lines  = gtk_check_button_new_with_label ("Lines");
      points = gtk_check_button_new_with_label ("Points");

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lines), TRUE);
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

      g_signal_connect (cart, "clicked", G_CALLBACK (coords_clicked_cb), NULL);
      g_signal_connect (polar, "clicked", G_CALLBACK (coords_clicked_cb), NULL);
      gtk_box_pack_start (GTK_BOX (vbox), cart, FALSE, FALSE, 4);
      gtk_box_pack_start (GTK_BOX (vbox), polar, FALSE, FALSE, 4);
    }

    {
      GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 4);
    
      deg = gtk_radio_button_new_with_label (NULL, "Degrees");
      gtk_widget_set_sensitive (deg, FALSE);
      g_signal_connect (deg, "clicked", G_CALLBACK (angle_clicked_cb),
			GINT_TO_POINTER (APL_ANGLE_DEGREES));

      rad =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (deg),
						     "Radians");
      gtk_widget_set_sensitive (rad, FALSE);
      g_signal_connect (rad, "clicked", G_CALLBACK (angle_clicked_cb),
			GINT_TO_POINTER (APL_ANGLE_RADIANS));

      pirad =
	gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (deg),
						     "Pi Radians");
      gtk_widget_set_sensitive (pirad, FALSE);
      g_signal_connect (pirad, "clicked", G_CALLBACK (angle_clicked_cb),
			GINT_TO_POINTER (APL_ANGLE_PI_RADIANS));

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rad), TRUE);

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
    
      file_button = gtk_button_new_with_label ("Set file name");
      g_signal_connect (file_button, "clicked",
			G_CALLBACK (file_clicked_cb), NULL);
      gtk_box_pack_start (GTK_BOX (hbox), file_button, FALSE, FALSE, 4);
      
      file_name = gtk_label_new ("(unset)");
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
				 (gdouble)window_width);

      GtkWidget *height_label = gtk_label_new ("Height");
      gtk_grid_attach (GTK_GRID (vbox_dims), height_label, 0, 1, 1, 1);
      
      height = gtk_spin_button_new_with_range (100.0, 10000.0, 1.0);
      gtk_grid_attach (GTK_GRID (vbox_dims), height, 1, 1, 1, 1);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (height),
				 (gdouble)window_height);

      GtkWidget *colour_label = gtk_label_new ("Background colour");
      gtk_grid_attach (GTK_GRID (vbox_dims), colour_label, 0, 2, 1, 1);
      
      colour = gtk_color_button_new ();
      gtk_grid_attach (GTK_GRID (vbox_dims), colour, 1, 2, 1, 1);
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
      x_col = gtk_spin_button_new_with_range (00, 1000.0, 1.0);
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
  g_signal_connect (hitit, "clicked", G_CALLBACK (hitit_clicked_cb), NULL);
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

void
aplplot_menu (void *fcn)
{
  aplplot_set_value = (aplplot_set_value_t)fcn;

  app = gtk_application_new ("org.gtk.example",
                             G_APPLICATION_HANDLES_COMMAND_LINE);
  g_signal_connect (app, "command-line",
                    G_CALLBACK (command_line_cb), NULL);
  g_signal_connect (app, "activate",
                    G_CALLBACK (activate), NULL);
  g_application_run (G_APPLICATION (app), 0, NULL);

  g_object_unref (app);
}
