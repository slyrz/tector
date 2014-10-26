#-----------------------------------------------------------------------------
# Autoconf Requirements
#-----------------------------------------------------------------------------
AC_PREREQ(2.61)
AC_INIT([tector],[0.1])
AC_LANG([C])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([src/core/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])
AM_MAINTAINER_MODE([enable])

m4_include([m4/ax_compiler_vendor.m4])
m4_include([m4/ax_check_compile_flag.m4])

#-----------------------------------------------------------------------------
# Define Package Options
#-----------------------------------------------------------------------------
AC_ARG_ENABLE([debugging], AS_HELP_STRING([--enable-debugging], [compile with debugging functionality]), [
  AS_IF([test "x$enable_debugging" = "xyes"], [debugging="yes"], [debugging="no"])
], [debugging="no"])

AM_CONDITIONAL([DEBUGGING], [test "x$debugging" = "xyes"])

#-----------------------------------------------------------------------------
# Checks For Compiler And Friends
#-----------------------------------------------------------------------------
AC_PROG_CC([gcc clang])
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
AC_GNU_SOURCE

AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

#-----------------------------------------------------------------------------
# Libraries
#---------.-------------------------------------------------------------------
AC_SEARCH_LIBS([expf],[m])

#-----------------------------------------------------------------------------
# Checks For Compiler Characteristics
#-----------------------------------------------------------------------------
AC_C_INLINE
AC_C_RESTRICT

AS_IF([test "x$debugging" = "xyes"], [
  CFLAGS=`echo "-g $CFLAGS" | sed -e "s/-O[0-9]//"`
], [
  CFLAGS=""
  AX_CHECK_COMPILE_FLAG([-march=native], [ CFLAGS="-march=native $CFLAGS" ])
  AX_CHECK_COMPILE_FLAG([-Ofast], [ CFLAGS="-Ofast $CFLAGS" ])
])

#-----------------------------------------------------------------------------
# End
#-----------------------------------------------------------------------------
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        ==========

        prefix: ............... ${prefix}
        sysconfdir: ........... ${sysconfdir}
        libdir: ............... ${libdir}
        includedir: ........... ${includedir}

        compiler: ............. ${CC}
        cflags: ............... ${CFLAGS}

        debugging: ............ ${debugging}
])
