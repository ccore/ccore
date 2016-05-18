#include "gtk3_window.h"

#include <epoxy/gl.h>

#include <gdk/gdk.h>

#include <ccore/core.h>
#include <ccore/display.h>

static GtkWidget *_gContext = NULL;
static cairo_t *_ct = NULL;
static unsigned int _glRb = 0;
static unsigned int _glFb = 0;

ccError ccGLContextFree(void);

static void eventDestroy(GtkWidget *widget, gpointer data)
{
	_gContext = NULL;

	ccGLContextFree();
}

ccError ccGLContextBind(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
	assert(_gContext == NULL);
#endif

	_gContext = gtk_gl_area_new();
	g_signal_connect(_gContext, "destroy", G_CALLBACK(eventDestroy), NULL);

	gtk_gl_area_set_auto_render(GTK_GL_AREA(_gContext), false);

	gtk_container_add(GTK_CONTAINER(win), _gContext);
	gtk_widget_show_all(win);

	GdkWindow *gdkw = gtk_widget_get_window(_gContext);
	GdkGLContext *gdkc = gtk_gl_area_get_context(GTK_GL_AREA(_gContext));
	
	_ct = gdk_cairo_create(gdkw);

	gdk_gl_context_make_current(gdkc);
  glGenFramebuffersEXT(1, &_glFb);
	glGenRenderbuffersEXT(1, &_glRb);

	int scale = gtk_widget_get_scale_factor(_gContext);
	int w = gtk_widget_get_allocated_width(_gContext) * scale;
	int h = gtk_widget_get_allocated_height(_gContext) * scale;
	glBindRenderbuffer(GL_RENDERBUFFER, _glRb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, w, h);

	return CC_E_NONE;
}

ccError ccGLContextFree(void)
{	
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	GtkWidget *win = ccGtkGetWidget();

#ifdef _DEBUG
	assert(win != NULL);
#endif

	if(_glRb != 0){
		glDeleteRenderbuffersEXT(1, &_glRb);
		_glRb = 0;
	}

	if(_glFb != 0){
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(1, &_glFb);
		_glFb = 0;
	}

	if(_gContext != NULL){
		gtk_container_remove(GTK_CONTAINER(win), _gContext);
		gtk_widget_destroy(_gContext);

		_gContext = NULL;
	}

	return CC_E_NONE;
}

ccError ccGLBuffersSwap(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return CC_E_GL_VERSION;
#endif

	if(_gContext == NULL){
		return CC_E_NONE;
	}

#ifdef _DEBUG
	assert(_ct != NULL);
#endif

	GdkWindow *gdkw = gtk_widget_get_window(_gContext);
	GdkGLContext *gdkc = gtk_gl_area_get_context(GTK_GL_AREA(_gContext));

	gdk_gl_context_make_current(gdkc);

	int scale = gtk_widget_get_scale_factor(_gContext);
	int w = gtk_widget_get_allocated_width(_gContext) * scale;
	int h = gtk_widget_get_allocated_height(_gContext) * scale;
	gdk_cairo_draw_from_gl(_ct, gdkw, _glRb, GL_RENDERBUFFER, scale, 0, 0, w, h);

	// Before renderring
	gdk_gl_context_make_current(gdkc);

  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, _glFb);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _glRb);
  glDisable(GL_DEPTH_TEST);

  GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
  if(status != GL_FRAMEBUFFER_COMPLETE_EXT){
		return CC_E_GL_VERSION;
	}

	return CC_E_NONE;
}

bool ccGLContextIsActive(void)
{
#ifdef GTK3_VERSION_PRE_3
	fprintf(stderr, "OpenGL version for GTK version not yet supported!\n");
	return false;
#endif

	return _gContext != NULL;
}
