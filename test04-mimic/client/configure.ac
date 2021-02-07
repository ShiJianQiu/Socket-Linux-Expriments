#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(client, 1.4, [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([client.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(client,1.4)
# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# pthread
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([pthread is missing])])
PTHREAD_CFLAGS=-pthread
AC_SUBST(PTHREAD_CFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([bzero select socket])

AC_OUTPUT(Makefile)
