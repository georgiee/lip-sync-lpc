/*
 * CelpSimulink.c
 *
 * Real-Time Workshop code generation for Simulink model "CelpSimulink.mdl".
 *
 * Model Version              : 1.10
 * Real-Time Workshop version : 7.1  (R2008a)  23-Jan-2008
 * C source code generated on : Mon Oct 31 17:46:25 2011
 */

#include "CelpSimulink.h"
#include "CelpSimulink_private.h"

/* Block signals (auto storage) */
BlockIO_CelpSimulink CelpSimulink_B;

/* Block states (auto storage) */
D_Work_CelpSimulink CelpSimulink_DWork;

/* Real-time model */
RT_MODEL_CelpSimulink CelpSimulink_M_;
RT_MODEL_CelpSimulink *CelpSimulink_M = &CelpSimulink_M_;

/* Model output function */
static void CelpSimulink_output(int_T tid)
{
  /* local block i/o variables */
  real_T rtb_Product3;
  real32_T rtb_Autocorrelation[11];
  real32_T rtb_Window[160];

  {
    int32_T s1_iter;
    int32_T iA;
    boolean_T isIEven;
    real32_T rTmpP;
    real32_T acc;
    int32_T upLim;
    int32_T j;
    real32_T rTmpA;
    int32_T i;

    /* Outputs for iterator SubSystem: '<Root>/Codebook Search' */

    /* InitializeConditions for S-Function (sdspstatminmax): '<S6>/Maximum1' */
    CelpSimulink_DWork.Maximum1_Valdata = rtMinusInf;

    /* InitializeConditions for UnitDelay: '<S5>/Unit Delay' */
    CelpSimulink_DWork.UnitDelay_DSTATE = CelpSimulink_P.UnitDelay_X0;
    for (s1_iter = 1; s1_iter < 81; s1_iter++) {
      CelpSimulink_B.ForIterator = s1_iter;

      /* Product: '<S4>/Product3' incorporates:
       *  Abs: '<S4>/Abs'
       */
      rtb_Product3 = fabs(0.0) / 0.0;

      /* S-Function (sdspstatminmax): '<S6>/Maximum1' */
      if (rtb_Product3 > CelpSimulink_DWork.Maximum1_Valdata) {
        CelpSimulink_DWork.Maximum1_Valdata = rtb_Product3;
      }

      /* Switch: '<S5>/Switch' incorporates:
       *  RelationalOperator: '<S6>/Relational Operator'
       *  S-Function (sdspstatminmax): '<S6>/Maximum1'
       *  UnitDelay: '<S5>/Unit Delay'
       */
      if (CelpSimulink_DWork.Maximum1_Valdata > rtb_Product3) {
        CelpSimulink_B.Switch = CelpSimulink_DWork.UnitDelay_DSTATE;
      } else {
        CelpSimulink_B.Switch = CelpSimulink_B.ForIterator;
      }

      /* Update for UnitDelay: '<S5>/Unit Delay' */
      CelpSimulink_DWork.UnitDelay_DSTATE = CelpSimulink_B.Switch;
    }

    /* end of Outputs for SubSystem: '<Root>/Codebook Search' */

    /* Signal Processing Blockset Variable Selector (sdspperm2) - '<Root>/Variable Selector' */
    /* Permute columns port 0 input columns = 256, output columns = 1 */
    {
      int32_T i_idx = (int32_T)(CelpSimulink_B.Switch - 1);

      /* Clip bad index */
      if (i_idx < 0) {
        i_idx = 0;
      } else if (i_idx > 255) {
        i_idx = 255;
      }

      {
        int_T A_idx = i_idx*80;
        memcpy(CelpSimulink_B.VariableSelector,
               &CelpSimulink_P.Constant1_Value[A_idx], 80*sizeof(real32_T));
      }
    }

    {
      /* Signal Processing Blockset From Wave File (sdspwafi2) - '<Root>/From Wave File' - Output */
      MWDSP_Wafi_FunctionPtrStruct* fcnPtrs = (MWDSP_Wafi_FunctionPtrStruct*)
        &CelpSimulink_DWork.FromWaveFile_RuntimeFcnPtrs[0];
      fcnPtrs->outputsFcn(CelpSimulink_DWork.FromWaveFile_FromWaveFileObj,
                          CelpSimulink_B.FromWaveFile);
    }

    /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S2>/Pre-Emphasis' */
    /* FIR, Direct-form */
    MWDSP_FIR_DF_RR(CelpSimulink_B.FromWaveFile, CelpSimulink_B.PreEmphasis,
                    &CelpSimulink_DWork.PreEmphasis_FILT_STATES[0],
                    &CelpSimulink_DWork.PreEmphasis_CIRCBUFFIDX, 2, 80, 1,
                    &CelpSimulink_P.PreEmphasis_RTP1COEFF[0], 1);

    /* Signal Processing Blockset Buffer/Unbuffer (sdsprebuff2) - '<S2>/Overlap Analysis Windows' */
    {
      const byte_T *u = (const byte_T *)CelpSimulink_B.PreEmphasis;
      byte_T *y = (byte_T *)rtb_Window;
      byte_T *mem = (byte_T *)
        &CelpSimulink_DWork.OverlapAnalysisWindows_CircBuff[0];
      int_T uWidth = 80 * sizeof(real32_T);
      int_T yWidth = 160 * sizeof(real32_T);
      int_T memWidth = yWidth - uWidth;
      memcpy(y, mem, memWidth);
      memcpy((y+memWidth), u, uWidth);
      memcpy(mem, (u+uWidth-memWidth), memWidth);
    }

    /* Signal Processing Blockset Window (sdspwindow2) - '<S2>/Window' */
    {
      int_T i;
      for (i=0; i<160; i++) {
        rtb_Window[i] *= (CelpSimulink_ConstP.Window_WindowSample[i]);
      }
    }

    /* Signal Processing Blockset Autocorrelation (sdspacf2) - '<S7>/Autocorrelation' */
    MWDSP_ACF_TD_R(rtb_Window, 160, rtb_Autocorrelation, 11, 1);

    /* Scale for biased acf estimate. */
    {
      real32_T *y = rtb_Autocorrelation;
      real32_T g;
      int_T i= 11;
      g = 1.0F / (real32_T)160;
      while (i--) {
        *y++ *= g;
      }
    }

    /* S-Function (sdsplevdurb2): '<S7>/Levinson-Durbin' */
    if (rtb_Autocorrelation[0] == 0.0F) {
      CelpSimulink_B.LevinsonDurbin[0] = 1.0F;
      iA = 1;
      for (s1_iter = 1; s1_iter < 11; s1_iter++) {
        CelpSimulink_B.LevinsonDurbin[iA] = 0.0F;
        iA++;
      }
    } else {
      isIEven = 0U;
      rTmpP = rtb_Autocorrelation[0];
      for (iA = 1; iA < 11; iA++) {
        acc = rtb_Autocorrelation[iA];
        s1_iter = 1;
        i = iA - 1;
        for (j = 1; j < iA; j++) {
          acc += CelpSimulink_B.LevinsonDurbin[s1_iter] * rtb_Autocorrelation[i];
          s1_iter++;
          i--;
        }

        acc = -acc;
        acc /= rTmpP;
        rTmpP -= acc * acc * rTmpP;
        upLim = (iA - 1) >> 1;
        s1_iter = 1;
        i = iA - 1;
        for (j = 1; j <= upLim; j++) {
          rTmpA = CelpSimulink_B.LevinsonDurbin[s1_iter];
          CelpSimulink_B.LevinsonDurbin[s1_iter] = acc *
            CelpSimulink_B.LevinsonDurbin[i] +
            CelpSimulink_B.LevinsonDurbin[s1_iter];
          CelpSimulink_B.LevinsonDurbin[i] = acc * rTmpA +
            CelpSimulink_B.LevinsonDurbin[i];
          s1_iter++;
          i--;
        }

        if (isIEven) {
          s1_iter = iA >> 1;
          CelpSimulink_B.LevinsonDurbin[s1_iter] = acc *
            CelpSimulink_B.LevinsonDurbin[s1_iter] +
            CelpSimulink_B.LevinsonDurbin[s1_iter];
        }

        isIEven = !isIEven;
        CelpSimulink_B.LevinsonDurbin[iA] = acc;
      }

      CelpSimulink_B.LevinsonDurbin[0] = 1.0F;
    }

    /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S3>/Time-Varying Synthesis Filter' */
    /* All-pole, Direct-form (a0 ~= 1) */
    MWDSP_AllPole_DF_A0Scale_RR(CelpSimulink_B.VariableSelector,
      CelpSimulink_B.TimeVaryingSynthesisFilter,
      &CelpSimulink_DWork.TimeVaryingSynthesisFilter_FILT[0],
      &CelpSimulink_DWork.TimeVaryingSynthesisFilter_CIRC, 10, 1, 80,
      CelpSimulink_B.LevinsonDurbin, 1);

    /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S3>/De-emphasis Filter' */
    /* All-pole, Direct-form (a0 == 1) */
    MWDSP_AllPole_DF_RR(CelpSimulink_B.TimeVaryingSynthesisFilter,
                        CelpSimulink_B.DeemphasisFilter,
                        &CelpSimulink_DWork.DeemphasisFilter_FILT_STATES[0],
                        &CelpSimulink_DWork.DeemphasisFilter_CIRCBUFFIDX, 1, 1,
                        80, &CelpSimulink_P.DeemphasisFilter_RTP1COEFF[0], 1);
  }

  UNUSED_PARAMETER(tid);
}

