## Process this file with autoconf to produce a configure script.

## Copyright (C) 2017 David Pinto <david.pinto@bioch.ox.ac.uk>
##
## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

AC_PREREQ([2.69])
AC_INIT([ActionTable], [0.0.0+])
AM_INIT_AUTOMAKE([gnu subdir-objects -Wall -Werror])

## Add the option to enable silent rules and make silent rules the
## default behavior.  Available since Automake 1.11 and included by
## default starting with Automake 1.13.
AM_SILENT_RULES([yes])


AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])


## Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL


## Checks for header files.
AC_CHECK_HEADERS([ \
  fcntl.h \
  stdint.h \
  stdlib.h \
  string.h \
  unistd.h \
], [], [AC_MSG_ERROR([Failed to find and use required header])])


## Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T


## Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([ \
  munmap \
  strerror \
  strtol \
  strtoull \
], [], [AC_MSG_ERROR([Failed to find required function])])



AC_CONFIG_FILES([Makefile])
AC_OUTPUT


## Print a summary so that the user knows everything went well and
## what needs to be done next.
AC_MSG_NOTICE([

$PACKAGE_NAME is now configured for:

  Build:                    $build
  Host:                     $host

  Source directory:         $srcdir
  Installation prefix:      $prefix
  C compiler:               $CC $CFLAGS
  LDFLAGS:                  $LDFLAGS

You can now run `make' to build.
])
