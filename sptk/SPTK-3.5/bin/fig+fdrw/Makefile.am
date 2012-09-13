## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.1 2007/09/22 17:43:54 s_sako Exp $
INCLUDES = -I$(top_srcdir)/include
bin_PROGRAMS = fig fdrw
fig_SOURCES = fig.c fig.h fig0.c fig1.c plot.c plot.h plsub.c 
fdrw_SOURCES = fdrw.c plot.c plot.h plsub.c 
fig_LDADD = $(top_srcdir)/lib/libSPTK.a
fdrw_LDADD = $(top_srcdir)/lib/libSPTK.a
