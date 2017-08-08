#include <gtk/gtk.h>
#include "color_wheel.h"

int main (int argc, char **argv) {
    GtkWidget *window;
    GtkWidget *wheel;
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Color wheel widget");
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    wheel = color_wheel_new ();

    GtkWidget *value_slider;
    GtkAdjustment *value_adjustment;
    value_adjustment = gtk_adjustment_new (1, 0, 1, 0.01, 0.1, 0);
    value_slider = gtk_scale_new(GTK_ORIENTATION_VERTICAL, value_adjustment);
    gtk_widget_set_vexpand (value_slider, TRUE);
    g_signal_connect (value_slider, "value-changed", G_CALLBACK (value_slider_moved), wheel);
    gtk_range_set_inverted(GTK_RANGE(value_slider), TRUE);

    GtkWidget *grid;
    grid = gtk_grid_new ();
    gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);
    gtk_grid_attach (GTK_GRID (grid), wheel, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), value_slider, 1, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER (window), grid);

    //gtk_container_add (GTK_CONTAINER (window), wheel);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_show_all (window);
    gtk_main ();
}