#pragma once

#if defined CC_USE_ALL || defined CC_USE_TEXT

#include <X11/X.h>
#include <X11/Xlib.h>

#include <ccore/text.h>

ccTextEvent ccTextEventHandle(XEvent event);

#endif
