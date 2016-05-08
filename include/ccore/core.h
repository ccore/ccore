//__________________________________________________________________________________//
//                               ______                                             //
//                              /  ___ /__  _ __ ___                                //
//                             /  / __/ _ \| '__/ _ \                               //
//                            |  | (_| (_) | | |  __/                               //
//                             \  \___\___/|_|  \___| 1.1                           //
//                              \______\                                            //
//                                                                                  //
//             Copyright (C) 2014 \ Job Talle (jobtalle@hotmail.com)                //
//                                 \ Thomas Versteeg (thomas@ccore.org)             //
//__________________________________________________________________________________//
//                                                                                  //
//      This program is free software: you can redistribute it and/or modify        //
//      it under the terms of the 3-clause BSD license.                             //
//                                                                                  //
//      You should have received a copy of the 3-clause BSD License along with      //
//      this program. If not, see <http://opensource.org/licenses/>.                //
//__________________________________________________________________________________//

#pragma once

#ifdef __linux__
#define X11
#define LINUX
#elif defined _WIN32
#define WINDOWS
#elif defined __APPLE__
#define OSX
#else
#error "OS not supported!"
#endif

#ifdef __GNUC__
#define CC_LIKELY(x) __builtin_expect(!!(x), 1)
#define CC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CC_LIKELY(x) (x)
#define CC_UNLIKELY(x) (x)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	CC_E_NONE = 0, // No errors

	// Global
	CC_E_INVALID_ARGUMENT,
	CC_E_WM, // A error caused by the window manager
	CC_E_OS, // A error caused by the operating system

	// Display related
	CC_E_DISPLAY_NONE, // The window couldn't find a display to attach to
	CC_E_DISPLAY_RESOLUTIONCHANGE, // Resolution change failed

	// Window related
	CC_E_WINDOW_NONE,
	CC_E_WINDOW_CREATE, // The window can't be created
	CC_E_WINDOW_DESTROY, // The window can't be destroyed
	CC_E_WINDOW_MODE, // The window mode couldn't be changed (also moving & resolution)
	CC_E_WINDOW_CURSOR, // The cursor couldn't be changed or moved
	CC_E_WINDOW_CLIPBOARD, // The clipboad couldn't be read or written to

	// Framebuffer related
#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
	CC_E_FRAMEBUFFER_SHAREDMEM, // Shared memory functions not available
	CC_E_FRAMEBUFFER_CREATE, // Couldn't create a framebuffer instance
	CC_E_FRAMEBUFFER_PIXELFORMAT, // Pixel format not supported
#endif

	// OpenGL related
	CC_E_GL_VERSION, // The target OpenGL version is not supported
	CC_E_GL_CONTEXT, // OpenGL context creation failed
	CC_E_GL_BUFFERSWAP, // The buffers couldn't swap

	// Thread related
	CC_E_THREAD_CREATE, // CCORE couldn't start a thread
	CC_E_THREAD_MUTEXCREATE, // A mutex object couldn't be created
	CC_E_THREAD_MUTEX, // Error working with a mutex

	// Gamepad related
	CC_E_GAMEPAD_NONE, // No gamepads could be found
	CC_E_GAMEPAD_DATA, // The gamepad could not be read
	CC_E_GAMEPAD_HAPTICNONE, // This motor is not accessible

	// Mouse related
	CC_E_MOUSE_NONE, // No mice could be found
	CC_E_MOUSE_DATA, // The mice could not be read

	// Memory related
	CC_E_MEMORY_OVERFLOW,

	// File related
	CC_E_FILE_OPEN, // Error opening the file
} ccError;

ccError ccFree(void); // Free ccore

const char *ccErrorString(ccError error); // Returns a error string

#ifdef __cplusplus
}
#endif
