pkginclude_HEADERS = include/ccore/*.h

lib_LIBRARIES = libccore.a

libccore_a_SOURCES = src/ccore/common/interface/*.c src/ccore/common/utils/*.c
if BUILD_WINDOWS
libccore_a_SOURCES += src/ccore/windows/interface/*.c src/ccore/windows/utils/*.c
endif
if BUILD_LINUX
libccore_a_SOURCES += src/ccore/linux/interface/*.c src/ccore/linux/utils/*.c
endif
if BUILD_X11
libccore_a_SOURCES += src/ccore/x11/interface/*.c
endif

#bin_PROGRAMS = test
#test_SOURCES = test/icon.c test/test.c
#test_LDADD = $(libdir)/libccore.a
