#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CC_GLGTK_TYPE (ccGLGtk_get_type())
#define CC_GLGTK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CC_GLGTK_TYPE, ccGLGtk))
#define CC_GLGTK_IS_TYPE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CC_GLGTK_TYPE))
#define CC_GLGTK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CC_GLGTK_TYPE, ccGLGtkClass))
#define CC_GLGTK_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CC_GLGTK_TYPE))
#define CC_GLGTK_GET_CLASS(obj) (G_TYPE_INSTANCE((obj), CC_GLGTK_TYPE, ccGLGtkClass))

typedef struct ccGLGtk {
	GtkWidget parent_instance;
} ccGLGtk;

typedef struct {
	GtkWidgetClass parent_class;
} ccGLGtkClass;

GType ccGLGtk_get_type(void);
GtkWidget *ccGLGtk_new(void);

void ccGLGtkMakeCurrent(ccGLGtk *widget);
void ccGLGtkSwapBuffers(ccGLGtk *widget);

G_END_DECLS
