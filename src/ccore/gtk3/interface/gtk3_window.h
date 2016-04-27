#pragma once

#include <gtk/gtk.h>

#include <ccore/core.h>

typedef struct {
	GtkApplication *app
	GtkWidget *win;
	char *title;
} ccWindow_gtk3;

#define GD ((ccWindow_gtk3*)_ccWindow->data)
