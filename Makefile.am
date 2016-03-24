pkginclude_HEADERS = include/ccore/*.h

lib_LIBRARIES = libccore.a
libccore_a_SOURCES = src/ccore/common/interface/com_display.c
libccore_a_SOURCES += src/ccore/common/interface/com_event.c
libccore_a_SOURCES += src/ccore/common/interface/com_window.c

libccore_a_SOURCES += src/ccore/common/utils/com_core.c
libccore_a_SOURCES += src/ccore/common/utils/com_error.c
libccore_a_SOURCES += src/ccore/common/utils/com_file.c
libccore_a_SOURCES += src/ccore/common/utils/com_rect.c
libccore_a_SOURCES += src/ccore/common/utils/com_string.c
libccore_a_SOURCES += src/ccore/common/utils/com_sysinfo.c

if BUILD_WINDOWS
libccore_a_SOURCES += src/ccore/windows/interface/win_display.c
libccore_a_SOURCES += src/ccore/windows/interface/win_gamepad.c
libccore_a_SOURCES += src/ccore/windows/interface/win_opengl.c
libccore_a_SOURCES += src/ccore/windows/interface/win_window.c

libccore_a_SOURCES += src/ccore/windows/utils/win_file.c
libccore_a_SOURCES += src/ccore/windows/utils/win_sysinfo.c
libccore_a_SOURCES += src/ccore/windows/utils/win_thread.c
libccore_a_SOURCES += src/ccore/windows/utils/win_time.c
endif
if BUILD_LINUX
libccore_a_SOURCES += src/ccore/linux/interface/lin_gamepad.c

libccore_a_SOURCES += src/ccore/linux/utils/lin_file.c
libccore_a_SOURCES += src/ccore/linux/utils/lin_sysinfo.c
libccore_a_SOURCES += src/ccore/linux/utils/lin_thread.c
libccore_a_SOURCES += src/ccore/linux/utils/lin_time.c
endif
if BUILD_X11
libccore_a_SOURCES += src/ccore/x11/interface/x11_display.c
libccore_a_SOURCES += src/ccore/x11/interface/x11_opengl.c
libccore_a_SOURCES += src/ccore/x11/interface/x11_text.c
libccore_a_SOURCES += src/ccore/x11/interface/x11_window.c
endif

bin_PROGRAMS = test
test_DEPENDENCIES = libccore_a
test_CPPFLAGS = $(AM_CPPFLAGS) -D_DEBUG
test_SOURCES = test/icon.c test/test.c
test_LDADD = libccore.a
