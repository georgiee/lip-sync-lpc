# Copyright 1994-2007 The MathWorks, Inc.
#
# File    : grt_lcc.tmf   $Revision: 1.26.4.21 $
#
# Abstract:
#	Real-Time Workshop template makefile for building a PC-based
#	stand-alone generic real-time version of Simulink model using
#	generated C code and
#			LCC compiler Version 2.4
#
# 	This makefile attempts to conform to the guidelines specified in the
# 	IEEE Std 1003.2-1992 (POSIX) standard. It is designed to be used
#       with GNU Make (gmake) which is located in matlabroot/bin/win32.
#
# 	Note that this template is automatically customized by the Real-Time
#	Workshop build procedure to create "<model>.mk"
#
#       The following defines can be used to modify the behavior of the
#	build:
#	  OPT_OPTS       - Optimization options. Default is none. To enable
#                          debugging specify as OPT_OPTS=-g4.
#	  OPTS           - User specific compile options.
#	  USER_SRCS      - Additional user sources, such as files needed by
#			   S-functions.
#	  USER_INCLUDES  - Additional include paths
#			   (i.e. USER_INCLUDES="-Iwhere-ever -Iwhere-ever2")
#			   (For Lcc, have a '/'as file seperator before the
#			   file name instead of a '\' .
#			   i.e.,  d:\work\proj1/myfile.c - reqd for 'gmake')
#
#       This template makefile is designed to be used with a system target
#       file that contains 'rtwgensettings.BuildDirSuffix' see grt.tlc

#------------------------ Macros read by make_rtw ------------------------------
#
# The following macros are read by the Real-Time Workshop build procedure:
#
#  MAKECMD         - This is the command used to invoke the make utility
#  HOST            - What platform this template makefile is targeted for
#                    (i.e. PC or UNIX)
#  BUILD           - Invoke make from the Real-Time Workshop build procedure
#                    (yes/no)?
#  SYS_TARGET_FILE - Name of system target file.

MAKECMD         = "%MATLAB%\bin\win32\gmake"
SHELL           = cmd
HOST            = PC
BUILD           = yes
SYS_TARGET_FILE = grt.tlc
BUILD_SUCCESS	= *** Created
COMPILER_TOOL_CHAIN = lcc


MAKEFILE_FILESEP = /

#---------------------- Tokens expanded by make_rtw ----------------------------
#
# The following tokens, when wrapped with "|>" and "<|" are expanded by the
# Real-Time Workshop build procedure.
#
#  MODEL_NAME          - Name of the Simulink block diagram
#  MODEL_MODULES       - Any additional generated source modules
#  MAKEFILE_NAME       - Name of makefile created from template makefile <model>.mk
#  MATLAB_ROOT         - Path to were MATLAB is installed.
#  MATLAB_BIN          - Path to MATLAB executable.
#  S_FUNCTIONS         - List of S-functions.
#  S_FUNCTIONS_LIB     - List of S-functions libraries to link.
#  SOLVER              - Solver source file name
#  NUMST               - Number of sample times
#  TID01EQ             - yes (1) or no (0): Are sampling rates of continuous task
#                        (tid=0) and 1st discrete task equal.
#  NCSTATES            - Number of continuous states
#  BUILDARGS           - Options passed in at the command line.
#  MULTITASKING        - yes (1) or no (0): Is solver mode multitasking
#  EXT_MODE            - yes (1) or no (0): Build for external mode
#  TMW_EXTMODE_TESTING - yes (1) or no (0): Build ext_test.c for external mode
#                        testing.
#  EXTMODE_TRANSPORT   - Index of transport mechanism (e.g. tcpip, serial) for extmode
#  EXTMODE_STATIC      - yes (1) or no (0): Use static instead of dynamic mem alloc.
#  EXTMODE_STATIC_SIZE - Size of static memory allocation buffer.


MODEL                = CelpSimulink
MODULES              = AudioDevice_rtw.c dll_utils_win.c fromwavefile_ex_win32.c CelpSimulink_data.c rt_logging.c rt_nonfinite.c 
MAKEFILE             = CelpSimulink.mk
MATLAB_ROOT          = C:/Arquivos de programas/MATLAB/R2008a
ALT_MATLAB_ROOT      = C:/ARQUIV~1/MATLAB/R2008a
MATLAB_BIN           = C:/Arquivos de programas/MATLAB/R2008a/bin
ALT_MATLAB_BIN       = C:/ARQUIV~1/MATLAB/R2008a/bin
S_FUNCTIONS          = 
S_FUNCTIONS_LIB      = 
SOLVER               = 
NUMST                = 1
TID01EQ              = 0
NCSTATES             = 0
BUILDARGS            =  GENERATE_REPORT=0
MULTITASKING         = 0
EXT_MODE             = 0
TMW_EXTMODE_TESTING  = 0
EXTMODE_TRANSPORT    = 0
EXTMODE_STATIC       = 0
EXTMODE_STATIC_SIZE  = 1000000

MODELREFS            = 
SHARED_SRC           = 
SHARED_SRC_DIR       = 
SHARED_BIN_DIR       = 
SHARED_LIB           = 
OPTIMIZATION_FLAGS   = 
ADDITIONAL_LDFLAGS   = 

