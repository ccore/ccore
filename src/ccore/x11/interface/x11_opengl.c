#include "x11_opengl.h"

/* Attribute list for a double buffered OpenGL context, with at least 4 bits per color and a 16 bit depth buffer */
static int attrList[] =
{
	GLX_RGBA, GLX_DOUBLEBUFFER,
	GLX_RED_SIZE, 4,
	GLX_GREEN_SIZE, 4,
	GLX_BLUE_SIZE, 4,
	GLX_DEPTH_SIZE, 16,
	None
};

ccReturn ccGLContextBind(void)
{
	XVisualInfo *visual;

	if(CC_UNLIKELY(_ccWindow == NULL)){
		ccErrorPush(CC_ERROR_WINDOW_NONE);
		return CC_FAIL;
	}

	visual = glXChooseVisual(XWINDATA->XDisplay, XWINDATA->XScreen, attrList);
	if(CC_UNLIKELY(!visual)){
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	XWINDATA->XContext = glXCreateContext(XWINDATA->XDisplay, visual, NULL, GL_TRUE);
	glXMakeCurrent(XWINDATA->XDisplay, XWINDATA->XWindow, XWINDATA->XContext);

	return CC_SUCCESS;
}

ccReturn ccGLContextFree(void)
{
	if(CC_UNLIKELY(XWINDATA->XContext == NULL)){
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	glXDestroyContext(XWINDATA->XDisplay, XWINDATA->XContext);
	
	return CC_SUCCESS;
}

ccReturn ccGLBuffersSwap(void)
{
	if(CC_UNLIKELY(XWINDATA->XContext == NULL)){
		ccErrorPush(CC_ERROR_GL_CONTEXT);
		return CC_FAIL;
	}

	glXSwapBuffers(XWINDATA->XDisplay, XWINDATA->XWindow);

	return CC_SUCCESS;
}

bool ccGLContextIsActive(void)
{
	return XWINDATA->XContext != NULL;
}
