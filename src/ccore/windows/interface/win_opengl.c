#include "win_opengl.h"

ccError ccGLContextBind(void)
{
	int pixelFormatIndex;

	ccAssert(ccWindowExists());

	_CC_WINDOW_DATA->hdc = GetDC(_CC_WINDOW_DATA->winHandle);
	if(_CC_WINDOW_DATA->hdc == NULL) {
		return CC_E_GL_CONTEXT;
	}

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		16,
		0, 0, 0, 0, 0, 0, 0
	};

	pixelFormatIndex = ChoosePixelFormat(_CC_WINDOW_DATA->hdc, &pfd);
	if(pixelFormatIndex == 0) {
		return CC_E_GL_CONTEXT;
	}

	if(SetPixelFormat(_CC_WINDOW_DATA->hdc, pixelFormatIndex, &pfd) == FALSE) {
		return CC_E_GL_CONTEXT;
	}

	_CC_WINDOW_DATA->renderContext = wglCreateContext(_CC_WINDOW_DATA->hdc);
	if(_CC_WINDOW_DATA->renderContext == NULL) {
		return CC_E_GL_CONTEXT;
	}

	//Make window the current context
	if(wglMakeCurrent(_CC_WINDOW_DATA->hdc, _CC_WINDOW_DATA->renderContext) == FALSE) {
		return CC_E_GL_CONTEXT;
	}

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{
	ccAssert(_ccWindow != NULL);

	wglDeleteContext(_CC_WINDOW_DATA->renderContext);
	_CC_WINDOW_DATA->renderContext = NULL;

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
	ccAssert(_ccWindow != NULL);
	if(SwapBuffers(_CC_WINDOW_DATA->hdc) == TRUE) {
		return CC_E_NONE;
	}
	else{
		return CC_E_GL_BUFFERSWAP;
	}
}

bool ccGLContextIsActive(void)
{
	return _CC_WINDOW_DATA->renderContext != NULL;
}
