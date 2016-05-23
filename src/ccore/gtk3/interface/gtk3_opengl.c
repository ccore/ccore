#include "gtk3_window.h"

#include <epoxy/gl.h>

#include <gdk/gdk.h>

#include <ccore/core.h>
#include <ccore/display.h>

static GtkWidget *_gWidget = NULL;
static bool _canDraw = false;

ccError ccGLContextFree(void);

static void eventDestroy(GtkWidget *widget, gpointer data)
{
	_gWidget = NULL;

	ccGLContextFree();
}

static gboolean eventRender(GtkGLArea *area, GdkGLContext *context)
{
	printf("Render\n");

	return true;
}

static void eventRealize(GtkGLArea *area)
{
	gtk_gl_area_make_current(area);
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

	_gWidget = gtk_gl_area_new();
	g_signal_connect(_gWidget, "destroy", G_CALLBACK(eventDestroy), NULL);
	g_signal_connect(_gWidget, "render", G_CALLBACK(eventRender), NULL);
	g_signal_connect(_gWidget, "realize", G_CALLBACK(eventRealize), NULL);

	//gtk_gl_area_set_auto_render(GTK_GL_AREA(_gWidget), false);

	gtk_container_add(GTK_CONTAINER(win), _gWidget);
	gtk_widget_show_all(win);

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

	if(_gWidget == NULL){
		return CC_E_NONE;
	}

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
