## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.4 2008/11/12 05:02:13 s_sako Exp $
INCLUDES = -I$(top_srcdir)/include
bin_PROGRAMS = gmm gmmp
gmm_SOURCES = _gmm.c gmm.c gmm.h
gmm_LDADD = $(top_srcdir)/lib/libSPTK.a
gmmp_SOURCES = _gmm.c gmmp.c gmm.h
gmmp_LDADD = $(top_srcdir)/lib/libSPTK.a
