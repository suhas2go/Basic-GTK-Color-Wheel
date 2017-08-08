#include <gtk/gtk.h>
#include <math.h>
#include "color_wheel.h"

/* Default ring fraction */
#define DEFAULT_FRACTION 0.1

/* Default width/height */
#define DEFAULT_SIZE 200

/* Default ring width */
#define DEFAULT_RING_WIDTH 100

#define parent_class color_wheel_parent_class

typedef struct
{
    /* Color value */
    gdouble h;
    gdouble s;
    gdouble v;

    /* ring_width is this fraction of size */
    gdouble ring_fraction;

    /* Size and ring width */
    gint size;
    gint ring_width;

    /* Window for capturing events */
    GdkWindow *window;

    /* Dragging mode */
    //DragMode mode;

    guint focus_on_ring : 1;

} ColorWheelPrivate;


/* Utility functions */

#define INTENSITY(r, g, b) ((r) * 0.30 + (g) * 0.59 + (b) * 0.11)

static gdouble rgb_to_intensity(gdouble r, gdouble g, gdouble b)
{
    gdouble intensity;
    intensity= (r) * 0.30 + (g) * 0.59 + (b) * 0.11;
}

/* Converts from HSV to RGB */
static void
hsv_to_rgb (gdouble *h,
            gdouble *s,
            gdouble *v)
{
    gdouble hue, saturation, value;
    gdouble f, p, q, t;

    if (*s == 0.0)
    {
        *h = *v;
        *s = *v;
        /* *v = *v; -- heh */
    }
    else
    {
        hue = *h * 6.0;
        saturation = *s;
        value = *v;

        if (hue == 6.0)
            hue = 0.0;

        f = hue - (int) hue;
        p = value * (1.0 - saturation);
        q = value * (1.0 - saturation * f);
        t = value * (1.0 - saturation * (1.0 - f));

        switch ((int) hue)
        {
            case 0:
                *h = value;
                *s = t;
                *v = p;
                break;

            case 1:
                *h = q;
                *s = value;
                *v = p;
                break;

            case 2:
                *h = p;
                *s = value;
                *v = t;
                break;

            case 3:
                *h = p;
                *s = q;
                *v = value;
                break;

            case 4:
                *h = t;
                *s = p;
                *v = value;
                break;

            case 5:
                *h = value;
                *s = p;
                *v = q;
                break;

            default:
                g_assert_not_reached ();
        }
    }
}

G_DEFINE_TYPE (ColorWheel, color_wheel, GTK_TYPE_DRAWING_AREA);


static void color_wheel_init (ColorWheel * wheel) {

    ColorWheelPrivate *priv;

    priv = G_TYPE_INSTANCE_GET_PRIVATE (wheel, COLOR_TYPE_WHEEL, ColorWheelPrivate);

    wheel->priv = priv;

    gtk_widget_add_events (GTK_WIDGET (wheel),
                           GDK_BUTTON_PRESS_MASK        |
                           GDK_KEY_PRESS_MASK           |
                           GDK_POINTER_MOTION_MASK      |
                           GDK_POINTER_MOTION_HINT_MASK |
                           GDK_PROXIMITY_OUT_MASK);

    gtk_widget_set_has_window (GTK_WIDGET (wheel), TRUE);
    gtk_widget_set_can_focus (GTK_WIDGET (wheel), TRUE);

    priv->ring_fraction = DEFAULT_FRACTION;
    priv->size          = DEFAULT_SIZE;
    priv->ring_width    = DEFAULT_RING_WIDTH;
    priv->v             = 1;


}

static void
color_wheel_map_cb (GtkWidget *widget)
{
    ColorWheel        *wheel = COLOR_WHEEL (widget);
    ColorWheelPrivate *priv  = wheel->priv;

    GTK_WIDGET_CLASS (parent_class)->map (widget);

    gdk_window_show (priv->window);
}

static void
color_wheel_unmap_cb (GtkWidget *widget)
{
    ColorWheel        *wheel = COLOR_WHEEL (widget);
    ColorWheelPrivate *priv  = wheel->priv;

    gdk_window_hide (priv->window);

    GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}


GtkWidget * color_wheel_new (void) {
    return g_object_new (COLOR_TYPE_WHEEL, NULL);
}

