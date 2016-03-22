#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([README.md])
AM_INIT_AUTOMAKE([ccore], [0.1])

AC_CONFIG_HEADER([include/core.h])
AC_CONFIG_SRCDIR([src/ccore/common/interface/com_display.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([m], [cos], [], [echo "Math library is required" && exit -1])
AC_CHECK_LIB([GL], [glXChooseVisual], [], [echo "OpenGL dev library is required" && exit -1])
AC_CHECK_LIB([X11], [XOpenDisplay], [], [echo "X11 dev library is required" && exit -1])
AC_CHECK_LIB([Xrandr], [XRRGetScreenResources], [], [echo "XRandr dev library is required" && exit -1])
AC_CHECK_LIB([Xinerama], [XineramaQueryExtension], [], [echo "Xinerama dev library is required" && exit -1])
AC_CHECK_LIB([Xi], [XISelectEvents], [], [echo "XInput dev library is required" && exit -1])
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

AC_ARG_ENABLE(gamepad, AC_HELP_STRING([--enable-gamepad], [Enable the gamepad module [[default=yes]]]),, enable_gamepad=yes)
if test x$enable_gamepad == xyes; then
    SUMMARY_modules="${SUMMARY_modules} gamepad"
    AC_DEFINE(CC_USE_GAMEPAD, 1, [ ])
fi

AC_ARG_ENABLE(time, AC_HELP_STRING([--enable-time], [Enable the timing module [[default=yes]]]),, enable_time=yes)
if test x$enable_time == xyes; then
    SUMMARY_modules="${SUMMARY_modules} time"
    AC_DEFINE(CC_USE_TIME, 1, [ ])
fi

AC_ARG_ENABLE(thread, AC_HELP_STRING([--enable-thread], [Enable the threading module [[default=yes]]]),, enable_thread=yes)
if test x$enable_thread == xyes; then
    SUMMARY_modules="${SUMMARY_modules} thread"
    AC_DEFINE(CC_USE_THREAD, 1, [ ])
fi

AC_ARG_ENABLE(sysinfo, AC_HELP_STRING([--enable-sysinfo], [Enable the system information module [[default=yes]]]),, enable_sysinfo=yes)
if test x$enable_sysinfo == xyes; then
    SUMMARY_modules="${SUMMARY_modules} sysinfo"
    AC_DEFINE(CC_USE_GAMEPAD, 1, [ ])
fi

AC_OUTPUT
