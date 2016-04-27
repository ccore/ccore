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

enum {
	_CC_EVENT_FLAG_QUIT = 1
};

static void destroy(GtkWidget *widget, gpointer data)
{
	GD->events ^= _CC_EVENT_FLAG_QUIT;
}

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
	ccAssert(rect.width > 0 && rect.height > 0);

	if(CC_UNLIKELY(_ccWindow != NULL)) {
		return CC_E_WINDOW_CREATE;
	}

	_ccWindow = malloc(sizeof(ccWindow));
	if(_ccWindow == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}	

	_ccWindow->data = calloc(1, sizeof(ccWindow_gtk3));
	if(_ccWindow->data == NULL){
		return CC_E_MEMORY_OVERFLOW;
	}

	_ccWindow->rect = rect;

	if(!gtk_init_check(NULL, NULL)){
		return CC_E_WM;
	}

	ccAssert(true);

	GD->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(GD->win), title);
	gtk_window_set_default_size(GTK_WINDOW(GD->win), rect.width, rect.height);
	gtk_window_set_resizable(GTK_WINDOW(GD->win), !(flags & CC_WINDOW_FLAG_NORESIZE));
	gtk_window_set_decorated(GTK_WINDOW(GD->win), !(flags & CC_WINDOW_FLAG_NOBUTTONS));
	if(flags & CC_WINDOW_FLAG_ALWAYSONTOP){
		gtk_window_set_keep_above(GTK_WINDOW(GD->win), true);
		gtk_window_resize(GTK_WINDOW(GD->win), rect.width, rect.height);
	}

	g_signal_connect(GD->win, "destroy", G_CALLBACK(destroy), NULL);

	gtk_widget_show_all(GD->win);

	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
	gtk_widget_destroy(GD->win);

	return CC_E_NONE;
}

bool ccWindowEventPoll(void)
{
	if(gtk_events_pending()){
		gtk_main_iteration();
	}

	if(GD->events == 0){
		return false;
	}

	if(GD->events & _CC_EVENT_FLAG_QUIT){
		_ccWindow->event.type = CC_EVENT_WINDOW_QUIT;
	}

	GD->events = 0;

	return true;
}

ccError ccWindowResizeMove(ccRect rect)
{
	ccAssert(false);
	ccAssert(_ccWindow);

	gtk_window_move(GTK_WINDOW(GD->win), rect.x, rect.y);
	gtk_window_resize(GTK_WINDOW(GD->win), rect.width, rect.height);

	return CC_E_NONE;
}

ccError ccWindowSetCentered(void)
{
	ccAssert(false);
	ccAssert(_ccWindow);

	gtk_window_set_position(GTK_WINDOW(GD->win), GTK_WIN_POS_CENTER_ALWAYS);

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
