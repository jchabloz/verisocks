#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([verisocks], [0.0.1])
AC_CONFIG_SRCDIR([include/vs_msg.h])
AC_CONFIG_HEADERS([include/vpi_config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
${CFLAGS=""}
AC_PROG_CC
AC_PROG_SED

# Checks for libraries.
# If libvpi exists and has the symbol, it needs to be used by the linker
AC_CHECK_LIB([vpi], [vpi_register_systf])

AC_SEARCH_LIBS([pow],[m])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h locale.h malloc.h math.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h],,[AC_MSG_ERROR([Required header file missing])])
AC_CHECK_HEADERS([vpi_user.h],,[
	AC_CHECK_HEADERS([iverilog/vpi_user.h],,[
		AC_MSG_ERROR([Could not find a valid vpi_user.h header file])
	])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gethostbyaddr gethostbyname localeconv memset pow select socket sqrt strcasecmp strdup strerror strrchr strstr],[],[AC_MSG_ERROR([Required function test failed])])

AC_OUTPUT

