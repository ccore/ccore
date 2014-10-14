#include <ccore/error.h>

static ccError *ccErrorStack;
static int ccErrorStackIndex;
static int ccErrorStackSize;

void ccErrorPush(ccError error)
{
	if(ccErrorStackSize <= ccErrorStackIndex) {
		ccErrorStackSize++;
		ccErrorStack = realloc(ccErrorStack, sizeof(ccError)* ccErrorStackSize);
	}
	ccErrorStack[ccErrorStackIndex] = error;
	ccErrorStackIndex++;
}

ccError ccErrorPop(void)
{
	if(ccErrorStackIndex == 0) {
		return CC_ERROR_NONE;
	}
	else {
		ccErrorStackIndex--;
		return ccErrorStack[ccErrorStackIndex];
	}
}

void _ccErrorFree(void)
{
	if(ccErrorStackSize != 0)
	{
		free(ccErrorStack);
		ccErrorStackSize = 0;
	}
}

const char *ccErrorString(ccError error)
{
	switch(error){
		case CC_ERROR_NONE:
			return "No errors";

		case CC_ERROR_INVALID_ARGUMENT:
			return "A wrong argument is supplied to the function";

		case CC_ERROR_WM:
			return "The Window Manager couldn't process the function";

			// Display related
		case CC_ERROR_DISPLAY_NONE:
			return "Could not open display";
		case CC_ERROR_DISPLAY_RESOLUTIONCHANGE:
			return "Could not change display resolution";

			// Window related
		case CC_ERROR_WINDOW_NONE:
			return "A window was not initialized";
		case CC_ERROR_WINDOW_CREATE:
			return "Can't create the window";
		case CC_ERROR_WINDOW_DESTROY:
			return "Can't destroy the window";
		case CC_ERROR_WINDOW_MODE:
			return "Couldn't change the window mode";
		case CC_ERROR_WINDOW_CURSOR:
			return "The cursor couldn't be changed or moved";
		case CC_ERROR_WINDOW_CLIPBOARD:
			return "The clipboad couldn't be read or written to";

			// OpenGL related
		case CC_ERROR_GL_VERSION:
			return "The current OpenGL version is not supported";
		case CC_ERROR_GL_CONTEXT:
			return "The openGL context could not be created";
		case CC_ERROR_GL_BUFFERSWAP:
			return "Failed to swap the buffers";

			// Thread related
		case CC_ERROR_THREAD_CREATE:
			return "A thread could not be started";
		case CC_ERROR_THREAD_MUTEXCREATE:
			return "A mutex object couldn't be created";
		case CC_ERROR_THREAD_MUTEX:
			return "Error working with a mutex";

			// Networking related
		case CC_ERROR_NET:
			return "A network function failed";

			// Gamepad related
		case CC_ERROR_GAMEPAD_NONE:
			return "No gamepad could be found";
		case CC_ERROR_GAMEPAD_DATA:
			return "The gamepad could not be read";
		case CC_ERROR_GAMEPAD_HAPTICNONE:
			return "This haptic motor is not accessible";

			// Mouse related
		case CC_ERROR_MOUSE_NONE:
			return "No mouse could be found";
		case CC_ERROR_MOUSE_DATA:
			return "The mouse could not be read";

			// Memory related
		case CC_ERROR_MEMORY_OVERFLOW:
			return "Out of memory";

			// File related
		case CC_ERROR_FILE_OPEN:
			return "Could not open file";
		default:
			return "Error message not defined";
	}
}
