#include "gtk3_window.h"

#include <string.h>
#include <stdint.h>

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

static void eventDestroy(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_QUIT;
}

static void eventButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		GD->events |= _EV_MOUSE_LEFT_DOWN;
	}else if(event->button == 3){
		GD->events |= _EV_MOUSE_RIGHT_DOWN;
	}
}

static void eventButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		GD->events |= _EV_MOUSE_LEFT_UP;
	}else if(event->button == 3){
		GD->events |= _EV_MOUSE_RIGHT_UP;
	}
}

static void eventFocusIn(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_FOCUS_IN;
}

static void eventFocusOut(GtkWidget *widget, gpointer data)
{
	GD->events |= _EV_FOCUS_OUT;
}

static void eventResize(GtkWidget *widget, GdkRectangle *rect, gpointer data)
{
	_ccWindow->rect.width = rect->width;
	_ccWindow->rect.height = rect->height;

	GD->events |= _EV_RESIZE;
}

static void imageDestroy(guchar *pixels, gpointer data)
{
	if(pixels){
		g_free(pixels);
	}
}

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
	ccAssert(rect.width > 0 && rect.height > 0);

	if(CC_UNLIKELY(_ccWindow)) {
		return CC_E_WINDOW_CREATE;
	}

	_ccWindow = malloc(sizeof(ccWindow));
	if(CC_UNLIKELY(!_ccWindow)){
		return CC_E_MEMORY_OVERFLOW;
	}	

	_ccWindow->data = calloc(1, sizeof(ccWindow_gtk3));
	if(CC_UNLIKELY(!_ccWindow->data)){
		return CC_E_MEMORY_OVERFLOW;
	}

	if(CC_UNLIKELY(!gtk_init_check(NULL, NULL))){
		return CC_E_WM;
	}

	GD->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(GD->win), title);
	gtk_window_set_default_size(GTK_WINDOW(GD->win), rect.width, rect.height);
	gtk_window_set_resizable(GTK_WINDOW(GD->win), !(flags & CC_WINDOW_FLAG_NORESIZE));
	gtk_window_set_decorated(GTK_WINDOW(GD->win), !(flags & CC_WINDOW_FLAG_NOBUTTONS));
	gtk_window_set_keep_above(GTK_WINDOW(GD->win), flags & CC_WINDOW_FLAG_ALWAYSONTOP);

	g_signal_connect(GD->win, "destroy", G_CALLBACK(eventDestroy), NULL);
	g_signal_connect(GD->win, "button-press-event", G_CALLBACK(eventButtonPress), NULL);
	g_signal_connect(GD->win, "button-release-event", G_CALLBACK(eventButtonRelease), NULL);
	g_signal_connect(GD->win, "focus-in-event", G_CALLBACK(eventFocusIn), NULL);
	g_signal_connect(GD->win, "focus-out-event", G_CALLBACK(eventFocusOut), NULL);
	g_signal_connect(GD->win, "size-allocate", G_CALLBACK(eventResize), NULL);

	gtk_widget_show_all(GD->win);

	_ccWindow->rect = rect;
	GD->flags = flags;

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

	// We can't force to resize the window when it's not resizable
	if(GD->flags & CC_WINDOW_FLAG_NORESIZE){
		gtk_window_set_resizable(GTK_WINDOW(GD->win), true);
	}

	gtk_window_move(GTK_WINDOW(GD->win), rect.x, rect.y);
	gtk_widget_set_size_request(GD->win, rect.width, rect.height);

	if(GD->flags & CC_WINDOW_FLAG_NORESIZE){
		gtk_window_set_resizable(GTK_WINDOW(GD->win), false);
	}

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
	ccAssert(_ccWindow);
	ccAssert(GD->win);

	gtk_window_unfullscreen(GTK_WINDOW(GD->win));
	gtk_window_unmaximize(GTK_WINDOW(GD->win));

	return ccWindowResizeMove(*rect);
}

ccError ccWindowSetMaximized(void)
{
	ccAssert(_ccWindow);
	ccAssert(GD->win);

	// We can't force to resize the window when it's not resizable
	if(GD->flags & CC_WINDOW_FLAG_NORESIZE){
		gtk_widget_hide(GD->win);
		gtk_window_set_resizable(GTK_WINDOW(GD->win), true);
	}

	//gtk_window_unfullscreen(GTK_WINDOW(GD->win));
	gtk_window_maximize(GTK_WINDOW(GD->win));

	if(GD->flags & CC_WINDOW_FLAG_NORESIZE){
		gtk_widget_show_all(GD->win);
	}

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
	ccAssert(_ccWindow);
	ccAssert(GD->win);

	gtk_window_set_urgency_hint(GTK_WINDOW(GD->win), true);

	return CC_E_NONE;
}

ccError ccWindowIconSet(ccPoint size, const uint32_t *icon)
{
	ccAssert(_ccWindow);
	ccAssert(size.x > 0 && size.y > 0);

	int len = size.x * size.y;
	int totallen = len * sizeof(uint32_t);
	guchar *buf = (guchar*)g_malloc(totallen * 20);
	if(!buf){
		return CC_E_MEMORY_OVERFLOW;
	}

	union pixel {
		uint32_t l;
		char c[4];
	};

	// Swap BGR for RGB, GdkPixbuf only supports RGB
	int i;
	for(i = 0; i < totallen; i++){
		union pixel p;
		p.l = icon[i];
		buf[(i << 2) + 0] = p.c[2];
		buf[(i << 2) + 1] = p.c[1];
		buf[(i << 2) + 2] = p.c[0];
		buf[(i << 2) + 3] = p.c[3];
	}

	GdkPixbuf *pix = gdk_pixbuf_new_from_data(buf, GDK_COLORSPACE_RGB, true, 8, size.x, size.y, size.x * sizeof(uint32_t), imageDestroy, NULL);
	if(!buf){
		return CC_E_WM;
	}

	gtk_window_set_icon(GTK_WINDOW(GD->win), pix);
	
	g_object_unref(pix);

	return CC_E_NONE;
}

ccError ccWindowMouseSetPosition(ccPoint target)
{
	return CC_E_NONE;
}

ccError ccWindowMouseSetCursor(ccCursor cursor)
{
	const char *name;
	switch(cursor){
		case CC_CURSOR_ARROW:
			name = "default";
			break;
		case CC_CURSOR_CROSS:
			name = "crosshair";
			break;
		case CC_CURSOR_BEAM:
			name = "text";
			break;
		case CC_CURSOR_MOVE:
			name = "move";
			break;
		case CC_CURSOR_HAND:
			name = "pointer";
			break;
		case CC_CURSOR_SIZEH:
			name = "ew-resize";
			break;
		case CC_CURSOR_SIZEV:
			name = "ns-resize";
			break;
		case CC_CURSOR_NO:
			name = "not-allowed";
			break;
		case CC_CURSOR_QUESTION:
			name = "help";
			break;
		case CC_CURSOR_NONE:
			name = "none";
			break;
	}

	GdkCursor *cur = gdk_cursor_new_from_name(gdk_display_get_default(), name);
	gdk_window_set_cursor(gtk_widget_get_window(GD->win), cur);
	
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
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_default(gdk_display_get_default());

	gtk_clipboard_set_text(clip, data, strlen(data));

	return CC_E_NONE;
}

char *ccWindowClipboardGet(void)
{
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_default(gdk_display_get_default());

	return gtk_clipboard_wait_for_text(clip);
}
