#include "gtk3_window.h"

#include <string.h>

#include <gtk/gtk.h>

#include <ccore/core.h>
#include <ccore/window.h>
#include <ccore/gamepad.h>
#include <ccore/opengl.h>
#include <ccore/types.h>
#include <ccore/event.h>
#include <ccore/assert.h>
#include <ccore/print.h>

static void activate(GtkApplication *app, gpointer udata)
{
	GD->win = gtk_application_window_new(GD->app);
	gtk_window_set_title(GTK_WINDOW(GD->win), GD->title);
	gtk_window_set_default_size(GTK_WINDOW(GD->win), rect.width, rect.height);
	gtk_widget_show_all(GD->win);

	free(GD->title);
}

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
	GD->app = gtk_application_new(title, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(GD->app, "activate", G_CALLBACK(activate), NULL);

	GD->title = (char*)malloc(strlen(title));
	strcpy(GD->title, title);

	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
	g_object_unref(app);

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
