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

void ccInitialize(void)
{
#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	_ccGamepads = NULL;
#endif
	_ccDisplays = NULL;
	_ccWindow = NULL;
}

void ccFree(void)
{
	_ccErrorFree();

#if defined CC_USE_ALL || defined CC_USE_FILE
	_ccFileFree();
#endif

#if defined CC_USE_ALL || defined CC_USE_GAMEPAD
	if(_ccGamepads != NULL) {
		ccGamepadFree();
	}
#endif
	if(_ccWindow != NULL) {
		if(ccGLContextIsActive()) ccGLContextFree();
		ccWindowFree();
	}
	if(_ccDisplays != NULL){
		ccDisplayFree();
	}
}
