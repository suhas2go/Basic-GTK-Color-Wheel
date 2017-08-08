#include <gtk/gtk.h>

#define COLOR_TYPE_WHEEL (color_wheel_get_type ())
#define COLOR_WHEEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), COLOR_TYPE_WHEEL, ColorWheel))
#define COLOR_WHEEL_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST ((obj), COLOR_WHEEL, ColorWheelClass))
#define COLOR_IS_WHEEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COLOR_TYPE_WHEEL))
#define COLOR_IS_WHEEL_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), COLOR_TYPE_WHEEL))
#define COLOR_WHEEL_GET_CLASS (G_TYPE_INSTANCE_GET_CLASS ((obj), COLOR_TYPE_WHEEL, ColorWheelClass))

typedef struct _ColorWheel ColorWheel;
typedef struct _ColorWheelClass ColorWheelClass;

struct _ColorWheel {
    GtkDrawingArea parent;

    /* private */
    gpointer priv;

};

struct _ColorWheelClass {
    GtkDrawingAreaClass parent_class;
};

GtkWidget *color_wheel_new (void);
void value_slider_moved (GtkRange *range, gpointer  user_data);
