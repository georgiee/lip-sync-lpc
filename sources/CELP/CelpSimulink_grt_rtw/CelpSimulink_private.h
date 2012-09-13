/*
 * CelpSimulink_private.h
 *
 * Real-Time Workshop code generation for Simulink model "CelpSimulink.mdl".
 *
 * Model Version              : 1.10
 * Real-Time Workshop version : 7.1  (R2008a)  23-Jan-2008
 * C source code generated on : Mon Oct 31 17:46:25 2011
 */
#ifndef RTW_HEADER_CelpSimulink_private_h_
#define RTW_HEADER_CelpSimulink_private_h_
#include "rtwtypes.h"
#include "dsp_rt.h"                    /* Signal Processing Blockset general run time support functions */
#include "fromwavefile_ex_win32.h"     /* Signal Processing Blockset run time support library */
#include "dspfilter_rt.h"              /* Signal Processing Blockset run time support library */
#include "dsprebuff_rt.h"              /* Signal Processing Blockset run time support library */
#include "dsp_ic_rt.h"                 /* Signal Processing Blockset run time support library */
#include "dspacf_rt.h"                 /* Signal Processing Blockset run time support library */
#include "AudioDevice_rtw.h"           /* Signal Processing Blockset run time support library */
#ifndef __RTWTYPES_H__
#error This file requires rtwtypes.h to be included
#else
#ifdef TMWTYPES_PREVIOUSLY_INCLUDED
#error This file requires rtwtypes.h to be included before tmwtypes.h
#else

/* Check for inclusion of an incorrect version of rtwtypes.h */
#ifndef RTWTYPES_ID_C08S16I32L32N32F1
#error This code was generated with a different "rtwtypes.h" than the file included
#endif                                 /* RTWTYPES_ID_C08S16I32L32N32F1 */
#endif                                 /* TMWTYPES_PREVIOUSLY_INCLUDED */
#endif                                 /* __RTWTYPES_H__ */

/*
 * UNUSED_PARAMETER(x)
 *   Used to specify that a function parameter (argument) is required but not
 *   accessed by the function body.
 */
#ifndef UNUSED_PARAMETER
# if defined(__LCC__)
#   define UNUSED_PARAMETER(x)                                   /* do nothing */
# else

/*
 * This is the semi-ANSI standard way of indicating that an
 * unused function parameter is required.
 */
#   define UNUSED_PARAMETER(x)         (void) (x)
# endif
#endif
#endif                                 /* RTW_HEADER_CelpSimulink_private_h_ */
