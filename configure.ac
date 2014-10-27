#-----------------------------------------------------------------------------
# Autoconf Requirements
#-----------------------------------------------------------------------------
AC_PREREQ(2.61)
AC_INIT([tector],[0.1])
AC_LANG([C])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([src/core/config.h])

AM_INIT_AUTOMAKE([foreign subdir-objects silent-rules])
AM_MAINTAINER_MODE([enable])

#-----------------------------------------------------------------------------
# Compiler
#-----------------------------------------------------------------------------
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
])
