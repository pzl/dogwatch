#include <cairo.h>
#include <gtk/gtk.h>
#include "gui.h"

static void do_drawing(cairo_t *);
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);


void gui_init(void){
	GtkWidget *window;
	GtkWidget *darea;

	//todo: gtk_init_check for text-based fallback
	gtk_init(NULL,NULL);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window),darea);

	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
	gtk_window_set_title(GTK_WINDOW(window), "Dogwatch");

	gtk_widget_show_all(window);

	gtk_main();
}



static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data){
	do_drawing(cr);
	return FALSE;
}

static void do_drawing(cairo_t *cr){
	cairo_set_source_rgb(cr,0,0,0);
	cairo_set_line_width(cr,0.5);

	cairo_move_to(cr,200,200);
	cairo_line_to(cr,100,100);

	cairo_stroke(cr);
}
