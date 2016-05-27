#include "gtk3_widget.h"

#include <GL/glx.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib-object.h>

#include <stdbool.h>

typedef struct {
	GdkWindow *win;
	GLXContext context;
	Display *disp;
	Window xwin;
} ccGLGtkPriv;

#define CC_GLGTK_GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), CC_GLGTK_TYPE, ccGLGtkPriv))

G_DEFINE_TYPE(ccGLGtk, ccGLGtk, GTK_TYPE_WIDGET)

/* Attribute list for a double buffered OpenGL context, with at least 4 bits per
 * color and a 16 bit depth buffer */
static int _glAttrList[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None};

static gboolean ccGLGtkDraw(GtkWidget *widget, cairo_t *cr)
{
	return false;
}

static void ccGLGtkSendConfigure(GtkWidget *widget)
{
	GdkEvent *event = gdk_event_new(GDK_CONFIGURE);

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	event->configure.window = g_object_ref(gtk_widget_get_window(widget));
	event->configure.send_event = true;
	event->configure.x = alloc.x;
	event->configure.y = alloc.y;
	event->configure.width = alloc.width;
	event->configure.height = alloc.height;

	gtk_widget_event(widget, event);
	gdk_event_free(event);
}

static void ccGLGtkRealize(GtkWidget *widget)
{
	ccGLGtkPriv *priv = CC_GLGTK_GET_PRIV(CC_GLGTK(widget));

	gtk_widget_set_realized(widget, true);

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	priv->disp = gdk_x11_get_default_xdisplay();
	XVisualInfo *vi = glXChooseVisual(priv->disp, 0, _glAttrList);

	priv->context = glXCreateContext(priv->disp, vi, NULL, GL_TRUE);
	XFree(vi);

	GdkWindowAttr attr = {
		.window_type = GDK_WINDOW_CHILD,
		.x = alloc.x,
		.y = alloc.y,
		.width = alloc.width,
		.height = alloc.height,
		.wclass = GDK_INPUT_OUTPUT,
		.visual = gdk_visual_get_best_with_both(vi->depth, GDK_VISUAL_DIRECT_COLOR),
		.event_mask = gtk_widget_get_events(widget)
	};

	priv->win = gdk_window_new(gtk_widget_get_parent_window(widget), &attr, GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL);
	priv->xwin = gdk_x11_window_get_xid(priv->win);

	gdk_window_set_user_data(priv->win, widget);
	gtk_widget_set_window(widget, priv->win);
	g_object_ref(widget);

	ccGLGtkSendConfigure(widget);
}

static void ccGLGtkUnrealize(GtkWidget *widget)
{
	ccGLGtkPriv *priv = CC_GLGTK_GET_PRIV(CC_GLGTK(widget));

	if(priv->disp){
		glXDestroyContext(priv->disp, priv->context);

		priv->disp = NULL;
	}

	GTK_WIDGET_CLASS(ccGLGtk_parent_class)->unrealize(widget);
}

static void ccGLGtkSizeAllocate(GtkWidget *widget, GtkAllocation *alloc)
{
	g_return_if_fail(CC_GLGTK_IS_TYPE(widget));
	g_return_if_fail(alloc != NULL);

	gtk_widget_set_allocation(widget, alloc);
	
	if(gtk_widget_get_realized(widget)){
		if(gtk_widget_get_has_window(widget)){
			gdk_window_move_resize(gtk_widget_get_window(widget), alloc->x, alloc->y, alloc->width, alloc->height);
		}

		ccGLGtkSendConfigure(widget);
	}
}

static void ccGLGtk_class_init(ccGLGtkClass *klass)
{
	GtkWidgetClass *wKlass;
	g_type_class_add_private(klass, sizeof(ccGLGtkClass));
	wKlass = GTK_WIDGET_CLASS(klass);

	wKlass->realize = ccGLGtkRealize;
	wKlass->unrealize = ccGLGtkUnrealize;
	wKlass->size_allocate = ccGLGtkSizeAllocate;
	wKlass->draw = ccGLGtkDraw;
}

static void ccGLGtk_init(ccGLGtk *self)
{
	GtkWidget *widget = (GtkWidget*)self;
	ccGLGtkPriv *priv = CC_GLGTK_GET_PRIV(self);

	gtk_widget_set_can_focus(widget, true);
	gtk_widget_set_receives_default(widget, true);
	gtk_widget_set_has_window(widget, true);
	//gtk_widget_set_double_buffered(widget, false);

	priv->disp = NULL;
}

GtkWidget *ccGLGtk_new(void)
{
	return g_object_new(CC_GLGTK_TYPE, NULL);
}

void ccGLGtkMakeCurrent(ccGLGtk *widget)
{
	ccGLGtkPriv *priv = CC_GLGTK_GET_PRIV(widget);
	glXMakeCurrent(priv->disp, priv->xwin, priv->context);
}

void ccGLGtkSwapBuffers(ccGLGtk *widget)
{
	ccGLGtkPriv *priv = CC_GLGTK_GET_PRIV(widget);
	glXSwapBuffers(priv->disp, priv->xwin);
}