#--------------------------- Model and reference models -----------------------
MODELLIB                  = CelpSimulinklib.lib
MODELREF_LINK_LIBS        = 
MODELREF_LINK_RSPFILE     = CelpSimulink_ref.rsp
MODELREF_INC_PATH         = 
RELATIVE_PATH_TO_ANCHOR   = ..
# NONE: standalone, SIM: modelref sim, RTW: modelref rtw
MODELREF_TARGET_TYPE       = NONE

#-- In the case when directory name contains space ---
ifneq ($(MATLAB_ROOT),$(ALT_MATLAB_ROOT))
MATLAB_ROOT := $(ALT_MATLAB_ROOT)
endif
ifneq ($(MATLAB_BIN),$(ALT_MATLAB_BIN))
MATLAB_BIN := $(ALT_MATLAB_BIN)
endif

#--------------------------- Tool Specifications ------------------------------

LCC = $(MATLAB_ROOT)\sys\lcc
include $(MATLAB_ROOT)\rtw\c\tools\lcctools.mak

#----------------------------- External mode -----------------------------------

# Uncomment -DVERBOSE to have information printed to stdout
# To add a new transport layer, see the comments in
#   <matlabroot>/toolbox/simulink/simulink/extmode_transports.m
ifeq ($(EXT_MODE),1)
  EXT_CC_OPTS = -DEXT_MODE -DWIN32 # -DVERBOSE
  ifeq ($(EXTMODE_TRANSPORT),0) #tcpip
    EXT_SRC = ext_svr.c updown.c ext_work.c rtiostream_interface.c rtiostream_tcpip.c
    EXT_LIB = $(MATLAB_ROOT)\sys\lcc\lib\wsock32.lib
  endif
  ifeq ($(EXTMODE_TRANSPORT),1) #serial_win32
    EXT_SRC  = ext_svr.c updown.c ext_work.c ext_svr_serial_transport.c
    EXT_SRC += ext_serial_pkt.c ext_serial_win32_port.c
  endif
  ifeq ($(TMW_EXTMODE_TESTING),1)
    EXT_SRC     += ext_test.c
    EXT_CC_OPTS += -DTMW_EXTMODE_TESTING
  endif
  ifeq ($(EXTMODE_STATIC),1)
    EXT_SRC     += mem_mgr.c
    EXT_CC_OPTS += -DEXTMODE_STATIC -DEXTMODE_STATIC_SIZE=$(EXTMODE_STATIC_SIZE)
  endif
endif

#------------------------------ Include Path ----------------------------------

# see MATLAB_INCLUDES and COMPILER_INCLUDES from lcctools.mak

ADD_INCLUDES = \
	-IC:/DOCUME~1/Maquina/MEUSDO~1/Arthur/UFRGS/DSP/CELP/CELPSI~1 \
	-IC:/DOCUME~1/Maquina/MEUSDO~1/Arthur/UFRGS/DSP/CELP \
	-I$(MATLAB_ROOT)/rtw/c/libsrc \
	-I$(MATLAB_ROOT)/toolbox/dspblks/include \
	-I$(MATLAB_ROOT)/toolbox/rtw/dspblks/c \


SHARED_INCLUDES =
ifneq ($(SHARED_SRC_DIR),)
SHARED_INCLUDES = -I$(SHARED_SRC_DIR) 
endif

INCLUDES = -I. -I$(RELATIVE_PATH_TO_ANCHOR) $(MATLAB_INCLUDES) $(ADD_INCLUDES) \
           $(COMPILER_INCLUDES) $(USER_INCLUDES) $(MODELREF_INC_PATH) $(SHARED_INCLUDES)

#------------------------ rtModel ----------------------------------------------
RTM_CC_OPTS = -DUSE_RTMODEL

#-------------------------------- C Flags --------------------------------------

# Optimization Options
OPT_OPTS = $(DEFAULT_OPT_OPTS)

# General User Options
OPTS =

# Compiler options, etc:
ifneq ($(OPTIMIZATION_FLAGS),)
CC_OPTS = $(OPTIMIZATION_FLAGS) $(OPTS) $(ANSI_OPTS) $(EXT_CC_OPTS) $(RTM_CC_OPTS)
else
CC_OPTS = $(OPT_OPTS) $(OPTS) $(ANSI_OPTS) $(EXT_CC_OPTS) $(RTM_CC_OPTS)
endif

CPP_REQ_DEFINES = -DMODEL=$(MODEL) -DRT -DNUMST=$(NUMST) \
                  -DTID01EQ=$(TID01EQ) -DNCSTATES=$(NCSTATES) \
                  -DMT=$(MULTITASKING) -DHAVESTDIO 

CFLAGS = $(CC_OPTS) $(CPP_REQ_DEFINES) $(INCLUDES) -noregistrylookup

ifeq ($(OPT_OPTS),$(DEFAULT_OPT_OPTS))
LDFLAGS = -s -L$(LIB)
else
LDFLAGS = -L$(LIB)
endif


#------------------------- Additional Libraries -------------------------------

LIBS =

ifeq ($(OPT_OPTS),$(DEFAULT_OPT_OPTS))
LIBS += $(MATLAB_ROOT)/toolbox/dspblks/lib/win32\dsp_dyn_rt_lcc.lib
else
LIBS += dsp_dyn_rt.lib
endif 

LIBS += $(EXT_LIB) $(S_FUNCTIONS_LIB)

#----------------------------- Source Files ------------------------------------

