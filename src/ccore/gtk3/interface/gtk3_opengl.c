#include "gtk3_window.h"
#include "gtk3_widget.h"

#include <GL/glx.h>

#include <ccore/core.h>
#include <ccore/display.h>
#include <ccore/opengl.h>

static GtkWidget *_context = NULL;

ccError ccGLContextBind(void)
{	
	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
	assert(_context == NULL);
#endif

	_context = ccGLGtk_new();

	gtk_container_add(GTK_CONTAINER(win), _context);

	gtk_widget_show_all(win);

	ccGLGtkMakeCurrent(CC_GLGTK(_context));

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{		
	//TODO

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
#ifdef _DEBUG
	assert(_context != NULL);
#endif

	ccGLGtkSwapBuffers(CC_GLGTK(_context));

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
	return _context != NULL;
}
