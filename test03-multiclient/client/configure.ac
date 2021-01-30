#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(client, 1.2, shiyufeng1997@qq.com)
AC_CONFIG_SRCDIR([client.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(client, 1.2)
# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([bzero select socket])

AC_OUTPUT(Makefile)
