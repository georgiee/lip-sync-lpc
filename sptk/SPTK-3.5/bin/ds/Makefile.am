## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.3 2007/09/28 03:14:36 s_sako Exp $
INCLUDES = -I$(top_srcdir)/include
lpcoefdir = $(DESTDIR)$(libdir)

bin_PROGRAMS = ds
ds_SOURCES = ds.c 
ds_LDADD = $(top_srcdir)/lib/libSPTK.a
ds_CPPFLAGS = -DLIB=\"$(lpcoefdir)\"

dist_lpcoef_DATA = lpfcoef.2to1 \
	lpfcoef.3to2 \
	lpfcoef.4to3 \
	lpfcoef.5to2dn \
	lpfcoef.5to2up 
