#include "gtk3_window.h"

#include <epoxy/gl.h>

#include <gdk/gdk.h>

#include <ccore/core.h>
#include <ccore/display.h>

static GtkWidget *_gWidget = NULL;
static GdkGLContext *_gContext = NULL;
static cairo_surface_t *_cSurface = NULL;
static unsigned int _glRb = 0;
static unsigned int _glFb = 0;
static bool _canDraw = false;

ccError ccGLContextFree(void);

static void eventDestroy(GtkWidget *widget, gpointer data)
{
	_gWidget = NULL;

	ccGLContextFree();
}

static gboolean eventConfigure(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	GdkWindow *gdkw = gtk_widget_get_window(widget);

	GError *error  = NULL;
	_gContext = gdk_window_create_gl_context(gdkw, &error);
	if(error != NULL){
		fprintf(stderr, "Gtk error: %s\n", error->message);
		g_error_free(error);
		return CC_E_GL_CONTEXT;
	}

#ifdef DEBUG
	gdk_gl_context_set_debug_enabled(_gContext, true);
#endif

	gdk_gl_context_realize(_gContext, &error);
	if(error != NULL){
		fprintf(stderr, "Gtk error: %s\n", error->message);
		g_error_free(error);
		return CC_E_GL_CONTEXT;
	}

	printf("Event configure 1\n");

	gdk_gl_context_make_current(_gContext);

	return true;
}

static gboolean eventDraw(GtkWidget *widget, cairo_t *cr, gpointer data)
{	
	if(_cSurface == NULL){
		if(!gtk_widget_get_realized(_gWidget)){
			return CC_E_WM;
		}

		gdk_gl_context_make_current(_gContext);
		glGenFramebuffersEXT(1, &_glFb);
		glGenRenderbuffersEXT(1, &_glRb);

		glBindRenderbuffer(GL_RENDERBUFFER, _glRb);

		int w = gtk_widget_get_allocated_width(_gWidget);
		int h = gtk_widget_get_allocated_height(_gWidget);

		int scale = gtk_widget_get_scale_factor(_gWidget);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, w * scale, h * scale);

		if(_cSurface != NULL){
			cairo_surface_destroy(_cSurface);
		}

		_cSurface = gdk_window_create_similar_surface(gtk_widget_get_window(_gWidget), CAIRO_CONTENT_COLOR, w, h);

		printf("Event configure 2\n\n");
	}

	GdkWindow *gdkw = gtk_widget_get_window(widget);

	//cairo_set_source_surface(cr, _cSurface, 0, 0);
	int scale = gtk_widget_get_scale_factor(_gWidget);
	int w = gtk_widget_get_allocated_width(_gWidget) * scale;
	int h = gtk_widget_get_allocated_height(_gWidget) * scale;
	gdk_cairo_draw_from_gl(cr, gdkw, _glRb, GL_RENDERBUFFER, scale, 0, 0, w, h);
	//cairo_set_source_rgb(cr, 0, 0.1, 0);
	cairo_paint(cr);

	gdk_gl_context_make_current(_gContext);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _glFb);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _glRb);

	printf("Draw\n");

	_canDraw = true;

	return false;
}

ccError ccGLContextBind(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
	assert(_gWidget == NULL);
#endif

	_gWidget = gtk_drawing_area_new();
	g_signal_connect(_gWidget, "destroy", G_CALLBACK(eventDestroy), NULL);
	g_signal_connect(_gWidget, "realize", G_CALLBACK(eventConfigure), NULL);
	g_signal_connect(_gWidget, "draw", G_CALLBACK(eventDraw), NULL);

	gtk_container_add(GTK_CONTAINER(win), _gWidget);
	gtk_widget_show_all(win);

	printf("Bind\n");

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{	
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
#endif

	if(_glRb != 0){
		glDeleteRenderbuffersEXT(1, &_glRb);
		_glRb = 0;
	}

	if(_glFb != 0){
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(1, &_glFb);
		_glFb = 0;
	}

	if(_cSurface != NULL){
		cairo_surface_destroy(_cSurface);
	}

	if(_gWidget != NULL){
		gtk_container_remove(GTK_CONTAINER(win), _gWidget);
		gtk_widget_destroy(_gWidget);

		_gWidget = NULL;
	}

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	if(_gWidget == NULL || _cSurface == NULL){
		return CC_E_NONE;
	}

	gtk_widget_queue_draw(_gWidget);
	gdk_gl_context_make_current(_gContext);

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return false;
#endif

	return _gWidget != NULL;
}
