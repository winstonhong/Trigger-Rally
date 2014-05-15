#----------------------------------------------------------------------------
# Autoconf input script. Start the ./autgen.sh script for producing
# the configure script.
#----------------------------------------------------------------------------
AC_PREREQ([2.54])

AC_INIT([trigger], [0.6.0], [jas@jareiko.net])
AC_CONFIG_SRCDIR([src/trigger/main.cpp])
AC_CONFIG_AUX_DIR([mk/autoconf])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#----------------------------------------------------------------------------
# Configuration header
#----------------------------------------------------------------------------
AC_CONFIG_HEADERS(config.h)

# stupid autoconf is adding default -g -O2 flags which we don't want :-/
test ".$CFLAGS" = "." && CFLAGS=" "
test ".$CXXFLAGS" = "." && CXXFLAGS=" "

#----------------------------------------------------------------------------
# Check for operating system
#----------------------------------------------------------------------------
AC_MSG_CHECKING([for target host])
case $host_os in 
	mingw*|cygwin*)
	AC_MSG_RESULT(WIN32)
	#AC_DEFINE([WIN32], [], [define when compiling for a win32 system])
	#actually don't define it... gcc does automatically for w32 host
	;;
	*)
	AC_MSG_RESULT([assuming unix])
	AC_DEFINE([UNIX], [], [define when compiling for a unix/posix system])
	;;
esac

#----------------------------------------------------------------------------
# Check for build variant (debug, profile, optimize)
#----------------------------------------------------------------------------
VARIANT=optimize
AC_ARG_ENABLE([optimize], [AC_HELP_STRING([--enable-optimize],
	[build with optimizations enabled (default YES)])],
	[test "$enableval" = "yes" && VARIANT=optimize])
																				
AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
	[build with debugging information (default NO)])],
	[test "$enableval" = "yes" && VARIANT=debug])
																				
AC_ARG_ENABLE([profile], [AC_HELP_STRING([--enable-profile],
	[build with profiling information (default NO)])],
	[test "$enableval" = "yes" && VARIANT=profile])
AC_SUBST([VARIANT])

#----------------------------------------------------------------------------
# find applications
#----------------------------------------------------------------------------
AC_PROG_CXX
AC_PROG_INSTALL

#----------------------------------------------------------------------------
# find libraries
#----------------------------------------------------------------------------

AM_PATH_SDL([1.2.5], , AC_MSG_ERROR([Please install libsdl >= 1.2.5]))

AX_FINDLIB([SDLIMAGE], [SDL_image], [SDL_image >= 1.2],
	AX_LANG_PROGRAM([#include "SDL_image.h"], [IMG_Load("");]),
	[], [-lSDL_image],
	[],
	[AC_MSG_ERROR([Please install SDLImage >= 1.2])],
	[$SDL_CFLAGS], [$SDL_LIBS])


#AX_FINDLIB([AUDIOLIB], [SDL_mixer], [SDL_mixer >= 1.2.1],
#	AX_LANG_PROGRAM([#include "SDL_mixer.h"], [Mix_OpenAudio(0,0,0,0);]),
#	[], [-lSDL_mixer],
#	[],
#	[AC_MSG_ERROR([Please install SDL_mixer >= 1.2.1])],
#	[$SDL_CFLAGS], [$SDL_LIBS])

case $host_os in
	
	# WIN32 system
	mingw*|cygwin*)
	
	AX_FINDLIB([AUDIOLIB], [FMOD], [FMOD],
		AX_LANG_PROGRAM([#include <fmod.h>], [FSOUND_Init(0,0,0);]),
		[], [-lfmod],
		[],
		[AC_MSG_ERROR([Please install FMOD])])
	
	;;
	
	# assume UNIX system
	*)
	
	AX_FINDLIB([AUDIOLIB], [Alut], [Alut],
		AX_LANG_PROGRAM([#include <AL/alut.h>], [alutInit(0,0);]),
		[], [-lopenal -lalut],
		[],
		[AC_MSG_ERROR([Please install FreeALUT/OpenAL])])
	
	;;
	
esac

AX_FINDLIB([PHYSFS], [PhysFS], [PhysFS],
	AX_LANG_PROGRAM([#include <physfs.h>], [PHYSFS_init(0);]),
	[], [-lphysfs],
	[],
	[AC_MSG_ERROR([Please install PhysFS])])

AX_CHECK_GL
if test "x$GL_CFLAGS" = "x" -a "x$GL_LIBS" = "x"; then
	AC_MSG_ERROR([Couldn't find OpenGL library and headers])
fi

AX_CHECK_GLU
if test "x$GLU_CFLAGS" = "x" -a "x$GLU_LIBS" = "x"; then
	AC_MSG_ERROR([Couldn't find GLU library and headers])
fi

# Um, can't get this to substitute [app]datadir :/
#AC_DEFINE([DATADIR], [$(datadir)], [comment])


# do jam specific fixes
AC_INIT_JAM

# finally output Jamconfig
AC_CONFIG_FILES([Jamconfig])
AC_OUTPUT

# a notice for the users
AC_MSG_NOTICE([

Please note that this project uses jam (and not make) as build tool.
])