ifeq ($(MODELREF_TARGET_TYPE), NONE)
    PRODUCT            = $(RELATIVE_PATH_TO_ANCHOR)/$(MODEL).exe
    BIN_SETTING        = $(LD) $(LDFLAGS) $(ADDITIONAL_LDFLAGS) -o $(PRODUCT) $(SYSLIBS)
    BUILD_PRODUCT_TYPE = executable
    REQ_SRCS           = $(MODEL).c $(MODULES) $(EXT_SRC) \
                         grt_main.c rt_sim.c
else
    # Model reference rtw target
    PRODUCT  = $(MODELLIB)
    REQ_SRCS = $(MODULES)
endif

USER_SRCS =

USER_OBJS       = $(USER_SRCS:.c=.obj)
LOCAL_USER_OBJS = $(notdir $(USER_OBJS))

SRCS      = $(REQ_SRCS) $(S_FUNCTIONS) $(SOLVER)
OBJS      = $(SRCS:.c=.obj)  $(USER_OBJS)
LINK_OBJS = $(SRCS:.c=.obj) $(LOCAL_USER_OBJS)

SHARED_OBJS := $(addsuffix .obj, $(basename $(wildcard $(SHARED_SRC))))
FMT_SHARED_OBJS = $(subst /,\,$(SHARED_OBJS))

#--------------------------------- Rules ---------------------------------------

ifeq ($(MODELREF_TARGET_TYPE),NONE)
$(PRODUCT) : $(OBJS) $(SHARED_LIB) $(LIBS) $(MODELREF_LINK_LIBS)
	$(BIN_SETTING) $(LINK_OBJS) @$(MODELREF_LINK_RSPFILE) $(SHARED_LIB) $(LIBS) 
	@cmd /C "echo $(BUILD_SUCCESS) executable: $(MODEL).exe"
else

$(PRODUCT) : $(OBJS) $(SHARED_LIB)
	$(LIBCMD) /out:$(MODELLIB) $(LINK_OBJS)
	@cmd /C "echo $(BUILD_SUCCESS) static library $(MODELLIB)"
endif

%.obj : %.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(RELATIVE_PATH_TO_ANCHOR)/%.c
	$(CC) -c -Fo$(@F) -I$(RELATIVE_PATH_TO_ANCHOR)/$(<F:.c=cn_rtw) $(CFLAGS)  $<

%.obj : $(MATLAB_ROOT)/rtw/c/grt/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/ext_mode/common/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/ext_mode/tcpip/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/ext_mode/serial/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/ext_mode/custom/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/rtw/c/src/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspacf/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspallpole/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspavi/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspbiquad/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspblms/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspburg/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspchol/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspconvcorr/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspendian/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspeph/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspfbsub/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspfft/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspfilterbank/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspfir/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspg711/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspic/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspiir/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspinterp/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspldl/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsplms/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsplpc2cc/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsplsp2poly/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsplu/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspmmutils/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsppinv/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsppoly2lsf/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsppolyval/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspqrdc/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspqsrt/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsprandsrc/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsprc2ac/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsprc2lpc/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dsprebuff/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspunwrap/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/rtw/dspblks/c/dspvfdly/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

%.obj : $(MATLAB_ROOT)/toolbox/dspblks/include/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<



%.obj : $(MATLAB_ROOT)/simulink/src/%.c
	$(CC) -c -Fo$(@F) $(CFLAGS) $<

# Libraries:



