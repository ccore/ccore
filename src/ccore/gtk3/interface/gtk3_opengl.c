#include "gtk3_window.h"

#include <GL/gl.h>

#include <gdk/gdk.h>

#include <ccore/core.h>
#include <ccore/display.h>

static GtkWidget *_gContext = NULL;
static bool _hasContext = false;

static void eventDestroy(GtkWidget *widget, gpointer data)
{
	_gContext = NULL;
	_hasContext = false;
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
	assert(_gContext == NULL);
#endif

	_gContext = gtk_gl_area_new();
	g_signal_connect(_gContext, "destroy", G_CALLBACK(eventDestroy), NULL);

	gtk_gl_area_set_auto_render(GTK_GL_AREA(_gContext), false);

	gtk_container_add(GTK_CONTAINER(win), _gContext);
	gtk_widget_show_all(win);

	_hasContext = true;

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
	assert(_gContext == NULL);
#endif

	gtk_container_remove(GTK_CONTAINER(win), _gContext);
	gtk_widget_destroy(_gContext);

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	if(!_hasContext){
		if(_gContext != NULL){
			return CC_E_GL_CONTEXT;
		}else{
			return CC_E_NONE;
		}
	}

#ifdef _DEBUG
	assert(_gContext != NULL);
#endif
	
	gtk_gl_area_queue_render(GTK_GL_AREA(_gContext));

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return false;
#endif

	return _hasContext;
}
