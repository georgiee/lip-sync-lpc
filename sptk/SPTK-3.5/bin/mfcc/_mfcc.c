/* ----------------------------------------------------------------- */
/*             The Speech Signal Processing Toolkit (SPTK)           */
/*             developed by SPTK Working Group                       */
/*             http://sp-tk.sourceforge.net/                         */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 1984-2007  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/*                1996-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the SPTK working group nor the names of its */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/****************************************************************

    $Id: _mfcc.c,v 1.4 2011/12/19 06:00:35 mataki Exp $

    Mel-Frequency Cepstral Analysis

        void mfcc(in, mc, sampleFreq, alpha, eps, wlng, flng
                  m, n, ceplift, dftmode, usehamming);

        double  *in        : input sequence
        double  *mc        : mel-frequency cepstral coefficients
        double  sampleFreq : sample frequency
        double  alpha      : pre-emphasis coefficient
        double  eps        : epsilon
        int     wlng       : frame length of input sequence
        int     flng       : frame length for fft
        int     m          : order of cepstrum
        int     n          : number of channel for mel-filter bank
        int     ceplift    : liftering coefficients
        Boolean dftmode    : use dft
        Boolean usehamming : use hamming window

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

#define MEL 1127.01048
#define EZERO (-1.0E10)


/* workspace for dct*/
static int dct_table_size = 0;
static float *dct_workspace = NULL;
static float *pLocalReal = NULL;
static float *pLocalImag = NULL;
static float *pWeightReal = NULL;
static float *pWeightImag = NULL;

static int dct_table_size_fft = 0;
static double *dct_workspace2 = NULL;
static double *pLocalReal2 = NULL;
static double *pLocalImag2 = NULL;
static double *pWeightReal2 = NULL;
static double *pWeightImag2 = NULL;


double Freq2Mel(double freq)
{
   return MEL * log(freq / 700.0 + 1.0);
}

double Sample2Mel(int sample, int num, double fs)
{
   double freq;
   freq = (double) (sample + 1) / (double) (num) * (fs / 2.0);

   return Freq2Mel(freq);
}

double CalcEnergy(double *x, const double eps, const int leng)
{
   int k;
   double energy = 0.0;
   for (k = 0; k < leng; k++)
      energy += x[k] * x[k];

   return ((energy <= 0) ? EZERO : log(energy));
}

void Hamming(double *x, const int leng)
{
   int k;
   double arg;

   arg = M_2PI / (leng - 1);
   for (k = 0; k < leng; k++)
      x[k] *= (0.54 - 0.46 * cos(k * arg));
}

int dft(float *pReal, float *pImag, const int nDFTLength)
{
   float *pTempReal;
   float *pTempImag;
   int k, n;
   double dTempReal, dTempImag;

   pTempReal = fgetmem(nDFTLength);
   pTempImag = fgetmem(nDFTLength);

   memcpy(pTempReal, pReal, sizeof(float) * nDFTLength);
   memcpy(pTempImag, pImag, sizeof(float) * nDFTLength);

   for (k = 0; k < nDFTLength; k++) {
      dTempReal = 0;
      dTempImag = 0;
      for (n = 0; n < nDFTLength; n++) {
         dTempReal += pTempReal[n] *
             cos(2.0 * PI * n * k / (double) nDFTLength) +
             pTempImag[n] * sin(2.0 * PI * n * k / (double) nDFTLength);
         dTempImag +=
             -pTempReal[n] *
             sin(2.0 * PI * n * k / (double) nDFTLength) +
             pTempImag[n] * cos(2.0 * PI * n * k / (double) nDFTLength);
      }
      pReal[k] = dTempReal;
      pImag[k] = dTempImag;

   }
   free(pTempReal);
   free(pTempImag);
}

/* nSize <= 0 : release resources  */
/*       >  0 : create cosine table of which size is 'nSize' */

int dct_create_table_fft(const int nSize)
{
   register int k, n;

   if (nSize == dct_table_size_fft) {
      /* no needs to resize workspace. */
      return (0);
   } else {
      /* release resources to resize workspace. */
      if (dct_workspace2 != NULL) {
         free(dct_workspace2);
         dct_workspace2 = NULL;
      }
      pLocalReal2 = NULL;
      pLocalImag2 = NULL;
      pWeightReal2 = NULL;
      pWeightImag2 = NULL;
   }

   /* getting resources. */
   if (nSize <= 0) {
      dct_table_size_fft = 0;
      return (0);
   } else {
      dct_table_size_fft = nSize;
      dct_workspace2 = dgetmem(dct_table_size_fft * 6);
      pWeightReal2 = dct_workspace2;
      pWeightImag2 = dct_workspace2 + dct_table_size_fft;
      pLocalReal2 = dct_workspace2 + (2 * dct_table_size_fft);
      pLocalImag2 = dct_workspace2 + (4 * dct_table_size_fft);

      for (k = 0; k < dct_table_size_fft; k++) {
         pWeightReal2[k] =
             cos(k * PI / (2.0 * dct_table_size_fft)) /
             sqrt(2.0 * dct_table_size_fft);
         pWeightImag2[k] =
             -sin(k * PI / (2.0 * dct_table_size_fft)) /
             sqrt(2.0 * dct_table_size_fft);
      }
      pWeightReal2[0] /= sqrt(2.0);
      pWeightImag2[0] /= sqrt(2.0);
   }

}

