#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ccore], [0.1])

AM_INIT_AUTOMAKE([foreign subdir-objects nostdinc -Wall -Werror])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_AR

AC_LANG_C

# Checks for libraries.
AC_CHECK_LIB([m], [cos], [], [echo "Math library is required" && exit -1])
AC_CHECK_LIB([GL], [glXChooseVisual], [], [echo "OpenGL dev library is required" && exit -1])

# Checks for the test library
have_check=no
PKG_CHECK_MODULES([CHECK], [check >= 0.8.2], [have_check=yes])

# Checks for header files.
AC_HEADER_STDC

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

build_linux=no
build_windows=no
build_x11=no

SUMMARY="Configuration summary:\n"

CheckX11()
{
	# Check for X11 libraries
	AC_CHECK_LIB([X11], [XOpenDisplay], [], [echo "X11 dev library is required" && exit -1])
	AC_CHECK_LIB([Xrandr], [XRRGetScreenResources], [], [echo "XRandr dev library is required" && exit -1])
	AC_CHECK_LIB([Xinerama], [XineramaQueryExtension], [], [echo "Xinerama dev library is required" && exit -1])
	AC_CHECK_LIB([Xi], [XISelectEvents], [], [echo "XInput dev library is required" && exit -1])
	AC_PATH_X

	build_x11=yes
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

	build_windows=yes
}

# AC_CANONICAL_HOST is needed to acces the 'host_os' variable
AC_CANONICAL_HOST
SUMMARY="${SUMMARY}Target host: ${host_os}\n"
case "${host_os}" in
	linux*)
		# Check Linux headers & libraries
		AC_CHECK_HEADERS([sys/ioctl.h sys/param.h sys/types.h unistd.h])
		AC_CHECK_LIB([pthread], [pthread_create], [], [echo "Pthread dev library is required" && exit -1])
		build_linux=yes

		CheckX11
		;;
	mingw*)
		CheckWindows
		;;
	*)
		AC_MSG_ERROR(["OS $host_os is not supported"])
		;;
esac

# Add the include flag so the header files can read ccore/*.h

# Pass the building booleans to Makefile.am
AM_CONDITIONAL([HAVE_CHECK], [test "$have_check" = "yes"])
AM_CONDITIONAL([BUILD_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BUILD_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([BUILD_X11], [test "$build_x11" = "yes"])

# The library automatically includes all modules, change this if it is unwanted
AC_ARG_ENABLE(all-modules, AC_HELP_STRING([--disable-all-modules], [Disable all ccore modules (not recommeded)]))
if test x$disable_all_modules != xyes; then
	AC_SUBST([AM_CPPFLAGS], "-DCC_USE_ALL")
	SUMMARY="${SUMMARY}Enabling all ccore modules (#define CC_USE_ALL)\n"
fi

if test x$have_check != xyes; then
	SUMMARY="${SUMMARY}Not building tests (test/test.c) because the 'check' package could not be found\n"
	SUMMARY="${SUMMARY}Please install the check package to be able to run the tests\n"
fi

# Create a custom configuration header
#AC_CONFIG_HEADER([include/ccore/config.h])
# Check if the source files are in the right location
AC_CONFIG_SRCDIR([src/ccore/common/interface/com_display.c])
AC_CONFIG_FILES([Makefile])
# Create a symlink of libccore.a in the lib folder
#AC_CONFIG_LINKS([lib/libccore.a:libccore.a])

# Display the output summary and show the ccore logo
AC_CONFIG_COMMANDS([summary], [cat logo.txt ; echo -en "$SUMMARY"], [SUMMARY="$SUMMARY"])

AC_OUTPUT