MODULES_dsp_dyn_rt = \
	acf_fd_c_rt.obj \
	acf_fd_d_rt.obj \
	acf_fd_r_rt.obj \
	acf_fd_z_rt.obj \
	acf_fft_interleave_zpad_d_rt.obj \
	acf_fft_interleave_zpad_r_rt.obj \
	acf_td_c_rt.obj \
	acf_td_d_rt.obj \
	acf_td_r_rt.obj \
	acf_td_z_rt.obj \
	copy_and_zero_pad_cc_nchan_rt.obj \
	copy_and_zero_pad_zz_nchan_rt.obj \
	allpole_df_a0scale_cc_rt.obj \
	allpole_df_a0scale_cr_rt.obj \
	allpole_df_a0scale_dd_rt.obj \
	allpole_df_a0scale_dz_rt.obj \
	allpole_df_a0scale_rc_rt.obj \
	allpole_df_a0scale_rr_rt.obj \
	allpole_df_a0scale_zd_rt.obj \
	allpole_df_a0scale_zz_rt.obj \
	allpole_df_cc_rt.obj \
	allpole_df_cr_rt.obj \
	allpole_df_dd_rt.obj \
	allpole_df_dz_rt.obj \
	allpole_df_rc_rt.obj \
	allpole_df_rr_rt.obj \
	allpole_df_zd_rt.obj \
	allpole_df_zz_rt.obj \
	allpole_lat_cc_rt.obj \
	allpole_lat_cr_rt.obj \
	allpole_lat_dd_rt.obj \
	allpole_lat_dz_rt.obj \
	allpole_lat_rc_rt.obj \
	allpole_lat_rr_rt.obj \
	allpole_lat_zd_rt.obj \
	allpole_lat_zz_rt.obj \
	allpole_tdf_a0scale_cc_rt.obj \
	allpole_tdf_a0scale_cr_rt.obj \
	allpole_tdf_a0scale_dd_rt.obj \
	allpole_tdf_a0scale_dz_rt.obj \
	allpole_tdf_a0scale_rc_rt.obj \
	allpole_tdf_a0scale_rr_rt.obj \
	allpole_tdf_a0scale_zd_rt.obj \
	allpole_tdf_a0scale_zz_rt.obj \
	allpole_tdf_cc_rt.obj \
	allpole_tdf_cr_rt.obj \
	allpole_tdf_dd_rt.obj \
	allpole_tdf_dz_rt.obj \
	allpole_tdf_rc_rt.obj \
	allpole_tdf_rr_rt.obj \
	allpole_tdf_zd_rt.obj \
	allpole_tdf_zz_rt.obj \
	avi_rt.obj \
	bq5_df2t_1fpf_1sos_cc_rt.obj \
	bq5_df2t_1fpf_1sos_cr_rt.obj \
	bq5_df2t_1fpf_1sos_dd_rt.obj \
	bq5_df2t_1fpf_1sos_dz_rt.obj \
	bq5_df2t_1fpf_1sos_rc_rt.obj \
	bq5_df2t_1fpf_1sos_rr_rt.obj \
	bq5_df2t_1fpf_1sos_zd_rt.obj \
	bq5_df2t_1fpf_1sos_zz_rt.obj \
	bq5_df2t_1fpf_nsos_cc_rt.obj \
	bq5_df2t_1fpf_nsos_cr_rt.obj \
	bq5_df2t_1fpf_nsos_dd_rt.obj \
	bq5_df2t_1fpf_nsos_dz_rt.obj \
	bq5_df2t_1fpf_nsos_rc_rt.obj \
	bq5_df2t_1fpf_nsos_rr_rt.obj \
	bq5_df2t_1fpf_nsos_zd_rt.obj \
	bq5_df2t_1fpf_nsos_zz_rt.obj \
	blms_an_wn_cc_rt.obj \
	blms_an_wn_dd_rt.obj \
	blms_an_wn_rr_rt.obj \
	blms_an_wn_zz_rt.obj \
	blms_an_wy_cc_rt.obj \
	blms_an_wy_dd_rt.obj \
	blms_an_wy_rr_rt.obj \
	blms_an_wy_zz_rt.obj \
	blms_ay_wn_cc_rt.obj \
	blms_ay_wn_dd_rt.obj \
	blms_ay_wn_rr_rt.obj \
	blms_ay_wn_zz_rt.obj \
	blms_ay_wy_cc_rt.obj \
	blms_ay_wy_dd_rt.obj \
	blms_ay_wy_rr_rt.obj \
	blms_ay_wy_zz_rt.obj \
	burg_a_c_rt.obj \
	burg_a_d_rt.obj \
	burg_a_r_rt.obj \
	burg_a_z_rt.obj \
	burg_ak_c_rt.obj \
	burg_ak_d_rt.obj \
	burg_ak_r_rt.obj \
	burg_ak_z_rt.obj \
	burg_k_c_rt.obj \
	burg_k_d_rt.obj \
	burg_k_r_rt.obj \
	burg_k_z_rt.obj \
	chol_c_rt.obj \
	chol_d_rt.obj \
	chol_r_rt.obj \
	chol_z_rt.obj \
	conv_td_cc_rt.obj \
	conv_td_dd_rt.obj \
	conv_td_dz_rt.obj \
	conv_td_rc_rt.obj \
	conv_td_rr_rt.obj \
	conv_td_zz_rt.obj \
	copy_and_zpad_cc_rt.obj \
	copy_and_zpad_dz_rt.obj \
	copy_and_zpad_rc_rt.obj \
	copy_and_zpad_zz_rt.obj \
	corr_td_cc_rt.obj \
	corr_td_cr_rt.obj \
	corr_td_dd_rt.obj \
	corr_td_dz_rt.obj \
	corr_td_rc_rt.obj \
	corr_td_rr_rt.obj \
	corr_td_zd_rt.obj \
	corr_td_zz_rt.obj \
	is_little_endian_rt.obj \
	eph_zc_fcn_rt.obj \
	bsub_nu_cc_c_rt.obj \
	bsub_nu_cr_c_rt.obj \
	bsub_nu_dd_d_rt.obj \
	bsub_nu_dz_z_rt.obj \
	bsub_nu_rc_c_rt.obj \
	bsub_nu_rr_r_rt.obj \
	bsub_nu_zd_z_rt.obj \
	bsub_nu_zz_z_rt.obj \
	bsub_rd_cc_c_rt.obj \
	bsub_rd_cr_c_rt.obj \
	bsub_rd_zd_z_rt.obj \
	bsub_rd_zz_z_rt.obj \
	bsub_u_cc_c_rt.obj \
	bsub_u_cr_c_rt.obj \
	bsub_u_dd_d_rt.obj \
	bsub_u_dz_z_rt.obj \
	bsub_u_rc_c_rt.obj \
	bsub_u_rr_r_rt.obj \
	bsub_u_zd_z_rt.obj \
	bsub_u_zz_z_rt.obj \
	fsub_nu_cc_c_rt.obj \
	fsub_nu_cr_c_rt.obj \
	fsub_nu_dd_d_rt.obj \
	fsub_nu_dz_z_rt.obj \
	fsub_nu_rc_c_rt.obj \
	fsub_nu_rr_r_rt.obj \
	fsub_nu_zd_z_rt.obj \
	fsub_nu_zz_z_rt.obj \
	fsub_rd_cc_c_rt.obj \
	fsub_rd_cr_c_rt.obj \
	fsub_rd_zd_z_rt.obj \
	fsub_rd_zz_z_rt.obj \
	fsub_u_cc_c_rt.obj \
	fsub_u_cr_c_rt.obj \
	fsub_u_dd_d_rt.obj \
	fsub_u_dz_z_rt.obj \
	fsub_u_rc_c_rt.obj \
	fsub_u_rr_r_rt.obj \
	fsub_u_zd_z_rt.obj \
	fsub_u_zz_z_rt.obj \
	copy_adjrow_intcol_br_c_rt.obj \
	copy_adjrow_intcol_br_z_rt.obj \
	copy_adjrow_intcol_c_rt.obj \
	copy_adjrow_intcol_z_rt.obj \
	copy_col_as_row_c_rt.obj \
	copy_col_as_row_z_rt.obj \
	copy_row_as_col_br_c_rt.obj \
	copy_row_as_col_br_dz_rt.obj \
	copy_row_as_col_br_rc_rt.obj \
	copy_row_as_col_br_z_rt.obj \
	copy_row_as_col_c_rt.obj \
	copy_row_as_col_dz_rt.obj \
	copy_row_as_col_rc_rt.obj \
	copy_row_as_col_z_rt.obj \
	fft_dbllen_tbl_c_rt.obj \
	fft_dbllen_tbl_z_rt.obj \
	fft_dbllen_trig_c_rt.obj \
	fft_dbllen_trig_z_rt.obj \
	fft_dblsig_br_c_rt.obj \
	fft_dblsig_br_z_rt.obj \
	fft_dblsig_c_rt.obj \
	fft_dblsig_z_rt.obj \
	fft_interleave_br_d_rt.obj \
	fft_interleave_br_r_rt.obj \
	fft_interleave_d_rt.obj \
	fft_interleave_r_rt.obj \
	fft_r2br_c_oop_rt.obj \
	fft_r2br_c_rt.obj \
	fft_r2br_dz_oop_rt.obj \
	fft_r2br_rc_oop_rt.obj \
	fft_r2br_z_oop_rt.obj \
	fft_r2br_z_rt.obj \
	fft_r2dif_tblm_c_rt.obj \
	fft_r2dif_tblm_z_rt.obj \
	fft_r2dif_tbls_c_rt.obj \
	fft_r2dif_tbls_z_rt.obj \
	fft_r2dif_trig_c_rt.obj \
	fft_r2dif_trig_z_rt.obj \
	fft_r2dit_tblm_c_rt.obj \
	fft_r2dit_tblm_z_rt.obj \
	fft_r2dit_tbls_c_rt.obj \
	fft_r2dit_tbls_z_rt.obj \
	fft_r2dit_trig_c_rt.obj \
	fft_r2dit_trig_z_rt.obj \
	fft_scaledata_dd_rt.obj \
	fft_scaledata_dz_rt.obj \
	fft_scaledata_rc_rt.obj \
	fft_scaledata_rr_rt.obj \
	ifft_addcssignals_c_c_oop_rt.obj \
	ifft_addcssignals_c_cbr_oop_rt.obj \
	ifft_addcssignals_d_z_oop_rt.obj \
	ifft_addcssignals_d_zbr_oop_rt.obj \
	ifft_addcssignals_r_c_oop_rt.obj \
	ifft_addcssignals_r_cbr_oop_rt.obj \
	ifft_addcssignals_z_z_oop_rt.obj \
	ifft_addcssignals_z_zbr_oop_rt.obj \
	ifft_dbllen_tbl_c_cbr_oop_rt.obj \
	ifft_dbllen_tbl_cbr_cbr_oop_rt.obj \
	ifft_dbllen_tbl_d_zbr_oop_rt.obj \
	ifft_dbllen_tbl_dbr_zbr_oop_rt.obj \
	ifft_dbllen_tbl_r_cbr_oop_rt.obj \
	ifft_dbllen_tbl_rbr_cbr_oop_rt.obj \
	ifft_dbllen_tbl_z_zbr_oop_rt.obj \
	ifft_dbllen_tbl_zbr_zbr_oop_rt.obj \
	ifft_dbllen_trig_c_cbr_oop_rt.obj \
	ifft_dbllen_trig_cbr_cbr_oop_rt.obj \
	ifft_dbllen_trig_d_zbr_oop_rt.obj \
	ifft_dbllen_trig_dbr_zbr_oop_rt.obj \
	ifft_dbllen_trig_r_cbr_oop_rt.obj \
	ifft_dbllen_trig_rbr_cbr_oop_rt.obj \
	ifft_dbllen_trig_z_zbr_oop_rt.obj \
	ifft_dbllen_trig_zbr_zbr_oop_rt.obj \
	ifft_deinterleave_d_d_inp_rt.obj \
	ifft_deinterleave_r_r_inp_rt.obj \
	2chabank_fr_df_cc_rt.obj \
	2chabank_fr_df_cr_rt.obj \
	2chabank_fr_df_dd_rt.obj \
	2chabank_fr_df_rr_rt.obj \
	2chabank_fr_df_zd_rt.obj \
	2chabank_fr_df_zz_rt.obj \
	2chsbank_df_cc_rt.obj \
	2chsbank_df_cr_rt.obj \
	2chsbank_df_dd_rt.obj \
	2chsbank_df_rr_rt.obj \
	2chsbank_df_zd_rt.obj \
	2chsbank_df_zz_rt.obj \
	fir_df_cc_rt.obj \
	fir_df_cr_rt.obj \
	fir_df_dd_rt.obj \
	fir_df_dz_rt.obj \
	fir_df_rc_rt.obj \
	fir_df_rr_rt.obj \
	fir_df_zd_rt.obj \
	fir_df_zz_rt.obj \
	fir_lat_cc_rt.obj \
	fir_lat_cr_rt.obj \
	fir_lat_dd_rt.obj \
	fir_lat_dz_rt.obj \
	fir_lat_rc_rt.obj \
	fir_lat_rr_rt.obj \
	fir_lat_zd_rt.obj \
	fir_lat_zz_rt.obj \
	fir_tdf_cc_rt.obj \
	fir_tdf_cr_rt.obj \
	fir_tdf_dd_rt.obj \
	fir_tdf_dz_rt.obj \
	fir_tdf_rc_rt.obj \
	fir_tdf_rr_rt.obj \
	fir_tdf_zd_rt.obj \
	fir_tdf_zz_rt.obj \
	g711_enc_a_sat_rt.obj \
	g711_enc_mu_sat_rt.obj \
	ic_copy_channel_rt.obj \
	ic_copy_matrix_rt.obj \
	ic_copy_scalar_rt.obj \
	ic_copy_vector_rt.obj \
	ic_old_copy_fcns_rt.obj \
	iir_df1_a0scale_cc_rt.obj \
	iir_df1_a0scale_cr_rt.obj \
	iir_df1_a0scale_dd_rt.obj \
	iir_df1_a0scale_dz_rt.obj \
	iir_df1_a0scale_rc_rt.obj \
	iir_df1_a0scale_rr_rt.obj \
	iir_df1_a0scale_zd_rt.obj \
	iir_df1_a0scale_zz_rt.obj \
	iir_df1_cc_rt.obj \
	iir_df1_cr_rt.obj \
	iir_df1_dd_rt.obj \
	iir_df1_dz_rt.obj \
	iir_df1_rc_rt.obj \
	iir_df1_rr_rt.obj \
	iir_df1_zd_rt.obj \
	iir_df1_zz_rt.obj \
	iir_df1t_a0scale_cc_rt.obj \
	iir_df1t_a0scale_cr_rt.obj \
	iir_df1t_a0scale_dd_rt.obj \
	iir_df1t_a0scale_dz_rt.obj \
	iir_df1t_a0scale_rc_rt.obj \
	iir_df1t_a0scale_rr_rt.obj \
	iir_df1t_a0scale_zd_rt.obj \
	iir_df1t_a0scale_zz_rt.obj \
	iir_df1t_cc_rt.obj \
	iir_df1t_cr_rt.obj \
	iir_df1t_dd_rt.obj \
	iir_df1t_dz_rt.obj \
	iir_df1t_rc_rt.obj \
	iir_df1t_rr_rt.obj \
	iir_df1t_zd_rt.obj \
	iir_df1t_zz_rt.obj \
	iir_df2_a0scale_cc_rt.obj \
	iir_df2_a0scale_cr_rt.obj \
	iir_df2_a0scale_dd_rt.obj \
	iir_df2_a0scale_dz_rt.obj \
	iir_df2_a0scale_rc_rt.obj \
	iir_df2_a0scale_rr_rt.obj \
	iir_df2_a0scale_zd_rt.obj \
	iir_df2_a0scale_zz_rt.obj \
	iir_df2_cc_rt.obj \
	iir_df2_cr_rt.obj \
	iir_df2_dd_rt.obj \
	iir_df2_dz_rt.obj \
	iir_df2_rc_rt.obj \
	iir_df2_rr_rt.obj \
	iir_df2_zd_rt.obj \
	iir_df2_zz_rt.obj \
	iir_df2t_a0scale_cc_rt.obj \
	iir_df2t_a0scale_cr_rt.obj \
	iir_df2t_a0scale_dd_rt.obj \
	iir_df2t_a0scale_dz_rt.obj \
	iir_df2t_a0scale_rc_rt.obj \
	iir_df2t_a0scale_rr_rt.obj \
	iir_df2t_a0scale_zd_rt.obj \
	iir_df2t_a0scale_zz_rt.obj \
	iir_df2t_cc_rt.obj \
	iir_df2t_cr_rt.obj \
	iir_df2t_dd_rt.obj \
	iir_df2t_dz_rt.obj \
	iir_df2t_rc_rt.obj \
	iir_df2t_rr_rt.obj \
	iir_df2t_zd_rt.obj \
	iir_df2t_zz_rt.obj \
	interp_fir_d_rt.obj \
	interp_fir_r_rt.obj \
	interp_lin_d_rt.obj \
	interp_lin_r_rt.obj \
	ldl_c_rt.obj \
	ldl_d_rt.obj \
	ldl_r_rt.obj \
	ldl_z_rt.obj \
	lms_an_wn_cc_rt.obj \
	lms_an_wn_dd_rt.obj \
	lms_an_wn_rr_rt.obj \
	lms_an_wn_zz_rt.obj \
	lms_an_wy_cc_rt.obj \
	lms_an_wy_dd_rt.obj \
	lms_an_wy_rr_rt.obj \
	lms_an_wy_zz_rt.obj \
	lms_ay_wn_cc_rt.obj \
	lms_ay_wn_dd_rt.obj \
	lms_ay_wn_rr_rt.obj \
	lms_ay_wn_zz_rt.obj \
	lms_ay_wy_cc_rt.obj \
	lms_ay_wy_dd_rt.obj \
	lms_ay_wy_rr_rt.obj \
	lms_ay_wy_zz_rt.obj \
	lmsn_an_wn_cc_rt.obj \
	lmsn_an_wn_dd_rt.obj \
	lmsn_an_wn_rr_rt.obj \
	lmsn_an_wn_zz_rt.obj \
	lmsn_an_wy_cc_rt.obj \
	lmsn_an_wy_dd_rt.obj \
	lmsn_an_wy_rr_rt.obj \
	lmsn_an_wy_zz_rt.obj \
	lmsn_ay_wn_cc_rt.obj \
	lmsn_ay_wn_dd_rt.obj \
	lmsn_ay_wn_rr_rt.obj \
	lmsn_ay_wn_zz_rt.obj \
	lmsn_ay_wy_cc_rt.obj \
	lmsn_ay_wy_dd_rt.obj \
	lmsn_ay_wy_rr_rt.obj \
	lmsn_ay_wy_zz_rt.obj \
	lmssd_an_wn_dd_rt.obj \
	lmssd_an_wn_rr_rt.obj \
	lmssd_an_wy_dd_rt.obj \
	lmssd_an_wy_rr_rt.obj \
	lmssd_ay_wn_dd_rt.obj \
	lmssd_ay_wn_rr_rt.obj \
	lmssd_ay_wy_dd_rt.obj \
	lmssd_ay_wy_rr_rt.obj \
	lmsse_an_wn_dd_rt.obj \
	lmsse_an_wn_rr_rt.obj \
	lmsse_an_wy_dd_rt.obj \
	lmsse_an_wy_rr_rt.obj \
	lmsse_ay_wn_dd_rt.obj \
	lmsse_ay_wn_rr_rt.obj \
	lmsse_ay_wy_dd_rt.obj \
	lmsse_ay_wy_rr_rt.obj \
	lmsss_an_wn_dd_rt.obj \
	lmsss_an_wn_rr_rt.obj \
	lmsss_an_wy_dd_rt.obj \
	lmsss_an_wy_rr_rt.obj \
	lmsss_ay_wn_dd_rt.obj \
	lmsss_ay_wn_rr_rt.obj \
	lmsss_ay_wy_dd_rt.obj \
	lmsss_ay_wy_rr_rt.obj \
	cc2lpc_d_rt.obj \
	cc2lpc_r_rt.obj \
	lpc2cc_d_rt.obj \
	lpc2cc_r_rt.obj \
	lsp2poly_evenord_d_rt.obj \
	lsp2poly_evenord_r_rt.obj \
	lsp2poly_oddord_d_rt.obj \
	lsp2poly_oddord_r_rt.obj \
	lu_c_rt.obj \
	lu_d_rt.obj \
	lu_r_rt.obj \
	lu_z_rt.obj \
	mmpcmaudio_rt.obj \
	mmrgb24convert2gray_rt.obj \
	mmrgb24convert_rt.obj \
	mmrgb24output_rt.obj \
	mmrgb24paddedoutput_rt.obj \
	pinv_c_rt.obj \
	pinv_d_rt.obj \
	pinv_r_rt.obj \
	pinv_z_rt.obj \
	poly2lsfn_d_rt.obj \
	poly2lsfn_r_rt.obj \
	poly2lsfr_d_rt.obj \
	poly2lsfr_r_rt.obj \
	poly2lsp_d_rt.obj \
	poly2lsp_r_rt.obj \
	polyval_cc_rt.obj \
	polyval_cr_rt.obj \
	polyval_dd_rt.obj \
	polyval_dz_rt.obj \
	polyval_rc_rt.obj \
	polyval_rr_rt.obj \
	polyval_zd_rt.obj \
	polyval_zz_rt.obj \
	qrcompqy_c_rt.obj \
	qrcompqy_d_rt.obj \
	qrcompqy_mixd_c_rt.obj \
	qrcompqy_mixd_z_rt.obj \
	qrcompqy_r_rt.obj \
	qrcompqy_z_rt.obj \
	qrdc_c_rt.obj \
	qrdc_d_rt.obj \
	qrdc_r_rt.obj \
	qrdc_z_rt.obj \
	qre_c_rt.obj \
	qre_d_rt.obj \
	qre_r_rt.obj \
	qre_z_rt.obj \
	qreslv_c_rt.obj \
	qreslv_d_rt.obj \
	qreslv_mixd_c_rt.obj \
	qreslv_mixd_z_rt.obj \
	qreslv_r_rt.obj \
	qreslv_z_rt.obj \
	sort_ins_idx_d_rt.obj \
	sort_ins_idx_r_rt.obj \
	sort_ins_idx_s08_rt.obj \
	sort_ins_idx_s16_rt.obj \
	sort_ins_idx_s32_rt.obj \
	sort_ins_idx_u08_rt.obj \
	sort_ins_idx_u16_rt.obj \
	sort_ins_idx_u32_rt.obj \
	sort_ins_val_d_rt.obj \
	sort_ins_val_r_rt.obj \
	sort_ins_val_s08_rt.obj \
	sort_ins_val_s16_rt.obj \
	sort_ins_val_s32_rt.obj \
	sort_ins_val_u08_rt.obj \
	sort_ins_val_u16_rt.obj \
	sort_ins_val_u32_rt.obj \
	sort_qk_idx_d_rt.obj \
	sort_qk_idx_r_rt.obj \
	sort_qk_idx_s08_rt.obj \
	sort_qk_idx_s16_rt.obj \
	sort_qk_idx_s32_rt.obj \
	sort_qk_idx_u08_rt.obj \
	sort_qk_idx_u16_rt.obj \
	sort_qk_idx_u32_rt.obj \
	sort_qk_val_d_rt.obj \
	sort_qk_val_r_rt.obj \
	sort_qk_val_s08_rt.obj \
	sort_qk_val_s16_rt.obj \
	sort_qk_val_s32_rt.obj \
	sort_qk_val_u08_rt.obj \
	sort_qk_val_u16_rt.obj \
	sort_qk_val_u32_rt.obj \
	srt_qid_findpivot_d_rt.obj \
	srt_qid_findpivot_r_rt.obj \
	srt_qid_partition_d_rt.obj \
	srt_qid_partition_r_rt.obj \
	srt_qkrec_c_rt.obj \
	srt_qkrec_d_rt.obj \
	srt_qkrec_r_rt.obj \
	srt_qkrec_z_rt.obj \
	randsrc_gc_c_rt.obj \
	randsrc_gc_d_rt.obj \
	randsrc_gc_r_rt.obj \
	randsrc_gc_z_rt.obj \
	randsrc_gz_c_rt.obj \
	randsrc_gz_d_rt.obj \
	randsrc_gz_r_rt.obj \
	randsrc_gz_z_rt.obj \
	randsrc_u_c_rt.obj \
	randsrc_u_d_rt.obj \
	randsrc_u_r_rt.obj \
	randsrc_u_z_rt.obj \
	randsrccreateseeds_32_rt.obj \
	randsrccreateseeds_64_rt.obj \
	randsrcinitstate_gc_32_rt.obj \
	randsrcinitstate_gc_64_rt.obj \
	randsrcinitstate_gz_rt.obj \
	randsrcinitstate_u_32_rt.obj \
	randsrcinitstate_u_64_rt.obj \
	lpc2ac_d_rt.obj \
	lpc2ac_r_rt.obj \
	rc2ac_d_rt.obj \
	rc2ac_r_rt.obj \
	lpc2rc_d_rt.obj \
	lpc2rc_r_rt.obj \
	rc2lpc_d_rt.obj \
	rc2lpc_r_rt.obj \
	buf_copy_frame_to_mem_OL_1ch_rt.obj \
	buf_copy_frame_to_mem_OL_rt.obj \
	buf_copy_input_to_output_1ch_rt.obj \
	buf_copy_input_to_output_rt.obj \
	buf_copy_scalar_to_mem_OL_1ch_rt.obj \
	buf_copy_scalar_to_mem_OL_rt.obj \
	buf_copy_scalar_to_mem_UL_1ch_rt.obj \
	buf_copy_scalar_to_mem_UL_rt.obj \
	buf_output_frame_1ch_rt.obj \
	buf_output_frame_rt.obj \
	buf_output_scalar_1ch_rt.obj \
	buf_output_scalar_rt.obj \
	unwrap_d_nrip_rt.obj \
	unwrap_d_nrop_rt.obj \
	unwrap_d_ripf_rt.obj \
	unwrap_d_rips_rt.obj \
	unwrap_d_ropf_rt.obj \
	unwrap_d_rops_rt.obj \
	unwrap_r_nrip_rt.obj \
	unwrap_r_nrop_rt.obj \
	unwrap_r_ripf_rt.obj \
	unwrap_r_rips_rt.obj \
	unwrap_r_ropf_rt.obj \
	unwrap_r_rops_rt.obj \
	vfdly_clip_d_rt.obj \
	vfdly_clip_r_rt.obj \
	vfdly_fir_c_rt.obj \
	vfdly_fir_d_rt.obj \
	vfdly_fir_r_rt.obj \
	vfdly_fir_z_rt.obj \
	vfdly_lin_c_rt.obj \
	vfdly_lin_d_rt.obj \
	vfdly_lin_r_rt.obj \
	vfdly_lin_z_rt.obj \


