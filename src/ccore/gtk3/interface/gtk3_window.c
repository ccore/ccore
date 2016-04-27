#include "gtk3_window.h"

#include <ccore/core.h>
#include <ccore/window.h>
#include <ccore/gamepad.h>
#include <ccore/opengl.h>
#include <ccore/types.h>
#include <ccore/event.h>
#include <ccore/assert.h>
#include <ccore/print.h>

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
	return CC_E_NONE;
}

bool ccWindowEventPoll(void)
{
	return false;
}

ccError ccWindowResizeMove(ccRect rect)
{
	return CC_E_NONE;
}

ccError ccWindowSetCentered(void)
{
	return CC_E_NONE;
}

ccError ccWindowSetWindowed(ccRect *rect)
{
	return CC_E_NONE;
}

ccError ccWindowSetMaximized(void)
{
	return CC_E_NONE;
}

ccError ccWindowSetFullscreen(int displayCount, ...)
{
	return CC_E_NONE;
}

ccError ccWindowSetTitle(const char *title)
{
	return CC_E_NONE;
}

ccError ccWindowSetBlink(void)
{
	return CC_E_NONE;
}

ccError ccWindowIconSet(ccPoint size, unsigned long *icon)
{
	return CC_E_NONE;
}

ccError ccWindowMouseSetPosition(ccPoint target)
{
	return CC_E_NONE;
}

ccError ccWindowMouseSetCursor(ccCursor cursor)
{
	return CC_E_NONE;
}

#if defined CC_USE_ALL || defined CC_USE_FRAMEBUFFER
ccError ccWindowFramebufferCreate(ccFramebufferFormat *format)
{
	return CC_E_NONE;
}

ccError ccWindowFramebufferUpdate()
{
	return CC_E_NONE;
}

ccError ccWindowFramebufferFree()
{
	return CC_E_NONE;
}
#endif

ccError ccWindowClipboardSet(const char *data)
{
	return CC_E_NONE;
}

char *ccWindowClipboardGet(void)
{
	return NULL;
}
