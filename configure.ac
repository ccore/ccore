#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ccore], m4_esyscmd_s([cat VERSION]))

AC_CONFIG_AUX_DIR([build-scripts])

CCORE_VERSION="$(cat VERSION)"
AC_SUBST(CCORE_VERSION)

# Set the programming language to 'C'
AC_LANG_C

AM_INIT_AUTOMAKE([foreign subdir-objects nostdinc silent-rules -Wall -Werror])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([disable])
AM_PROG_AR

# Initialize libtool
LT_INIT([win32-dll])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_CHECK_TOOL(WINDRES, [windres], [:])

# Checks for the test library
PKG_CHECK_MODULES([CHECK], [check >= 0.8.2], [have_check=yes], [have_check=no])

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC 
AC_CHECK_FUNCS([clock_gettime memset select strdup sysinfo])

# Check if we want to build with Gtk3 instead of X11
AC_ARG_ENABLE([gtk3], [AS_HELP_STRING([--enable-gtk3], [compile against GTK3 [default=auto]])], [enable_gtk3=$enableval],	[enable_gtk3=auto])

build_linux=no
build_windows=no
build_x11=no
build_gtk3=no
build_opengl=no

build_examples=yes

SUMMARY="Configuration summary\n"
SUMMARY="${SUMMARY}ccore version: ${CCORE_VERSION}\n"

case "$host" in
	*-*-cygwin*)
		# We build SDL on cygwin without the UNIX emulation layer
		BASE_CFLAGS="-I/usr/include/mingw -mno-cygwin"
		BASE_LDFLAGS="-mno-cygwin"
		;;
	*)
		BASE_CFLAGS="-D_GNU_SOURCE=1"
		;;
esac

EXTRA_LDFLAGS="$BASE_LDFLAGS"
EXTRA_LIBS="$BASE_LIBS"
EXTRA_CFLAGS="$BASE_CFLAGS"
EXTRA_CPPFLAGS="-I$srcdir/include"

CheckX11()
{
	AC_CHECK_LIB([X11], [XOpenDisplay], [], [echo "X11 dev library is required" && exit -1])
	AC_CHECK_LIB([Xrandr], [XRRGetScreenResources], [], [echo "XRandr dev library is required" && exit -1])
	AC_CHECK_LIB([Xinerama], [XineramaQueryExtension], [], [echo "Xinerama dev library is required" && exit -1])
	AC_CHECK_LIB([Xi], [XISelectEvents], [], [echo "XInput dev library is required" && exit -1])
	AC_CHECK_LIB([Xext], [XShmCreateImage], [], [echo "Xshm (Xext) dev library is required" && exit -1])
	AC_PATH_X

	EXTRA_CFLAGS="${EXTRA_CFLAGS} $X_CFLAGS"
	EXTRA_LDFLAGS="${EXTRA_LDFLAGS} $X_LIBS"

	build_x11=yes
	SUMMARY="${SUMMARY}Using X11 to handle window and input functions\n"
}

CheckGTK3()
{
	AC_CHECK_LIB([gtk-3], [gtk_application_new], [], [echo "GTK3 dev library is required" && exit -1])
	PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.16], [], [echo "GTK3 dev library is the wrong version (version 3.16 or higher is required)" && exit -1])

	EXTRA_CFLAGS="${EXTRA_CFLAGS} $GTK_CFLAGS"
	EXTRA_LIBS="${EXTRA_LIBS} $GTK_LIBS"

	build_gtk3=yes
	SUMMARY="${SUMMARY}Using GTK3 to handle window and input functions\n"
}

CheckOpenGL()
{
	AC_CHECK_LIB([m], [cos], [], [echo "Math library is required" && exit -1])
	AC_CHECK_LIB([GL], [glXChooseVisual], [], [echo "OpenGL dev library is required" && exit -1])

	build_opengl=yes
	SUMMARY="${SUMMARY}Using OpenGL\n"
}

