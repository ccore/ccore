#include "gtk3_window.h"
#include "gtk3_opengl.h"

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

static ccRect _rect;
static ccPoint _mouse;
static ccEvent _event;
static ccDisplay *_display;
static bool _supportsRawInput;
static bool _hasWindow = false;

static GtkWidget *_gWin;
static int _gEvents;
static int _gFlags;

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
	_gEvents |= _EV_QUIT;
}

static void eventButtonPress(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		_gEvents |= _EV_MOUSE_LEFT_DOWN;
	}else if(event->button == 3){
		_gEvents |= _EV_MOUSE_RIGHT_DOWN;
	}
}

static void eventButtonRelease(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if(event->button == 1){
		_gEvents |= _EV_MOUSE_LEFT_UP;
	}else if(event->button == 3){
		_gEvents |= _EV_MOUSE_RIGHT_UP;
	}
}

static void eventFocusIn(GtkWidget *widget, gpointer data)
{
	_gEvents |= _EV_FOCUS_IN;
}

static void eventFocusOut(GtkWidget *widget, gpointer data)
{
	_gEvents |= _EV_FOCUS_OUT;
}

static void eventResize(GtkWidget *widget, GdkRectangle *rect, gpointer data)
{
	_rect.width = rect->width;
	_rect.height = rect->height;

	_gEvents |= _EV_RESIZE;
}

static void imageDestroy(guchar *pixels, gpointer data)
{
	if(pixels){
		g_free(pixels);
	}
}

GtkWidget *ccGtkGetWidget()
{
	return _gWin;
}

ccError ccWindowCreate(ccRect rect, const char *title, int flags)
{
#ifdef _DEBUG
	assert(rect.width > 0 && rect.height > 0);
#endif

	if(CC_UNLIKELY(_hasWindow)) {
		return CC_E_WINDOW_CREATE;
	}

	if(CC_UNLIKELY(!gtk_init_check(NULL, NULL))){
		return CC_E_WM;
	}

	_gWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(_gWin), title);
	gtk_window_set_default_size(GTK_WINDOW(_gWin), rect.width, rect.height);
	gtk_window_set_resizable(GTK_WINDOW(_gWin), !(flags & CC_WINDOW_FLAG_NORESIZE));
	gtk_window_set_decorated(GTK_WINDOW(_gWin), !(flags & CC_WINDOW_FLAG_NOBUTTONS));
	gtk_window_set_keep_above(GTK_WINDOW(_gWin), flags & CC_WINDOW_FLAG_ALWAYSONTOP);

	g_signal_connect(_gWin, "destroy", G_CALLBACK(eventDestroy), NULL);
	g_signal_connect(_gWin, "button-press-event", G_CALLBACK(eventButtonPress), NULL);
	g_signal_connect(_gWin, "button-release-event", G_CALLBACK(eventButtonRelease), NULL);
	g_signal_connect(_gWin, "focus-in-event", G_CALLBACK(eventFocusIn), NULL);
	g_signal_connect(_gWin, "focus-out-event", G_CALLBACK(eventFocusOut), NULL);
	g_signal_connect(_gWin, "size-allocate", G_CALLBACK(eventResize), NULL);

	gtk_widget_show_all(_gWin);

	_hasWindow = true;
	_rect = rect;
	_gFlags = flags;

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

	ccGtkContextSetCurrent();

	if(_gEvents == 0){
		return false;
	}

	if(_gEvents & _EV_QUIT){
		_event.type = CC_EVENT_WINDOW_QUIT;

		// Clear the flag
		_gEvents &= ~_EV_QUIT;
	}else if(_gEvents & _EV_MOUSE_LEFT_DOWN){
		_event.type = CC_EVENT_MOUSE_DOWN;
		_event.mouseButton = CC_MOUSE_BUTTON_LEFT;

		_gEvents &= ~_EV_MOUSE_LEFT_DOWN;
	}else if(_gEvents & _EV_MOUSE_RIGHT_DOWN){
		_event.type = CC_EVENT_MOUSE_DOWN;
		_event.mouseButton = CC_MOUSE_BUTTON_RIGHT;

		_gEvents &= ~_EV_MOUSE_RIGHT_DOWN;
	}else if(_gEvents & _EV_MOUSE_LEFT_UP){
		_event.type = CC_EVENT_MOUSE_UP;
		_event.mouseButton = CC_MOUSE_BUTTON_LEFT;

		_gEvents &= ~_EV_MOUSE_LEFT_UP;
	}else if(_gEvents & _EV_MOUSE_RIGHT_UP){
		_event.type = CC_EVENT_MOUSE_UP;
		_event.mouseButton = CC_MOUSE_BUTTON_RIGHT;

		_gEvents &= ~_EV_MOUSE_RIGHT_UP;
	}else if(_gEvents & _EV_FOCUS_IN){
		_event.type = CC_EVENT_FOCUS_GAINED;

		_gEvents &= ~_EV_FOCUS_IN;
	}else if(_gEvents & _EV_FOCUS_OUT){
		_event.type = CC_EVENT_FOCUS_LOST;

		_gEvents &= ~_EV_FOCUS_OUT;
	}else if(_gEvents & _EV_RESIZE){
		_event.type = CC_EVENT_WINDOW_RESIZE;

		_gEvents &= ~_EV_RESIZE;
	}

	return true;
}

