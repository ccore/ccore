#include "gtk3_window.h"

#include <epoxy/gl.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <GL/glx.h>

#include <ccore/core.h>
#include <ccore/display.h>
#include <ccore/opengl.h>

static GLXContext _glContext;
static Display *_xDisplay;
static Window _xWin;

/* Attribute list for a double buffered OpenGL context, with at least 4 bits per
 * color and a 16 bit depth buffer */
static int _glAttrList[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None};

ccError ccGLContextBind(void)
{	
	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
#endif

	_xDisplay = gdk_x11_get_default_xdisplay();
	XVisualInfo *vi = glXChooseVisual(_xDisplay, 0, _glAttrList);
	if(CC_UNLIKELY(!vi)){
		return CC_E_GL_CONTEXT;
	}

	_glContext = glXCreateContext(_xDisplay, vi, NULL, GL_TRUE);
	XFree(vi);

	Window _xWindow = gdk_x11_window_get_xid(gtk_widget_get_window(win));
	glXMakeCurrent(_xDisplay, _xWindow, _glContext);

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{		
#ifdef _DEBUG
	assert(_xDisplay != NULL);
#endif

	glXDestroyContext(_xDisplay, _glContext);

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
#ifdef _DEBUG
	assert(_xDisplay != NULL);
#endif

	glXSwapBuffers(_xDisplay, _xWin);

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
	return _xDisplay != NULL;
}
