
WLINC=../../patches/kdrivers/include
WINC=/usr/include/wanpipe

AM_CFLAGS		= -fPIC -Wall -Wstrict-prototypes -Wmissing-prototypes -g -I$(WLINC) -I$(WINC)  -I. \
	-D__LINUX__ -D_REENTRANT -D_GNU_SOURCE -O2 -D_DEBUG_=2 -D_GNUC_ -I../lib  -D__COMPILING_LIBSANGOMA__ 

# pedantic flags that will help us to keep clean code
# I feel sorry for the coders who like messy-warnings-all-over-the-place code
AM_CFLAGS += -Wall -Wwrite-strings -Wunused-variable -Wstrict-prototypes -Wmissing-prototypes

#ACLOCAL_AMFLAGS = -I m4 

LIB_SOURCES		= libsangoma.c libsangoma_utils.c libsangoma_hwec.c libsangoma.h libhpsangoma.c libhpsangoma.h libhpsangoma_priv.c libhpsangoma_priv.h

if LIBPRI
LIB_SOURCES		+= sangoma_pri.c sangoma_pri.h
endif 

library_includedir      = $(includedir)
library_include_HEADERS = libsangoma.h libhpsangoma.h 

if LIBPRI
library_include_HEADERS	+= sangoma_pri.h
AM_CFLAGS               += -I@LIBPRI_PATH@
endif

lib_LTLIBRARIES         = libsangoma.la
libsangoma_la_CFLAGS    = $(AM_CFLAGS)
libsangoma_la_LDFLAGS   = -version-info @LIBSANGOMA_LT_CURRENT@:@LIBSANGOMA_LT_REVISION@:@LIBSANGOMA_LT_AGE@

if LIBPRI
libsangoma_la_CFLAGS	+= -I$(libpripath)
libsangoma_la_LDFLAGS	+= -L$(libpripath) -lpri
endif 

libsangoma_la_SOURCES   = $(LIB_SOURCES)

lib_LIBRARIES           = libsangoma.a
libsangoma_a_CFLAGS     = $(AM_CFLAGS)
libsangoma_a_SOURCES    = $(LIB_SOUCES)