dsp_dyn_rt.lib : $(MAKEFILE) rtw_proj.tmw $(MODULES_dsp_dyn_rt)
	@cmd /C "echo ### Creating $@"
	@if exist $@ del $@
	$(LIBCMD) /out:$@ $(MODULES_dsp_dyn_rt)
	@cmd /C "echo ### $@ Created"



#----------------------------- Dependencies ------------------------------------

$(OBJS) : $(MAKEFILE) rtw_proj.tmw

$(SHARED_OBJS) : $(SHARED_BIN_DIR)/%.obj : $(SHARED_SRC_DIR)/%.c 
	$(CC) -c -Fo$@ $(CFLAGS) $<

$(SHARED_LIB) : $(SHARED_OBJS)
	@cmd /C "echo ### Creating $@" 
	$(LIBCMD) /out:$@ $(FMT_SHARED_OBJS)
	@cmd /C "echo ### $@ Created"

#--------- Miscellaneous rules to purge, clean and lint (sol2 only) ------------

purge : clean
	@cmd /C "echo ### Deleting the generated source code for $(MODEL)"
	@del $(MODEL).c $(MODEL).h $(MODEL)_types.h $(MODEL)_data.c \
	        $(MODEL)_private.h $(MODEL).rtw $(MODULES) rtw_proj.tmw $(MAKEFILE)

clean :
	@cmd /C "echo ### Deleting the objects and $(PROGRAM)"
	@del $(LINK_OBJS) ..\$(MODEL).exe

	@del dsp_dyn_rt.lib


# EOF: grt_lcc.tmf
