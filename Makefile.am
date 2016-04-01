#ACLOCAL_AMFLAGS = -I m4 --install
AM_CPPFLAGS += -I$(top_srcdir)/include

pkginclude_HEADERS = include/ccore/assert.h
pkginclude_HEADERS += include/ccore/core.h
pkginclude_HEADERS += include/ccore/display.h
pkginclude_HEADERS += include/ccore/error.h
pkginclude_HEADERS += include/ccore/event.h
pkginclude_HEADERS += include/ccore/file.h
pkginclude_HEADERS += include/ccore/gamepad.h
pkginclude_HEADERS += include/ccore/opengl.h
pkginclude_HEADERS += include/ccore/print.h
pkginclude_HEADERS += include/ccore/string.h
pkginclude_HEADERS += include/ccore/sysinfo.h
#pkginclude_HEADERS += include/ccore/text.h
pkginclude_HEADERS += include/ccore/thread.h
pkginclude_HEADERS += include/ccore/time.h
pkginclude_HEADERS += include/ccore/types.h
pkginclude_HEADERS += include/ccore/window.h

lib_LTLIBRARIES = libccore.la
libccore_la_LDFLAGS = -version-info $(CCORE_VERSION)
libccore_la_SOURCES = src/ccore/common/interface/com_display.c
libccore_la_SOURCES += src/ccore/common/interface/com_event.c
libccore_la_SOURCES += src/ccore/common/interface/com_window.c

libccore_la_SOURCES += src/ccore/common/utils/com_core.c
libccore_la_SOURCES += src/ccore/common/utils/com_error.c
libccore_la_SOURCES += src/ccore/common/utils/com_file.c
libccore_la_SOURCES += src/ccore/common/utils/com_rect.c
libccore_la_SOURCES += src/ccore/common/utils/com_string.c
libccore_la_SOURCES += src/ccore/common/utils/com_sysinfo.c

if BUILD_WINDOWS
libccore_la_SOURCES += src/ccore/windows/interface/win_display.c
libccore_la_SOURCES += src/ccore/windows/interface/win_gamepad.c
libccore_la_SOURCES += src/ccore/windows/interface/win_opengl.c
libccore_la_SOURCES += src/ccore/windows/interface/win_window.c

libccore_la_SOURCES += src/ccore/windows/utils/win_file.c
libccore_la_SOURCES += src/ccore/windows/utils/win_sysinfo.c
libccore_la_SOURCES += src/ccore/windows/utils/win_thread.c
libccore_la_SOURCES += src/ccore/windows/utils/win_time.c
endif
if BUILD_LINUX
libccore_la_SOURCES += src/ccore/linux/interface/lin_gamepad.c

libccore_la_SOURCES += src/ccore/linux/utils/lin_file.c
libccore_la_SOURCES += src/ccore/linux/utils/lin_sysinfo.c
libccore_la_SOURCES += src/ccore/linux/utils/lin_thread.c
libccore_la_SOURCES += src/ccore/linux/utils/lin_time.c
endif
if BUILD_X11
libccore_la_SOURCES += src/ccore/x11/interface/x11_display.c
libccore_la_SOURCES += src/ccore/x11/interface/x11_opengl.c
#libccore_la_SOURCES += src/ccore/x11/interface/x11_text.c
libccore_la_SOURCES += src/ccore/x11/interface/x11_window.c
endif

if HAVE_CHECK
check_PROGRAMS = check_ccore
check_ccore_SOURCES = $(top_builddir)/test/icon.c $(top_builddir)/test/test.c
check_ccore_CPPFLAGS = $(AM_CPPFLAGS) -D_DEBUG
check_ccore_CFLAGS = @CHECK_CFLAGS@
check_ccore_LDADD = @CHECK_LIBS@ $(top_builddir)/.libs/libccore.la

TESTS = check_ccore
endif