void value_slider_moved (GtkRange *range, gpointer  user_data)
{
    GtkWidget *widget = user_data;
    ColorWheel *wheel = COLOR_WHEEL (widget);
    ColorWheelPrivate *priv  = wheel->priv;
    gdouble value = gtk_range_get_value (range);
    priv->v = value;
    gtk_widget_queue_draw (widget);
}

static double
compute_hue (ColorWheel *wheel, gdouble x, gdouble y)
{
    GtkAllocation allocation;
    gdouble       center_x;
    gdouble       center_y;
    gdouble       dx, dy;
    gdouble       angle;

    gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

    center_x = allocation.width / 2.0;
    center_y = allocation.height / 2.0;

    dx = x - center_x;
    dy = center_y - y;

    angle = atan2 (dy, dx);
    if (angle < 0.0)
        angle += 2.0 * G_PI;

    return angle / (2.0 * G_PI);
}

static double
compute_sat (ColorWheel *wheel, gdouble x, gdouble y)
{
    GtkAllocation allocation;
    gdouble center_x;
    gdouble center_y;
    gdouble dx, dy;
    gdouble sat;
    gdouble a;
    ColorWheelPrivate *priv   = wheel->priv;
    gdouble radius = priv->size / 2.0;

    gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

    center_x = allocation.width / 2.0;
    center_y = allocation.height / 2.0;

    dx = x - center_x;
    dy = center_y - y;
    a = sqrt(dx*dx + dy*dy);
    sat = a/radius;

    return sat;
}

/* Computes whether a point is inside the hue ring */
static gboolean
is_in_ring (ColorWheel *wheel, gdouble x, gdouble y)
{
    ColorWheelPrivate *priv = wheel->priv;
    GtkAllocation          allocation;
    gdouble                dx, dy, dist;
    gdouble                center_x;
    gdouble                center_y;
    gdouble                radius;

    gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

    center_x = allocation.width / 2.0;
    center_y = allocation.height / 2.0;

    radius = priv->size / 2.0;

    dx = x - center_x;
    dy = center_y - y;
    dist = dx * dx + dy * dy;

    return (dist <= radius * radius);
}

static gboolean
color_wheel_button_press_cb (GtkWidget      *widget,
                               GdkEventButton *event) {
    g_print("button press\n");
    ColorWheel *wheel = COLOR_WHEEL(widget);
    ColorWheelPrivate *priv = wheel->priv;
    gdouble x, y;

    x = event->x;
    y = event->y;
    if(is_in_ring(wheel, x, y)) {

        priv->h = compute_hue(wheel, x, y);
        priv->s = compute_sat(wheel, x, y);
        g_print("%f %f %f\n", priv->h, priv->s, priv->v);
        gdouble r,g,b;
        r=priv->h;g=priv->s;b=priv->v;
        hsv_to_rgb (&r, &g, &b);
        g_print("%d %f %f\n",(int)floor(r*255), g*255, b*255);
        gtk_widget_queue_draw (widget);
    }
}

static gboolean
color_wheel_button_release_cb (GtkWidget      *widget,
                                 GdkEventButton *event) {
    g_print ("burron release");
}

static gboolean
color_wheel_motion_cb (GtkWidget      *widget,
                                 GdkEventMotion *event) {
    g_print ("motion release");
}

static gboolean keypress_my_handler_callback(GtkWidget * widget, GdkEventKey *event)
{   gtk_widget_grab_focus (widget);
    g_print ("my handler");
}

static gboolean keyrelease_my_handler_callback(GtkWidget * widget, GdkEventKey *event)
{   gtk_widget_grab_focus (widget);
    g_print ("my handler");
}