int dct_create_table(const int nSize)
{
   register int k, n;

   if (nSize == dct_table_size) {
      /* no needs to resize workspace. */
      return (0);
   } else {
      /* release resources to resize workspace. */
      if (dct_workspace != NULL) {
         free(dct_workspace);
         dct_workspace = NULL;
      }
      pLocalReal = NULL;
      pLocalImag = NULL;
      pWeightReal = NULL;
      pWeightImag = NULL;
   }

   /* getting resources. */
   if (nSize <= 0) {
      dct_table_size = 0;
      return (0);
   } else {
      dct_table_size = nSize;
      dct_workspace = fgetmem(dct_table_size * 6);
      pWeightReal = dct_workspace;
      pWeightImag = dct_workspace + dct_table_size;
      pLocalReal = dct_workspace + (2 * dct_table_size);
      pLocalImag = dct_workspace + (4 * dct_table_size);

      for (k = 0; k < dct_table_size; k++) {
         pWeightReal[k] =
             cos(k * PI / (2.0 * dct_table_size)) / sqrt(2.0 * dct_table_size);
         pWeightImag[k] =
             -sin(k * PI / (2.0 * dct_table_size)) / sqrt(2.0 * dct_table_size);
      }
      pWeightReal[0] /= sqrt(2.0);
      pWeightImag[0] /= sqrt(2.0);
   }

}

int dct_based_on_fft(float *pReal, float *pImag, const float *pInReal,
                     const float *pInImag)
{
   register int n, k;


   for (n = 0; n < dct_table_size_fft; n++) {
      pLocalReal2[n] = (double) pInReal[n];
      pLocalImag2[n] = (double) pInImag[n];
      pLocalReal2[dct_table_size_fft + n] =
          (double) pInReal[dct_table_size_fft - 1 - n];
      pLocalImag2[dct_table_size_fft + n] =
          (double) pInImag[dct_table_size_fft - 1 - n];
   }


   fft(pLocalReal2, pLocalImag2, dct_table_size_fft * 2);       /* double input */


   for (k = 0; k < dct_table_size_fft; k++) {
      pReal[k] = (float)
          (pLocalReal2[k] * pWeightReal2[k] - pLocalImag2[k] * pWeightImag2[k]);
      pImag[k] = (float)
          (pLocalReal2[k] * pWeightImag2[k] + pLocalImag2[k] * pWeightReal2[k]);
   }


}


int dct_based_on_dft(float *pReal, float *pImag, const float *pInReal,
                     const float *pInImag)
{
   register int n, k;

   for (n = 0; n < dct_table_size; n++) {
      pLocalReal[n] = pInReal[n];
      pLocalImag[n] = pInImag[n];
      pLocalReal[dct_table_size + n] = pInReal[dct_table_size - 1 - n];
      pLocalImag[dct_table_size + n] = pInImag[dct_table_size - 1 - n];
   }

   dft(pLocalReal, pLocalImag, dct_table_size * 2);


   for (k = 0; k < dct_table_size; k++) {
      pReal[k] =
          pLocalReal[k] * pWeightReal[k] - pLocalImag[k] * pWeightImag[k];
      pImag[k] =
          pLocalReal[k] * pWeightImag[k] + pLocalImag[k] * pWeightReal[k];
   }
}

void preEmphasise(double *x, double *y, const double alpha, const int leng)
{
   int k;
   y[0] = x[0] * (1.0 - alpha);
   for (k = 1; k < leng; k++)
      y[k] = x[k] - x[k - 1] * alpha;
}

void spec(double *x, double *sp, const int leng)
{
   int k, no;
   double *y, *mag;

   no = leng / 2;

   y = dgetmem(leng + no);
   mag = y + leng;

   fftr(x, y, leng);
   for (k = 1; k < no; k++) {
      mag[k] = x[k] * x[k] + y[k] * y[k];
      sp[k] = sqrt(mag[k]);
   }
   free(y);
}

