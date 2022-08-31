##
 # configure.ac
 # Copyright (C) 2015-2019 Peter Belkner <info@pbelkner.de>
 # This program is free software; you can redistribute it and/or
 # modify it under the terms of the GNU General Public
 # License as published by the Free Software Foundation; either
 # version 2.0 of the License, or (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # General Public License for more details.
 #
 # You should have received a copy of the GNU General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 # MA  02110-1301  USA
 ##
# http://www.freesoftwaremagazine.com/books/autotools_a_guide_to_autoconf_automake_libtool
# https://autotools.io/autoconf/arguments.html
# http://www.gnu.org/software/libidn/manual/html_node/Autoconf-tests.html
# http://www.clearchain.com/blog/posts/autotools
#
# AC_ARG_WITH (option-name, help-string, action-if-present, action-if-not-present)
# AC_CHECK_HEADER (header-file, [action-if-found], [action-if-not-found], [includes = `default-includes'])
# AC_CHECK_LIB (library, function, [action-if-found], [action-if-not-found], [other-libraries])

AC_INIT([bs1770gain], [0.8.3], [info@pbelkner.de], [], [http://pbelkner.de/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR()
AC_PROG_CC
AC_PROG_RANLIB

unset withval
AC_ARG_ENABLE(dynload, AS_HELP_STRING([--enable-dynload],
		[load FFmpeg's shared objects ("*.so", "*.dylib", or "*.dll") dynamically (requires option --with-ffmpeg)]),
    dynload=yes, dynload=no)

AM_CONDITIONAL([FF_DYNLOAD], [test x$dynload = xyes ])

unset withval
AC_ARG_WITH(ffmpeg, AS_HELP_STRING([--with-ffmpeg=[DIR]],
		[path to FFmpeg installation (i.e. FFmpeg's installation prefix]),
    ffmpeg=$withval, ffmpeg=no)

if test \( x$dynload = xyes \) -a \( x$ffmpeg = xno \); then
  AC_MSG_ERROR(["option --enable-dynload requires option --with-ffmpeg"])
fi

# AC_CANONICAL_HOST is needed to access the 'host_os' variable    
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_mac=no

# Detect the target system [
case "${host_os}" in # [
  linux*)
    build_linux=yes
    win32='no'
    ;;
  cygwin*|mingw*)
    build_windows=yes
    win32='yes'
    ;;
  darwin*)
    build_mac=yes
    win32='no'
    ;;
  *)
    AC_MSG_ERROR(["OS $host_os is not supported"])
    ;;
esac # ]

AM_CONDITIONAL([WIN32], [test x$win32 = xyes])
AC_MSG_RESULT($win32)
# Detect the target system ]

LIBS_INITIAL=${LIBS}

# test for FFmpeg [
if test "x$ffmpeg" != "xno"; then # [
    LDFLAGS="${LDFLAGS} -L$ffmpeg/lib"
    CPPFLAGS="${CPPFLAGS} -I$ffmpeg/include"
    if test "x$win32" = "xno"; then # [
      export LD_LIBRARY_PATH="$ffmpeg/lib:${LD_LIBRARY_PATH}"
    else # ] [
      export PATH="$ffmpeg/bin:${PATH}"
    fi # ]
    FFMPEG_PREFIX="$ffmpeg"
    AC_SUBST(FFMPEG_PREFIX)
fi # ]

if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavutil/avutil.h,
    AC_CHECK_LIB(avutil, avutil_version, [ffmpeg=yes LIBS="${LIBS} -lavutil"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswscale/swscale.h,
    AC_CHECK_LIB(swscale, swscale_version, [ffmpeg=yes LIBS="${LIBS} -lswscale"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavcodec/avcodec.h,
    AC_CHECK_LIB(avcodec, avcodec_version, [ffmpeg=yes LIBS="${LIBS} -lavcodec"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavformat/avformat.h,
    AC_CHECK_LIB(avformat, avformat_version, [ffmpeg=yes LIBS="${LIBS} -lavformat"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswresample/swresample.h,
    AC_CHECK_LIB(swresample, swresample_version, [ffmpeg=yes LIBS="${LIBS} -lswresample"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libswscale/swscale.h,
    AC_CHECK_LIB(swscale, swscale_version, [ffmpeg=yes LIBS="${LIBS} -lswscale"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libpostproc/postprocess.h,
    AC_CHECK_LIB(postproc, postproc_version, [ffmpeg=yes LIBS="${LIBS} -lpostproc"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" != "xno"; then # [
  AC_CHECK_HEADER(libavfilter/avfilter.h,
    AC_CHECK_LIB(avfilter, avfilter_version, [ffmpeg=yes LIBS="${LIBS} -lavfilter"], ffmpeg=no),
    ffmpeg=no)
fi # ]
if test "x$ffmpeg" == "xno" ; then # [
  AC_MSG_ERROR([FFmpeg not found])
fi # ]
# test for FFmpeg ]

# define avutil major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavutil/avutil.h>

int main() { printf("%u",avutil_version()>>16); return 0; }
_ACEOF

FF_AVUTIL_V="`$tmp`"
AC_SUBST(FF_AVUTIL_V)
AC_DEFINE_UNQUOTED([FF_AVUTIL_V], ["$FF_AVUTIL_V"], [Define to libavutil major version.])
rm -f "$tmp"
# define avutil major version ]

# define avcodec major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavcodec/avcodec.h>

int main() { printf("%u",avcodec_version()>>16); return 0; }
_ACEOF

FF_AVCODEC_V="`$tmp`"
AC_SUBST(FF_AVCODEC_V)
AC_DEFINE_UNQUOTED([FF_AVCODEC_V], ["$FF_AVCODEC_V"], [Define to libavcodec major version.])
rm -f "$tmp"
# define avcodec major version ]

# define avformat major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavformat/avformat.h>

int main() { printf("%u",avformat_version()>>16); return 0; }
_ACEOF

FF_AVFORMAT_V="`$tmp`"
AC_SUBST(FF_AVFORMAT_V)
AC_DEFINE_UNQUOTED([FF_AVFORMAT_V], ["$FF_AVFORMAT_V"], [Define to libavformat major version.])
rm -f "$tmp"
# define avformat major version ]

# define swresample major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libswresample/swresample.h>

int main() { printf("%u",swresample_version()>>16); return 0; }
_ACEOF

FF_SWRESAMPLE_V="`$tmp`"
AC_SUBST(FF_SWRESAMPLE_V)
AC_DEFINE_UNQUOTED([FF_SWRESAMPLE_V], ["$FF_SWRESAMPLE_V"], [Define to libswresample major version.])
rm -f "$tmp"
# define swresample major version ]

# define swscale major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libswscale/swscale.h>

int main() { printf("%u",swscale_version()>>16); return 0; }
_ACEOF

FF_SWSCALE_V="`$tmp`"
AC_SUBST(FF_SWSCALE_V)
AC_DEFINE_UNQUOTED([FF_SWSCALE_V], ["$FF_SWSCALE_V"], [Define to libswscale major version.])
rm -f "$tmp"
# define swscale major version ]

# define postproc major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libpostproc/postprocess.h>

int main() { printf("%u",postproc_version()>>16); return 0; }
_ACEOF

FF_POSTPROC_V="`$tmp`"
AC_SUBST(FF_POSTPROC_V)
AC_DEFINE_UNQUOTED([FF_POSTPROC_V], ["$FF_POSTPROC_V"], [Define to libswscale major version.])
rm -f "$tmp"
# define postproc major version ]

# define avfilter major version [
tmp=`mktemp`

cat << _ACEOF | gcc -xc ${CPPFLAGS} -o "$tmp" - ${LDFLAGS} ${LIBS}
#include <stdio.h>
#include <libavfilter/avfilter.h>

int main() { printf("%u",avfilter_version()>>16); return 0; }
_ACEOF

FF_AVFILTER_V="`$tmp`"
AC_SUBST(FF_AVFILTER_V)
AC_DEFINE_UNQUOTED([FF_AVFILTER_V], ["$FF_AVFILTER_V"], [Define to libavfilter major version.])
rm -f "$tmp"
# define avfilter major version ]

case "${host_os}" in # [
  linux*)
		########
    AVUTIL_SO=libavutil.so.$FF_AVUTIL_V
    SWSCALE_SO=libswscale.so.$FF_SWSCALE_V
    AVCODEC_SO=libavcodec.so.$FF_AVCODEC_V
    AVFORMAT_SO=libavformat.so.$FF_AVFORMAT_V
    SWRESAMPLE_SO=libswresample.so.$FF_SWRESAMPLE_V
    SWSCALE_SO=libswscale.so.$FF_SWSCALE_V
    POSTPROC_SO=libpostproc.so.$FF_POSTPROC_V
    AVFILTER_SO=libavfilter.so.$FF_AVFILTER_V
    ;;
  cygwin*|mingw*)
		########
  	AVUTIL_SO=avutil-$FF_AVUTIL_V.dll
  	SWSCALE_SO=swscale-$FF_SWSCALE_V.dll
  	AVCODEC_SO=avcodec-$FF_AVCODEC_V.dll
  	AVFORMAT_SO=avformat-$FF_AVFORMAT_V.dll
  	SWRESAMPLE_SO=swresample-$FF_SWRESAMPLE_V.dll
  	SWSCALE_SO=swscale-$FF_SWSCALE_V.dll
  	POSTPROC_SO=postproc-$FF_POSTPROC_V.dll
  	AVFILTER_SO=avfilter-$FF_AVFILTER_V.dll
    ;;
  darwin*)
		########
    AVUTIL_SO=libavutil.$FF_AVUTIL_V.dylib
    SWSCALE_SO=libswscale.$FF_SWSCALE_V.dylib
    AVCODEC_SO=libavcodec.$FF_AVCODEC_V.dylib
    AVFORMAT_SO=libavformat.$FF_AVFORMAT_V.dylib
    SWRESAMPLE_SO=libswresample.$FF_SWRESAMPLE_V.dylib
    SWSCALE_SO=libswscale.$FF_SWSCALE_V.dylib
    POSTPROC_SO=libpostproc.$FF_POSTPROC_V.dylib
    AVFILTER_SO=libavfilter.$FF_AVFILTER_V.dylib
    ;;
  *)
    AC_MSG_ERROR(["OS $host_os is not supported"])
    ;;
esac # ]

# test for dynload [
if test "x$dynload" = "xyes"; then # [
  # revert LIBS to it's initial state
  # why??? because when loading ffmpeg libs dynamically we don't want to link
  # them! instead we have a special module doing the necessary forwarding.
  LIBS=${LIBS_INITIAL}
  AC_DEFINE([HAVE_FF_DYNLOAD], [1],
      [Define to 1 if FFmpeg should be loaded dynamically.])
  # test for dl [
  if test "xyes" != "x$win32"; then # [
    unset withval
    AC_ARG_WITH(dl, AS_HELP_STRING([--with-dl=[DIR]],
        [path to libdl installation]), dl=$withval, dl=yes)
    if test "x$dl" != "xno"; then # [
      if test "x$dl" != "xyes"; then # [
        LDFLAGS="${LDFLAGS} -L$dl/lib"
        CPPFLAGS="${CPPFLAGS} -I$dl/include"
      fi # ]
    fi # ]
    if test "x$dl" != "xno"; then # [
      AC_CHECK_HEADER(dlfcn.h,
        AC_CHECK_LIB(dl, dlopen, [dl=yes LIBS="${LIBS} -ldl"], dl=no),
        dl=no)
    fi # ]
    if test "x$dl" == "xno" ; then # [
      AC_MSG_ERROR([libdl not found])
    fi # ]
  fi # ]
  # test for dl ]
  AC_MSG_NOTICE([loading FFmpeg dynamically])
else # ] [
  AC_MSG_NOTICE([linking FFmpeg])
fi # ]
# test for dynload ]

if test "x$win32" = "xyes"; then # [
  unset withval
  AC_ARG_WITH(getoptw,
      AS_HELP_STRING([--with-getoptw=[DIR]],[path to getoptW installation]),
      getoptw=$withval, getoptw=yes)
  if test "x$getoptw" != "xno"; then # [
    if test "x$getoptw" != "xyes"; then # [
      LDFLAGS="${LDFLAGS} -L$getoptw/lib"
      CPPFLAGS="${CPPFLAGS} -I$getoptw/include"
#      if test "$win32" = "xno"; then # [
#        export LD_LIBRARY_PATH="$getoptw/lib:${LD_LIBRARY_PATH}"
#      else # ] [
#        export PATH="$getoptw/bin:${PATH}"
#      fi # ]
      GETOPTW_PREFIX="$getoptw"
      AC_SUBST(GETOPTW_PREFIX)
    fi # ]
  fi # ]
  if test "x$getoptw" != "xno"; then # [
    AC_CHECK_HEADER(getoptW.h,
      AC_CHECK_LIB(getoptW,
          getoptW_long,
          [getoptw=yes LIBS="${LIBS} -lgetoptW"],
          getoptw=no),
      getoptw=no,
         [[#ifdef HAVE_GETOPTW_H
         # include <getoptW.h>
         #endif
         ]])
  else # ] [
    AC_MSG_ERROR([getoptW not found])
  fi # ]
fi # ]

# dealing with shared objects [
if test "x$win32" = "xyes"; then # [
  SODIR='bin'
  # not needed:
  #LDFLAGS=" ${LDFLAGS} -static-libgcc -static-libstdc++"
  # -mwindows for linking a windows application, i.e. WinMain().
  # -municode for linking wmain() instead of main,() i.e. an unicode
  #   application. Just -municode is needed:
  #LDFLAGS=" ${LDFLAGS} -mwindows -municode"
  LDFLAGS=" ${LDFLAGS} -municode"
else # ] [
  SODIR='lib'
  if test "x$build_mac" = "xyes"; then # [
    AC_CHECK_HEADER(libproc.h, AC_CHECK_LIB(proc, proc_pidpath,
      [proc=yes LIBS="${LIBS} -lproc"], proc=no), proc=no)

    if test "x$proc" == "xno" ; then # [
      AC_MSG_ERROR([libproc not found])
    else # ] [
      AC_DEFINE([HAVE_LIBPROC], [1], [Define to 1 if you have libproc.])
    fi # ]
  fi # ]

  if test "x$dynload" = "xyes"; then # [
    LIBS="${LIBS} -ldl"
  fi # ]

  LIBS="${LIBS} -lm"
  #############################################################################
#  LIBS=" ${LIBS} -pthread"
  AC_CHECK_HEADER(pthread.h,
    AC_CHECK_LIB(pthread, pthread_create, [pthread=yes LIBS="${LIBS} -pthread"], pthread=no),
    pthread=no)
  if test "x$pthread" == "xno" ; then # [
    AC_MSG_NOTICE([*******************************************************************])
    AC_MSG_NOTICE([* pthread not found.                                              *])
    AC_MSG_NOTICE([* bs10gain will be build without support for parallel processing. *])
    AC_MSG_NOTICE([*******************************************************************])
    AC_DEFINE([HAVE_PTHREAD], [0],
        [Define to 1 if you have pthreads.])
  else # ] [
    AC_DEFINE([HAVE_PTHREAD], [1],
        [Define to 1 if you have pthreads.])
  fi # ]
fi # ]

AC_SUBST(SODIR)
AC_SUBST(AVUTIL_SO)
AC_SUBST(SWRESAMPLE_SO)
AC_SUBST(SWSCALE_SO)
AC_SUBST(AVCODEC_SO)
AC_SUBST(AVFORMAT_SO)
AC_SUBST(POSTPROC_SO)
AC_SUBST(AVFILTER_SO)
AC_SUBST(BS1770GAIN_LDFLAGS)

#XX AC_MSG_NOTICE([$dynload])
#XX if test "x$dynload" = "xyes"; then # [
#XX mkdir -p ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$AVUTIL_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$SWRESAMPLE_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$SWSCALE_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$AVCODEC_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$AVFORMAT_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$POSTPROC_SO ./libbg/bs1770gain-tools
#XX cp -p $FFMPEG_PREFIX/$SODIR/$AVFILTER_SO ./libbg/bs1770gain-tools
#XX fi # ]

#CPPFLAGS="-DDEBUG ${CPPFLAGS}"
#CFLAGS="-g ${CFLAGS}"

CFLAGS="-Werror ${CFLAGS}"
CFLAGS="-Wall ${CFLAGS}"
CFLAGS="-Wformat ${CFLAGS}"
CFLAGS="-Wpointer-arith ${CFLAGS}"
CFLAGS="-Wcast-align ${CFLAGS}"
CFLAGS="-Wcast-qual ${CFLAGS}"
CFLAGS="-Wreturn-type ${CFLAGS}"
CFLAGS="-Wextra ${CFLAGS}"
CFLAGS="-Wstrict-prototypes ${CFLAGS}"

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  libpbutil/Makefile
  lib1770-2/Makefile
  libff/Makefile
  libbg/Makefile
])
AC_OUTPUT
