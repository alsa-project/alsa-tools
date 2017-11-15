AC_INIT(hdspmixer, 1.11)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

AC_PROG_CXX
AC_PROG_MAKE_SET
AC_STDC_HEADERS
AC_PATH_X

AM_PATH_ALSA(1.0.0)

dnl library check
AC_PATH_PROG(FLTK_CONFIG, fltk-config, no)
if test "$FLTK_CONFIG" = "no" && ! test "`hostname -f`" = "alsa.alsa-project.org"; then
    AC_MSG_ERROR(fltk-config is required)
fi
FLTK_CXXFLAGS="`$FLTK_CONFIG --cxxflags` -fno-exceptions"
FLTK_LIBS="`$FLTK_CONFIG --ldflags`"

CXXFLAGS="$CXXFLAGS $ALSA_CFLAGS $FLTK_CXXFLAGS"
LIBS="$LIBS $ALSA_LIBS $FLTK_LIBS"

AC_OUTPUT(Makefile src/Makefile pixmaps/Makefile desktop/Makefile)