void fbank(double *x, double *fb, const double eps, const double fs,
           const int leng, const int n)
{
   int i, k, l, fnum, no, startNum, chanNum = 0;
   int *noMel;
   double *w, *countMel;
   double maxMel, startFreq, endFreq, kMel;

   no = leng / 2;
   noMel = (int *) getmem((size_t) no, sizeof(int));
   countMel = dgetmem(n + 1 + no);
   w = countMel + n + 1;
   maxMel = Freq2Mel(fs / 2.0);

   for (k = 0; k <= n; k++)
      countMel[k] = (double) (k + 1) / (double) (n + 1) * maxMel;
   for (k = 1; k < no; k++) {
      kMel = Sample2Mel(k - 1, no, fs);
      while (countMel[chanNum] < kMel && chanNum <= n)
         chanNum++;
      noMel[k] = chanNum;
   }

   for (k = 1; k < no; k++) {
      chanNum = noMel[k];
      kMel = Sample2Mel(k - 1, no, fs);
      w[k] = (countMel[chanNum] - kMel) / (countMel[0]);
   }

   for (k = 1; k < no; k++) {
      fnum = noMel[k];
      if (fnum > 0)
         fb[fnum] += x[k] * w[k];
      if (fnum <= n)
         fb[fnum + 1] += (1 - w[k]) * x[k];
   }

   free(noMel);
   free(countMel);

   for (k = 1; k <= n; k++) {
      if (fb[k] < eps)
         fb[k] = eps;
      fb[k] = log(fb[k]);
   }
}


void dct(double *in, double *d, const int size, const int m,
         const Boolean dftmode)
{
   char *s, c;
   int dct_create_table_fft(const int nSize);
   int dct_based_on_fft(float *pReal, float *pImag, const float *pInReal,
                        const float *pInImag);
   int dft(float *pReal, float *pImag, const int nDFTLength);
   int dct_create_table(const int nSize);
   int dct_based_on_dft(float *pReal, float *pImag, const float *pInReal,
                        const float *pInImag);

   float *pReal, *pImag;
   int k, n, i, j, iter;

   double *x, *y, *dgetmem();
   int size2;
   float *x2, *y2;

   x = dgetmem(size2 = size + size);
   y = x + size;
   pReal = fgetmem(size2);
   pImag = pReal + size;
   x2 = fgetmem(size2);
   y2 = x2 + size;

   for (k = 0; k < size; k++) {
      x[k] = in[k + 1];
      x2[k] = (float) x[k];
      y2[k] = (float) y[k];
   }

   iter = 0;
   i = size;
   while ((i /= 2) != 0) {
      iter++;
   }
   j = 1;
   for (i = 1; i <= iter; i++) {
      j *= 2;
   }
   if (size != j || dftmode) {
      dct_create_table(size);
      dct_based_on_dft(pReal, pImag, (const float *) x2, (const float *) y2);
   } else {
      dct_create_table_fft(size);
      dct_based_on_fft(pReal, pImag, (const float *) x2, (const float *) y2);
   }

   for (k = 0; k < m; k++) {
      d[k] = (double) pReal[k];
   }
}

void CepLifter(double *x, double *y, const int m, const int leng)
{
   int k;
   double theta;
   for (k = 0; k < m; k++) {
      theta = PI * (double) k / (double) leng;
      y[k] = (1.0 + (double) leng / 2.0 * sin(theta)) * x[k];
   }
}

void mfcc(double *in, double *mc, const double sampleFreq, const double alpha,
          const double eps, const int wlng, const int flng, const int m,
          const int n, const int ceplift, const Boolean dftmode,
          const Boolean usehamming)
{
   static double *x = NULL, *px, *wx, *sp, *fb, *dc;
   double energy = 0.0, c0 = 0.0;
   int k, size = wlng;

   if (x == NULL) {
      x = dgetmem(wlng + wlng + flng + flng + n + 1 + m + 1);
      px = x + wlng;
      wx = px + wlng;
      sp = wx + flng;
      fb = sp + flng;
      dc = fb + n + 1;
   } else {
      free(x);
      x = dgetmem(wlng + wlng + flng + flng + n + 1 + m + 1);
      px = x + wlng;
      wx = px + wlng;
      sp = wx + flng;
      fb = sp + flng;
      dc = fb + n + 1;
      size = wlng;
   }

   movem(in, x, sizeof(*in), wlng);
   /* calculate energy */
   energy = CalcEnergy(x, eps, wlng);
   preEmphasise(x, px, alpha, wlng);
   /* apply hamming window */
   if (usehamming)
      Hamming(px, wlng);
   for (k = 0; k < wlng; k++)
      wx[k] = px[k];
   spec(wx, sp, flng);
   fbank(sp, fb, eps, sampleFreq, flng, n);
   /* calculate 0'th coefficient */
   for (k = 1; k <= n; k++)
      c0 += fb[k];
   c0 *= sqrt(2.0 / (double) n);
   dct(fb, dc, n, m, dftmode);
   /* liftering */
   if (ceplift > 0)
      CepLifter(dc, mc, m, ceplift);
   else
      movem(dc, mc, sizeof(*dc), m);

   for (k = 0; k < m - 1; k++)
      mc[k] = mc[k + 1];
   mc[m - 1] = c0;
   mc[m] = energy;

}
