#include <ccore/opengl.h>
#include <ccore/window.h>

#include "x11_window.h"

/* Attribute list for a double buffered OpenGL context, with at least 4 bits per
 * color and a 16 bit depth buffer */
static int attrList[] = {GLX_RGBA,
												 GLX_DOUBLEBUFFER,
												 GLX_RED_SIZE,
												 4,
												 GLX_GREEN_SIZE,
												 4,
												 GLX_BLUE_SIZE,
												 4,
												 GLX_DEPTH_SIZE,
												 16,
												 None};

ccError ccGLContextBind(void)
{
	if(CC_UNLIKELY(_ccWindow == NULL)) {
		return CC_E_WINDOW_NONE;
	}

	XVisualInfo *visual = glXChooseVisual(XD->display, XD->screen, attrList);
	if(CC_UNLIKELY(!visual)) {
		return CC_E_GL_CONTEXT;
	}

	XD->context = glXCreateContext(XD->display, visual, NULL, GL_TRUE);
	glXMakeCurrent(XD->display, XD->win, XD->context);

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{
	if(CC_UNLIKELY(XD->context == NULL)) {
		return CC_E_GL_CONTEXT;
	}

	glXDestroyContext(XD->display, XD->context);

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
	if(CC_UNLIKELY(XD->context == NULL)) {
		return CC_E_GL_CONTEXT;
	}

	glXSwapBuffers(XD->display, XD->win);

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
	return XD->context != NULL;
}