ccError ccWindowSetRect(ccRect rect)
{
#ifdef _DEBUG
	assert(_hasWindow);
	assert(_gWin);
#endif

	// We can't force to resize the window when it's not resizable
	if(_gFlags & CC_WINDOW_FLAG_NORESIZE){
		gtk_window_set_resizable(GTK_WINDOW(_gWin), true);
	}

	gtk_window_move(GTK_WINDOW(_gWin), rect.x, rect.y);
	gtk_widget_set_size_request(_gWin, rect.width, rect.height);

	if(_gFlags & CC_WINDOW_FLAG_NORESIZE){
		gtk_window_set_resizable(GTK_WINDOW(_gWin), false);
	}

	return CC_E_NONE;
}

ccError ccWindowSetCentered(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(_gWin);
#endif

	gtk_window_set_position(GTK_WINDOW(_gWin), GTK_WIN_POS_CENTER_ALWAYS);

	return CC_E_NONE;
}

ccError ccWindowSetWindowed(ccRect rect)
{
#ifdef _DEBUG
	assert(_hasWindow);
	assert(_gWin);
#endif

	gtk_window_unfullscreen(GTK_WINDOW(_gWin));
	gtk_window_unmaximize(GTK_WINDOW(_gWin));

	return ccWindowSetRect(rect);
}

ccError ccWindowSetMaximized(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(_gWin);
#endif

	// We can't force to resize the window when it's not resizable
	if(_gFlags & CC_WINDOW_FLAG_NORESIZE){
		gtk_widget_hide(_gWin);
		gtk_window_set_resizable(GTK_WINDOW(_gWin), true);
	}

	//gtk_window_unfullscreen(GTK_WINDOW(_gWin));
	gtk_window_maximize(GTK_WINDOW(_gWin));

	if(_gFlags & CC_WINDOW_FLAG_NORESIZE){
		gtk_widget_show_all(_gWin);
	}

	return CC_E_NONE;
}

ccError ccWindowSetFullscreen(int displayCount, ...)
{
	//TODO implement gtk_window_fullscreen_on_window
	gtk_window_fullscreen(GTK_WINDOW(_gWin));

	return CC_E_NONE;
}

ccError ccWindowSetTitle(const char *title)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(_gWin);
#endif

	gtk_window_set_title(GTK_WINDOW(_gWin), title);

	return CC_E_NONE;
}

ccError ccWindowSetBlink(void)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(_gWin);
#endif

	gtk_window_set_urgency_hint(GTK_WINDOW(_gWin), true);

	return CC_E_NONE;
}

ccError ccWindowIconSet(ccPoint size, const uint32_t *icon)
{
#ifdef _DEBUG
	assert(_hasWindow);
#endif
#ifdef _DEBUG
	assert(size.x > 0 && size.y > 0);
#endif

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

	gtk_window_set_icon(GTK_WINDOW(_gWin), pix);
	
	g_object_unref(pix);

	return CC_E_NONE;
}

ccError ccWindowMouseSetPosition(ccPoint target)
{
	GdkDisplay *disp = gdk_display_get_default();
	GdkSeat *seat = gdk_display_get_default_seat(disp);
	GdkDevice *pointer = gdk_seat_get_pointer(seat);
	GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(_gWin));

	int x, y;
	gtk_window_get_position(GTK_WINDOW(_gWin), &x, &y);

	gdk_device_warp(pointer, screen, x + target.x, y + target.y);

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
	gdk_window_set_cursor(gtk_widget_get_window(_gWin), cur);
	
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

void *ccWindowFramebufferGetPixels()
{	
	return NULL;
}
#endif

ccError ccWindowClipboardSet(const char *data)
{
#ifdef GTK3_VERSION_PRE_3
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
#else
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_default(gdk_display_get_default());
#endif

	gtk_clipboard_set_text(clip, data, strlen(data));

	return CC_E_NONE;
}

char *ccWindowClipboardGet(void)
{
#ifdef GTK3_VERSION_PRE_3
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
#else
	GtkClipboard *clip = (GtkClipboard*)gtk_clipboard_get_default(gdk_display_get_default());
#endif

	return gtk_clipboard_wait_for_text(clip);
}

ccEvent ccWindowEventGet(void)
{
	return _event;
}

ccRect ccWindowGetRect(void)
{	
	return _rect;
}
