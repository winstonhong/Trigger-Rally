#! /bin/sh

# Correct working directory?
if test ! -f configure.ac ; then
  echo "*** Please invoke this script from directory containing configure.ac."
  exit 1
fi

# construct aclocal.m4 from macro files
MACRODIR=mk/autoconf
aclocal -I $MACRODIR

# generate a Jamconfig.in
autoconf --trace=AC_SUBST | \
  sed -e 's/configure.ac:[0-9]*:AC_SUBST:\([^:]*\).*/\1 ?= "@\1@" ;/g' \
  > Jamconfig.in

# seems autoconf --trace misses some things :-/
echo 'INSTALL ?= "@INSTALL@" ;' >> Jamconfig.in
echo 'JAMCONFIG_READ = yes ;' >> Jamconfig.in

# autoheader/config.h not used by trigger yet
autoheader

autoconf