/* Model update function */
static void CelpSimulink_update(int_T tid)
{
  {
    static char sErr[512];
    void *device = CelpSimulink_DWork.ToAudioDevice_AudioDevice;
    AudioDeviceLibrary *adl = (AudioDeviceLibrary*)
      &CelpSimulink_DWork.ToAudioDevice_AudioDeviceLib[0];
    sErr[0] = 0;
    if (device)
      adl->deviceUpdate(device, sErr, CelpSimulink_B.DeemphasisFilter, 1, 1);
    if (*sErr) {
      DestroyAudioDeviceLibrary(adl);
      rtmSetErrorStatus(CelpSimulink_M, sErr);
      rtmSetStopRequested(CelpSimulink_M, 1);
    }
  }

  /* Update absolute time for base rate */
  if (!(++CelpSimulink_M->Timing.clockTick0))
    ++CelpSimulink_M->Timing.clockTickH0;
  CelpSimulink_M->Timing.t[0] = CelpSimulink_M->Timing.clockTick0 *
    CelpSimulink_M->Timing.stepSize0 + CelpSimulink_M->Timing.clockTickH0 *
    CelpSimulink_M->Timing.stepSize0 * 4294967296.0;
  UNUSED_PARAMETER(tid);
}

/* Model initialize function */
void CelpSimulink_initialize(boolean_T firstTime)
{
  (void)firstTime;

  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize real-time model */
  (void) memset((char_T *)CelpSimulink_M,0,
                sizeof(RT_MODEL_CelpSimulink));

  /* Initialize timing info */
  {
    int_T *mdlTsMap = CelpSimulink_M->Timing.sampleTimeTaskIDArray;
    mdlTsMap[0] = 0;
    CelpSimulink_M->Timing.sampleTimeTaskIDPtr = (&mdlTsMap[0]);
    CelpSimulink_M->Timing.sampleTimes =
      (&CelpSimulink_M->Timing.sampleTimesArray[0]);
    CelpSimulink_M->Timing.offsetTimes =
      (&CelpSimulink_M->Timing.offsetTimesArray[0]);

    /* task periods */
    CelpSimulink_M->Timing.sampleTimes[0] = (0.01);

    /* task offsets */
    CelpSimulink_M->Timing.offsetTimes[0] = (0.0);
  }

  rtmSetTPtr(CelpSimulink_M, &CelpSimulink_M->Timing.tArray[0]);

  {
    int_T *mdlSampleHits = CelpSimulink_M->Timing.sampleHitArray;
    mdlSampleHits[0] = 1;
    CelpSimulink_M->Timing.sampleHits = (&mdlSampleHits[0]);
  }

  rtmSetTFinal(CelpSimulink_M, -1);
  CelpSimulink_M->Timing.stepSize0 = 0.01;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    CelpSimulink_M->rtwLogInfo = &rt_DataLoggingInfo;
    rtliSetLogXSignalInfo(CelpSimulink_M->rtwLogInfo, NULL);
    rtliSetLogXSignalPtrs(CelpSimulink_M->rtwLogInfo, NULL);
    rtliSetLogT(CelpSimulink_M->rtwLogInfo, "tout");
    rtliSetLogX(CelpSimulink_M->rtwLogInfo, "");
    rtliSetLogXFinal(CelpSimulink_M->rtwLogInfo, "");
    rtliSetSigLog(CelpSimulink_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(CelpSimulink_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(CelpSimulink_M->rtwLogInfo, 0);
    rtliSetLogMaxRows(CelpSimulink_M->rtwLogInfo, 1000);
    rtliSetLogDecimation(CelpSimulink_M->rtwLogInfo, 1);
    rtliSetLogY(CelpSimulink_M->rtwLogInfo, "");
    rtliSetLogYSignalInfo(CelpSimulink_M->rtwLogInfo, NULL);
    rtliSetLogYSignalPtrs(CelpSimulink_M->rtwLogInfo, NULL);
  }

  CelpSimulink_M->solverInfoPtr = (&CelpSimulink_M->solverInfo);
  CelpSimulink_M->Timing.stepSize = (0.01);
  rtsiSetFixedStepSize(&CelpSimulink_M->solverInfo, 0.01);
  rtsiSetSolverMode(&CelpSimulink_M->solverInfo, SOLVER_MODE_SINGLETASKING);

  /* block I/O */
  CelpSimulink_M->ModelData.blockIO = ((void *) &CelpSimulink_B);
  (void) memset(((void *) &CelpSimulink_B),0,
                sizeof(BlockIO_CelpSimulink));

  {
    int_T i;
    void *pVoidBlockIORegion;
    pVoidBlockIORegion = (void *)(&CelpSimulink_B.FromWaveFile[0]);
    for (i = 0; i < 411; i++) {
      ((real32_T*)pVoidBlockIORegion)[i] = 0.0F;
    }
  }

  /* parameters */
  CelpSimulink_M->ModelData.defaultParam = ((real_T *) &CelpSimulink_P);

  /* states (dwork) */
  CelpSimulink_M->Work.dwork = ((void *) &CelpSimulink_DWork);
  (void) memset((char_T *) &CelpSimulink_DWork,0,
                sizeof(D_Work_CelpSimulink));
  CelpSimulink_DWork.Maximum1_Valdata = 0.0;

  {
    int_T i;
    real32_T *dwork_ptr = (real32_T *)
      &CelpSimulink_DWork.PreEmphasis_FILT_STATES[0];
    for (i = 0; i < 1122; i++) {
      dwork_ptr[i] = 0.0F;
    }
  }
}

/* Model terminate function */
void CelpSimulink_terminate(void)
{
  {
    /* Signal Processing Blockset From Wave File (sdspwafi2) - '<Root>/From Wave File' - Terminate */
    MWDSP_Wafi_FunctionPtrStruct* fcnPtrs = (MWDSP_Wafi_FunctionPtrStruct*)
      &CelpSimulink_DWork.FromWaveFile_RuntimeFcnPtrs[0];
    if (fcnPtrs != NULL && fcnPtrs->terminateFcn != NULL)
      fcnPtrs->terminateFcn(CelpSimulink_DWork.FromWaveFile_FromWaveFileObj);
  }

  {
    static char sErr[512];
    void *device = CelpSimulink_DWork.ToAudioDevice_AudioDevice;
    AudioDeviceLibrary *adl = (AudioDeviceLibrary*)
      &CelpSimulink_DWork.ToAudioDevice_AudioDeviceLib[0];
    sErr[0] = 0;
    if (device) {
      adl->deviceTerminate(device, sErr);
      adl->deviceDestroy(device, sErr, 0);
    }

    DestroyAudioDeviceLibrary(adl);
  }
}

/*========================================================================*
 * Start of GRT compatible call interface                                 *
 *========================================================================*/
void MdlOutputs(int_T tid)
{
  CelpSimulink_output(tid);
}

void MdlUpdate(int_T tid)
{
  CelpSimulink_update(tid);
}

void MdlInitializeSizes(void)
{
  CelpSimulink_M->Sizes.numContStates = (0);/* Number of continuous states */
  CelpSimulink_M->Sizes.numY = (0);    /* Number of model outputs */
  CelpSimulink_M->Sizes.numU = (0);    /* Number of model inputs */
  CelpSimulink_M->Sizes.sysDirFeedThru = (0);/* The model is not direct feedthrough */
  CelpSimulink_M->Sizes.numSampTimes = (1);/* Number of sample times */
  CelpSimulink_M->Sizes.numBlocks = (20);/* Number of blocks */
  CelpSimulink_M->Sizes.numBlockIO = (8);/* Number of block outputs */
  CelpSimulink_M->Sizes.numBlockPrms = (20485);/* Sum of parameter "widths" */
}

void MdlInitializeSampleTimes(void)
{
}

void MdlInitialize(void)
{
  {
    /* Signal Processing Blockset From Wave File (sdspwafi2) - '<Root>/From Wave File' - Initialize */
    MWDSP_Wafi_FunctionPtrStruct* fcnPtrs = (MWDSP_Wafi_FunctionPtrStruct*)
      &CelpSimulink_DWork.FromWaveFile_RuntimeFcnPtrs[0];

    /* destroy previous runtime object, if there is one */
    if (fcnPtrs != NULL && fcnPtrs->terminateFcn != NULL)
      fcnPtrs->terminateFcn(CelpSimulink_DWork.FromWaveFile_FromWaveFileObj);
    CelpSimulink_DWork.FromWaveFile_FromWaveFileObj = exMWDSP_Wafi_Create(
      "C:\\Documents and Settings\\Maquina\\Meus documentos\\Arthur\\UFRGS\\DSP\\CELP\\audio.wav",
      16U, 80, 1, 80, 8000.0, 1);
    if (CelpSimulink_DWork.FromWaveFile_FromWaveFileObj == NULL) {
      const char* errMsg = exMWDSP_Wafi_GetErrorMessage();
      rtmSetErrorStatus(CelpSimulink_M, errMsg);
      rtmSetStopRequested(CelpSimulink_M, 1);
    } else {
      exMWDSP_Wafi_GetFunctionPtrs(fcnPtrs);
    }
  }

  /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S2>/Pre-Emphasis' */
  /* FIR, Direct-form */
  {
    real32_T *statePtr = (real32_T *)
      &CelpSimulink_DWork.PreEmphasis_FILT_STATES[0];

    /* Scalar expansion of ICs with extra zero element per channel */
    *statePtr++ = *(const real32_T *)&CelpSimulink_ConstP.pooled1;
    *statePtr++ = 0.0F;
  }

  /* Copy ICs into circular buffer */
  {
    const int_T bufLenBytes = 80 * sizeof(real32_T);
    byte_T *circBufPtr = (byte_T *)
      &CelpSimulink_DWork.OverlapAnalysisWindows_CircBuff[0];
    const byte_T *icPtr = (const byte_T *)&CelpSimulink_ConstP.pooled1;
    int_T i = 1;
    while (i-- > 0) {
      MWDSP_CopyScalarICs(circBufPtr, icPtr, 80, sizeof(real32_T));
      circBufPtr += bufLenBytes;
    }
  }

  /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S3>/Time-Varying Synthesis Filter' */
  /* All-pole, Direct-form (a0 ~= 1) */
  {
    real32_T *statePtr = (real32_T *)
      &CelpSimulink_DWork.TimeVaryingSynthesisFilter_FILT[0];

    /* Scalar expansion of ICs with extra zero element per channel */
    int_T chanCount = 80;
    while (chanCount--) {
      int_T numElems= 10;
      while (numElems--) {
        *statePtr++ = *(const real32_T *)&CelpSimulink_ConstP.pooled1;
      }

      *statePtr++ = 0.0F;
    }
  }

  /* Signal Processing Blockset Filter Implementation (sdspfilter2) - '<S3>/De-emphasis Filter' */
  /* All-pole, Direct-form (a0 == 1) */
  {
    real32_T *statePtr = (real32_T *)
      &CelpSimulink_DWork.DeemphasisFilter_FILT_STATES[0];

    /* Scalar expansion of ICs with extra zero element per channel */
    int_T chanCount = 80;
    while (chanCount--) {
      *statePtr++ = *(const real32_T *)&CelpSimulink_ConstP.pooled1;
      *statePtr++ = 0.0F;
    }
  }
}

void MdlStart(void)
{
  /* Start for iterator SubSystem: '<Root>/Codebook Search' */

  /* InitializeConditions for S-Function (sdspstatminmax): '<S6>/Maximum1' */
  CelpSimulink_DWork.Maximum1_Valdata = rtMinusInf;

  /* InitializeConditions for UnitDelay: '<S5>/Unit Delay' */
  CelpSimulink_DWork.UnitDelay_DSTATE = CelpSimulink_P.UnitDelay_X0;

  /* end of Start for SubSystem: '<Root>/Codebook Search' */
  {
    /* Signal Processing Blockset ToAudioDevice (sdspToAudioDevice) - '<Root>/To Audio Device' - Start */
    void *device = NULL;
    AudioDeviceLibrary adl;
    static char sErr[512];
    sErr[0] = 0;
    adl = CreateAudioDeviceLibrary(&sErr[0]);
    memcpy(&CelpSimulink_DWork.ToAudioDevice_AudioDeviceLib[0], &adl, sizeof
           (AudioDeviceLibrary));
    if (!*sErr) {                      /* Create the device */
      adl.deviceCreate(&sErr[0], NULL, "Default", 1, &device,
                       80, 8000.0, 1, 512, 8000.0,
                       1, 1);
    }

    if (!*sErr) {
      adl.deviceStart(device, &sErr[0]);
    }

    if (*sErr) {
      DestroyAudioDeviceLibrary(&adl);
      rtmSetErrorStatus(CelpSimulink_M, sErr);
      rtmSetStopRequested(CelpSimulink_M, 1);
    }

    CelpSimulink_DWork.ToAudioDevice_AudioDevice = device;
  }

  MdlInitialize();
}

RT_MODEL_CelpSimulink *CelpSimulink(void)
{
  CelpSimulink_initialize(1);
  return CelpSimulink_M;
}

void MdlTerminate(void)
{
  CelpSimulink_terminate();
}

/*========================================================================*
 * End of GRT compatible call interface                                   *
 *========================================================================*/
