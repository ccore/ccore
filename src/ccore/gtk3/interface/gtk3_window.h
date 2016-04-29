#pragma once

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <ccore/core.h>
#include <ccore/window.h>

typedef struct {
	GtkWidget *win;
	int events;
	int flags;
} ccWindow_gtk3;

#define GD ((ccWindow_gtk3*)_ccWindow->data)
