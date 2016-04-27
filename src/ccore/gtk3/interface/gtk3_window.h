#pragma once

#include <gtk/gtk.h>

#include <ccore/core.h>
#include <ccore/window.h>

typedef struct {
	GtkWidget *win;
	int events;
} ccWindow_gtk3;

#define GD ((ccWindow_gtk3*)_ccWindow->data)