/* Paints the hue ring */
static void
paint_ring (ColorWheel *wheel, cairo_t *cr, gdouble value)
{
    GtkWidget *widget = GTK_WIDGET (wheel);
    ColorWheelPrivate *priv   = wheel->priv;
    gint width, height;
    gint xx, yy;
    gdouble dx, dy, dist;
    gdouble center_x;
    gdouble center_y;
    gdouble inner, outer;
    guint32 *buf, *p;
    gdouble angle;
    gdouble hue;
    gdouble r, g, b;
    cairo_surface_t *source;
    cairo_t *source_cr;
    gint stride;

    width  = gtk_widget_get_allocated_width  (widget);
    height = gtk_widget_get_allocated_height (widget);

    center_x = width  / 2.0;
    center_y = height / 2.0;

    outer = priv->size / 2.0;
    inner = outer - priv->ring_width;

    /* Create an image initialized with the ring colors */

    stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width);
    buf = g_new (guint32, height * stride / 4);

    for (yy = 0; yy < height; yy++)
    {
        p = buf + yy * width;
        dy = -(yy - center_y);

        for (xx = 0; xx < width; xx++)
        {
            dx = xx - center_x;

            dist = dx * dx + dy * dy;
            if (dist < ((inner-1) * (inner-1)) || dist > ((outer+1) * (outer+1)))
            {
                *p++ = 0;
                continue;
            }

            angle = atan2 (dy, dx);
            if (angle < 0.0)
                angle += 2.0 * G_PI;

            hue = angle / (2.0 * G_PI);

            r = hue;
            g = 1.0;
            b = value;
            hsv_to_rgb (&r, &g, &b);

            *p++ = (((int)floor (r * 255 + 0.5) << 16) |
                    ((int)floor (g * 255 + 0.5) << 8) |
                    (int)floor (b * 255 + 0.5));
        }
    }

    source = cairo_image_surface_create_for_data ((unsigned char *)buf,
                                                  CAIRO_FORMAT_RGB24,
                                                  width, height, stride);

    /* Now draw the value marker onto the source image, so that it
     * will get properly clipped at the edges of the ring
     */

    source_cr = cairo_create (source);
    gdouble red, green, blue;
    red = priv->h; green = priv->s; blue = priv->v;
    hsv_to_rgb(&red, &green, &blue);
    if (rgb_to_intensity (red, green, blue) > 0.5)
        cairo_set_source_rgb (source_cr, 0.0, 0.0, 0.0);
    else
        cairo_set_source_rgb (source_cr, 1.0, 1.0, 1.0);

    gdouble selection_center_x, selection_center_y;
    selection_center_x = center_x + cos (priv->h * 2.0 * G_PI) * priv->s * priv->size / 2;
    selection_center_y = center_y - sin (priv->h * 2.0 * G_PI) * priv->s * priv->size / 2;

    cairo_arc(source_cr, selection_center_x, selection_center_y, priv->size/50, 0, 2 * G_PI);


    cairo_stroke(source_cr);

/*
     // Past Suhas: Future Suhas, this prints a line sort of thing.

    cairo_move_to (source_cr, center_x, center_y);
    cairo_line_to (source_cr,
                   center_x + cos (priv->h * 2.0 * G_PI) * priv->size / 2,
                   center_y - sin (priv->h * 2.0 * G_PI) * priv->size / 2);
    cairo_stroke (source_cr);

    */

    cairo_destroy (source_cr);


    /* Draw the ring using the source image */

    cairo_save (cr);

    cairo_set_source_surface (cr, source, 0, 0);
    cairo_surface_destroy (source);

    cairo_set_line_width (cr, priv->ring_width);
    cairo_new_path (cr);
    cairo_arc (cr,
               center_x, center_y,
               priv->size / 2.0 - priv->ring_width / 2.0,
               0, 2 * G_PI);
    cairo_stroke (cr);

    cairo_restore (cr);



    g_free (buf);
}

gboolean
color_wheel_draw_cb (GtkWidget *widget, cairo_t *cr)
{
    ColorWheel *wheel = COLOR_WHEEL (widget);
    ColorWheelPrivate *priv  = wheel->priv;
    gboolean draw_focus;

    draw_focus = gtk_widget_has_visible_focus (widget);

    paint_ring (wheel, cr, priv->v);

    if (draw_focus && priv->focus_on_ring)
    {
        GtkStyleContext *context = gtk_widget_get_style_context (widget);

        gtk_render_focus (context, cr, 0, 0,
                          gtk_widget_get_allocated_width (widget),
                          gtk_widget_get_allocated_height (widget));
    }

    return FALSE;
}


static void color_wheel_class_init (ColorWheelClass *klass) {

    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class= GTK_WIDGET_CLASS (klass);

    widget_class->map = color_wheel_map_cb;
    widget_class->unmap = color_wheel_unmap_cb;
    widget_class->button_press_event   = color_wheel_button_press_cb;
    widget_class->button_release_event = color_wheel_button_release_cb;
    widget_class->motion_notify_event  = color_wheel_motion_cb;
    widget_class->key_press_event = keypress_my_handler_callback; // works
    widget_class->key_release_event = keyrelease_my_handler_callback; // works

    widget_class->draw = color_wheel_draw_cb;
    g_type_class_add_private (object_class, sizeof (ColorWheelPrivate));
}
