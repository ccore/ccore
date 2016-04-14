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
		ccErrorPush(CC_ERROR_WINDOW_NONE);
		return CC_FAIL;
	}

	XVisualInfo *visual = glXChooseVisual(XD->display, XD->screen, attrList);
	if(CC_UNLIKELY(!visual)) {
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	XD->context = glXCreateContext(XD->display, visual, NULL, GL_TRUE);
	glXMakeCurrent(XD->display, XD->win, XD->context);

	return CC_SUCCESS;
}

ccError ccGLContextFree(void)
{
	if(CC_UNLIKELY(XD->context == NULL)) {
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	glXDestroyContext(XD->display, XD->context);

	return CC_SUCCESS;
}

ccError ccGLBuffersSwap(void)
{
	if(CC_UNLIKELY(XD->context == NULL)) {
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	glXSwapBuffers(XD->display, XD->win);

	return CC_SUCCESS;
}

bool ccGLContextIsActive(void)
{
	return XD->context != NULL;
}
