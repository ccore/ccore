#include <ccore/core.h>

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
#include <ccore/gamepad.h>
#endif
#include <ccore/display.h>
#include <ccore/window.h>
#include <ccore/opengl.h>
#if defined CC_USE_ALL || defined CC_USE_FILE
#include <ccore/file.h>
#endif

#include <ccore/event.h>

ccError ccInitialize(void)
{
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	_ccGamepads = NULL;
#endif
	_ccDisplays = NULL;
	_ccWindow = NULL;

	return CC_E_NONE;
}

ccError ccFree(void)
{
#if defined CC_USE_ALL || defined CC_USE_FILE
	_ccFileFree();
#endif

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	if(_ccGamepads != NULL) {
		ccGamepadFree();
	}
#endif
	if(_ccWindow != NULL) {
		if(ccGLContextIsActive()){
			ccGLContextFree();
		}
		ccWindowFree();
	}
	if(_ccDisplays != NULL) {
		ccDisplayFree();
	}

	return CC_E_NONE;
}

const char *ccErrorString(ccError error)
{
	switch(error){
		case CC_E_NONE:
			return "No errors";

		case CC_E_INVALID_ARGUMENT:
			return "A wrong argument is supplied to the function";

		case CC_E_WM:
			return "Something went wrong with the Window Manager";
		case CC_E_OS:
			return "Something went wrong with the Operating System";

			// Display related
		case CC_E_DISPLAY_NONE:
			return "Could not open display";
		case CC_E_DISPLAY_RESOLUTIONCHANGE:
			return "Could not change display resolution";

			// Window related
		case CC_E_WINDOW_NONE:
			return "A window was not initialized";
		case CC_E_WINDOW_CREATE:
			return "Can't create the window";
		case CC_E_WINDOW_DESTROY:
			return "Can't destroy the window";
		case CC_E_WINDOW_MODE:
			return "Couldn't change the window mode";
		case CC_E_WINDOW_CURSOR:
			return "The cursor couldn't be changed or moved";
		case CC_E_WINDOW_CLIPBOARD:
			return "The clipboad couldn't be read or written to";	
			
			// Framebuffer related
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
		case CC_E_FRAMEBUFFER_SHAREDMEM:
			return "Failed initializing shared memory for the framebuffer";
		case CC_E_FRAMEBUFFER_CREATE:
			return "Can't create a framebuffer instance";
		case CC_E_FRAMEBUFFER_PIXELFORMAT:
			return "The pixel format the framebuffer wants to use is not supported";
#endif

			// OpenGL related
		case CC_E_GL_VERSION:
			return "The current OpenGL version is not supported";
		case CC_E_GL_CONTEXT:
			return "The openGL context could not be created";
		case CC_E_GL_BUFFERSWAP:
			return "Failed to swap the buffers";

			// Thread related
		case CC_E_THREAD_CREATE:
			return "A thread could not be started";
		case CC_E_THREAD_MUTEXCREATE:
			return "A mutex object couldn't be created";
		case CC_E_THREAD_MUTEX:
			return "Error working with a mutex";

			// Gamepad related
		case CC_E_GAMEPAD_NONE:
			return "No gamepad could be found";
		case CC_E_GAMEPAD_DATA:
			return "The gamepad could not be read";
		case CC_E_GAMEPAD_HAPTICNONE:
			return "This haptic motor is not accessible";

			// Mouse related
		case CC_E_MOUSE_NONE:
			return "No mouse could be found";
		case CC_E_MOUSE_DATA:
			return "The mouse could not be read";

			// Memory related
		case CC_E_MEMORY_OVERFLOW:
			return "Out of memory";

			// File related
		case CC_E_FILE_OPEN:
			return "Could not open file";
		default:
			return "Error message not defined";
	}
}

char *ccEventString(ccEvent event)
{
	switch(event.type){
		case CC_EVENT_SKIP:
			return "Skip";
		case CC_EVENT_KEY_DOWN:
			return "Key Down";
		case CC_EVENT_KEY_UP:
			return "Key Up";
		case CC_EVENT_MOUSE_MOVE:
			return "Mouse Move";
		case CC_EVENT_MOUSE_DOWN:
			return "Mouse Down";
		case CC_EVENT_MOUSE_UP:
			return "Mouse Up";
		case CC_EVENT_MOUSE_SCROLL:
			return "Mouse Scroll";
		case CC_EVENT_FOCUS_GAINED:
			return "Focus Gained";
		case CC_EVENT_FOCUS_LOST:
			return "Focus Lost";
		case CC_EVENT_WINDOW_QUIT:
			return "Window Quit";
		case CC_EVENT_WINDOW_RESIZE:
			return "Window Resize";
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
		case CC_EVENT_GAMEPAD:
			return "Gamepad";
#endif
		default:
			return "Error, event not listed";
	}
}
