#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ccore], [0.1])

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall -Werror])

AM_PROG_AR

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m], [cos], [], [echo "Math library is required" && exit -1])
AC_CHECK_LIB([GL], [glXChooseVisual], [], [echo "OpenGL dev library is required" && exit -1])
AC_CHECK_LIB([pthread], [pthread_create], [], [echo "Pthread dev library is required" && exit -1])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memset select strdup sysinfo])

SUMMARY="Configuration summary:\n"

CheckX11()
{
	AC_CHECK_LIB([X11], [XOpenDisplay], [], [echo "X11 dev library is required" && exit -1])
	AC_CHECK_LIB([Xrandr], [XRRGetScreenResources], [], [echo "XRandr dev library is required" && exit -1])
	AC_CHECK_LIB([Xinerama], [XineramaQueryExtension], [], [echo "Xinerama dev library is required" && exit -1])
	AC_CHECK_LIB([Xi], [XISelectEvents], [], [echo "XInput dev library is required" && exit -1])

	SOURCES="$SOURCES $srcdir/src/ccore/linux/interface/*.c"

	SUMMARY="${SUMMARY}Using: X11\n"
}

CheckWindows()
{
	AC_MSG_CHECKING(Windows compiler)
	have_win32_gcc=no
	AC_TRY_COMPILE(["#include <windows.h>"],[],[have_win32_gcc=yes])
	AC_MSG_RESULT($have_win32_gcc)
  if test x$have_win32_gcc != xyes; then
  	AC_MSG_ERROR(["*** Your compiler ($CC) does not produce Windows executables!"])
	fi
}

SOURCES="$SOURCES $srcdir/src/ccore/common/interface/*.c"
SOURCES="$SOURCES $srcdir/src/ccore/common/utils/*.c"

AC_CANONICAL_HOST
SUMMARY="${SUMMARY}Target host: ${host_os}\n"
case "${host_os}" in
	linux*)
		SOURCES="$SOURCES $srcdir/src/ccore/linux/interface/*.c"
		SOURCES="$SOURCES $srcdir/src/ccore/linux/utils/*.c"

		CheckX11
		;;
	mingw*)
		CheckWindows
		SOURCES="$SOURCES $srcdir/src/ccore/windows/interface/*.c"
		SOURCES="$SOURCES $srcdir/src/ccore/windows/utils/*.c"
		;;
	*)
		AC_MSG_ERROR(["OS $host_os is not supported"])
		;;
esac

AC_SUBST(SOURCES)

# The library automatically includes all modules, change this if it is unwanted
AC_ARG_ENABLE(all-modules, AC_HELP_STRING([--disable-all-modules], [Disable all ccore modules (not recommeded)]))
if test x$disable_all_modules != xyes; then
	AC_DEFINE(CC_USE_ALL, 1, ["Include all the modules"])
	SUMMARY="${SUMMARY}Enabling all ccore modules (#define CC_USE_ALL)\n"
fi

AC_CONFIG_HEADER([include/ccore/config.h])
AC_CONFIG_SRCDIR([src/ccore/common/interface/com_display.c])
AC_CONFIG_FILES([Makefile])

AC_CONFIG_COMMANDS([summary], [cat logo.txt ; echo -en "$SUMMARY"], [SUMMARY="$SUMMARY"])

AC_OUTPUT
