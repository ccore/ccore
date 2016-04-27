#include "gtk3_window.h"

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <ccore/core.h>
#include <ccore/window.h>
#include <ccore/gamepad.h>
#include <ccore/opengl.h>
#include <ccore/types.h>
#include <ccore/event.h>
#include <ccore/assert.h>
#include <ccore/print.h>

enum {
	_EV_QUIT =             1 << 0,
	_EV_MOUSE_LEFT_DOWN =  1 << 1,
	_EV_MOUSE_RIGHT_DOWN = 1 << 2,
	_EV_MOUSE_LEFT_UP =    1 << 3,
	_EV_MOUSE_RIGHT_UP =   1 << 4,
	_EV_FOCUS_IN =         1 << 5,
	_EV_FOCUS_OUT =        1 << 6,
	_EV_RESIZE =           1 << 7,
};

static void destroy(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_QUIT;
}

static void buttonPress(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		GD->events |= _EV_MOUSE_LEFT_DOWN;
	}else if(event->button == 3){
		GD->events |= _EV_MOUSE_RIGHT_DOWN;
	}
}

static void buttonRelease(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		GD->events |= _EV_MOUSE_LEFT_UP;
	}else if(event->button == 3){
		GD->events |= _EV_MOUSE_RIGHT_UP;
	}
}

static void focusIn(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_FOCUS_IN;
}

static void focusOut(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_FOCUS_OUT;
}

static void resize(GtkWidget *widget, GdkRectangle *rect, gpointer data)
{
	_ccWindow->rect.width = rect->width;
	_ccWindow->rect.height = rect->height;

	GD->events |= _EV_RESIZE;
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
	g_signal_connect(GD->win, "button-press-event", G_CALLBACK(buttonPress), NULL);
	g_signal_connect(GD->win, "button-release-event", G_CALLBACK(buttonRelease), NULL);
	g_signal_connect(GD->win, "focus-in-event", G_CALLBACK(focusIn), NULL);
	g_signal_connect(GD->win, "focus-out-event", G_CALLBACK(focusOut), NULL);
	g_signal_connect(GD->win, "size-allocate", G_CALLBACK(resize), NULL);

	gtk_widget_show_all(GD->win);

	return CC_E_NONE;
}

ccError ccWindowFree(void)
{
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

	if(GD->events & _EV_QUIT){
		_ccWindow->event.type = CC_EVENT_WINDOW_QUIT;

		// Clear the flag
		GD->events &= ~_EV_QUIT;
	}else if(GD->events & _EV_MOUSE_LEFT_DOWN){
		_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
		_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;

		GD->events &= ~_EV_MOUSE_LEFT_DOWN;
	}else if(GD->events & _EV_MOUSE_RIGHT_DOWN){
		_ccWindow->event.type = CC_EVENT_MOUSE_DOWN;
		_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;

		GD->events &= ~_EV_MOUSE_RIGHT_DOWN;
	}else if(GD->events & _EV_MOUSE_LEFT_UP){
		_ccWindow->event.type = CC_EVENT_MOUSE_UP;
		_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_LEFT;

		GD->events &= ~_EV_MOUSE_LEFT_UP;
	}else if(GD->events & _EV_MOUSE_RIGHT_UP){
		_ccWindow->event.type = CC_EVENT_MOUSE_UP;
		_ccWindow->event.mouseButton = CC_MOUSE_BUTTON_RIGHT;

		GD->events &= ~_EV_MOUSE_RIGHT_UP;
	}else if(GD->events & _EV_FOCUS_IN){
		_ccWindow->event.type = CC_EVENT_FOCUS_GAINED;

		GD->events &= ~_EV_FOCUS_IN;
	}else if(GD->events & _EV_FOCUS_OUT){
		_ccWindow->event.type = CC_EVENT_FOCUS_LOST;

		GD->events &= ~_EV_FOCUS_OUT;
	}else if(GD->events & _EV_RESIZE){
		_ccWindow->event.type = CC_EVENT_WINDOW_RESIZE;

		GD->events &= ~_EV_RESIZE;
	}

	return true;
}

ccError ccWindowResizeMove(ccRect rect)
{
	ccAssert(_ccWindow);
	ccAssert(GD->win);

	gtk_window_move(GTK_WINDOW(GD->win), rect.x, rect.y);
	gtk_window_resize(GTK_WINDOW(GD->win), rect.width, rect.height);

	return CC_E_NONE;
}

ccError ccWindowSetCentered(void)
{
	ccAssert(_ccWindow);
	ccAssert(GD->win);

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
	ccAssert(_ccWindow);
	ccAssert(GD->win);

	gtk_window_set_title(GTK_WINDOW(GD->win), title);

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