CheckWin32()
{
	AC_MSG_CHECKING(Win32 compiler compatibility)

	win32_can_compile=no
	AC_TRY_COMPILE([
		#include <windows.h>
	], [], [win32_can_compile=yes])
	if test x$win32_can_compile != xyes; then
		AC_MSG_ERROR([The compiler ($CC) could not build the windows header files])
	fi

	AC_MSG_RESULT(yes)
}

# AC_CANONICAL_HOST is needed to acces the 'host_os' variable
AC_CANONICAL_HOST

SUMMARY="${SUMMARY}Target host: ${host_os}\n"
case "${host_os}" in
	linux*)
		# Check Linux headers & libraries
		AC_CHECK_HEADERS([sys/ioctl.h sys/param.h sys/types.h unistd.h])
		AC_CHECK_LIB([pthread], [pthread_create], [], [echo "Pthread dev library is required" && exit -1])

		if test x$enable_gtk3 = xyes; then
			CheckGTK3
		else
			CheckX11
		fi
		CheckOpenGL

		build_linux=yes
		;;
	mingw* | cygwin*)
		AC_FUNC_ALLOCA

		CheckWin32

		EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -static-libgcc -no-undefined -version-info ${CCORE_VERSION}"
		EXTRA_LIBS="${EXTRA_LIBS} -mwindows -lmingw32 -lws2_32 -luser32 -lgdi32 -lwinmm"

		build_windows=yes
		;;
	*)
		AC_MSG_ERROR(["OS $host_os is not supported"])
		;;
esac

# Add the include flag so the header files can read ccore/*.h

# Pass the building booleans to Makefile.am
AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_X11], [test "$build_x11" = "yes"])
AM_CONDITIONAL([BUILD_GTK3], [test "$build_gtk3" = "yes"])
AM_CONDITIONAL([BUILD_OPENGL], [test "$build_opengl" = "yes"])

AM_CONDITIONAL([BUILD_TESTS], [test "$have_check" = "yes"])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "$build_examples" = "yes"])

if test x$have_check != xyes; then
	AC_DEFINE([HAVE_CHECK], [1], [Use check])
	SUMMARY="${SUMMARY}Not building tests (test/test.c) because the 'check' package could not be found\n"
	SUMMARY="${SUMMARY}Please install the check package to be able to run the tests\n"
fi

# The library automatically includes all modules, change this if it is unwanted
AC_ARG_ENABLE(all-modules, AC_HELP_STRING([--disable-all-modules], [Disable all ccore modules (not recommeded)]))
if test x$disable_all_modules != xyes; then
	EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -DCC_USE_ALL"
	SUMMARY="${SUMMARY}Enabling all ccore modules (#define CC_USE_ALL)\n"
fi
# Check if we want to build ccore with the debug flag
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [compile with assertions and other debug options [default=auto]])], [enable_debug=$enableval],	[enable_debug=auto])
if test x$enable_debug = xyes; then
	EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -D_DEBUG"
	SUMMARY="${SUMMARY}Building the library with assertions and other options (#define _DEBUG)\n"
fi

AC_SUBST(EXTRA_LDFLAGS)
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_CPPFLAGS)

SUMMARY="${SUMMARY}Building the library with the following flags:\n"
SUMMARY="${SUMMARY}- LDFLAGS:\n\t${EXTRA_LDFLAGS}\n"
SUMMARY="${SUMMARY}- LIBS:\n\t${EXTRA_LIBS}\n"
SUMMARY="${SUMMARY}- CPPFLAGS:\n\t${EXTRA_CPPFLAGS}\n"
SUMMARY="${SUMMARY}- CFLAGS:\n\t${EXTRA_CFLAGS}\n"

# Check if the source files are in the right location
AC_CONFIG_SRCDIR([src/ccore/common/interface/com_display.c])
AC_CONFIG_FILES([Makefile])
# Setup m4 folder for libtool
AC_CONFIG_MACRO_DIR([m4])

# Display the output summary and show the ccore logo
AC_CONFIG_COMMANDS([summary], [cat logo.txt ; echo -en "$SUMMARY"], [SUMMARY="$SUMMARY"])

AC_OUTPUT()
