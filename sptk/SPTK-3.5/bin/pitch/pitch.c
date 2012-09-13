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

/************************************************************************
*                                                                       *
*    Pitch Extraction                                                   *
*                                                                       *
*                                      1998.7  M.Tamura                 *
*                                      2000.3  T.Tanaka                 *
*                                      2011.10 A.Tamamori               *
*                                      2011.11 T.Sawada                 *
*                                                                       *
*       usage:                                                          *
*               pitch [ options ] [ infile ] > stdout                   *
*       options:                                                        *
*               -a  a     :  algorithm used for pitch      [0]          *
*                            estimation                                 *
*                              0 (snack)                                *
*                              1 (swipe)                                *
*               -s  s     :  sampling frequency (Hz)       [16]         *
*               -p  p     :  frame shift                   [80]         *
*               -t  t     :  voiced/unvoiced threshold     [0.3]        *
*                            (used only for swipe algorithm)            *
*               -L  L     :  minimum fundamental frequency [60]         *
*                            to search for (Hz)                         *
*               -H  H     :  maximum fundamental frequency [240]        *
*                            to search for (Hz)                         *
*               -o  o     :  output format                 [0]          *
*                              0 (pitch)                                *
*                              1 (f0)                                   *
*                              2 (log(f0))                              *
*       infile:                                                         *
*               data sequence                                           *
*                       x(0), x(1), ..., x(n-1), ...                    *
*       stdout:                                                         *
*               pitch, f0, or log(f0)                                   *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: pitch.c,v 1.39 2011/12/19 12:47:27 mataki Exp $";


/*  Standard C Libraries  */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#  ifndef HAVE_STRRCHR
#     define strrchr rindex
#  endif
#endif


#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

/*  Default Values  */
#define LOW    60.0
#define HIGH   240.0
#define FRAME_SHIFT 80
#define SAMPLE_FREQ 16.0
#define ATYPE 0
#define OTYPE 0
#define STR_LEN 255
#define THRESH 0.3
#define NOISEMASK 50.0
#define SEED 1
#define RND_MAX 32767
#define FSP 10.0
#define ALPHA 0.00275
#define BETA_1 9600.0
#define BETA_2 168.0
#define BETA_3 96000.0

/*  Command Name  */
char *cmnd;

typedef struct _float_list {
   float f;
   struct _float_list *next;
} float_list;

static double rnd(unsigned long *next)
{
   double r;

   *next = *next * 1103515245L + 12345;
   r = (*next / 65536L) % 32768L;

   return (r / RND_MAX);
}

double nrandom(unsigned long *next)
{
   static int sw = 0;
   static double r1, r2, se;

   if (sw == 0) {
      sw = 1;
      do {
         r1 = 2 * rnd(next) - 1;
         r2 = 2 * rnd(next) - 1;
         se = r1 * r1 + r2 * r2;
      }
      while (se > 1 || se == 0);
      se = sqrt(-2 * log(se) / se);
      return (r1 * se);
   } else {
      sw = 0;
      return (r2 * se);
   }
}

void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - pitch extraction\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -a a  : algorithm used for pitch        [%d]\n",
           ATYPE);
   fprintf(stderr, "               estimation\n");
   fprintf(stderr, "                 0 (snack)\n");
   fprintf(stderr, "                 1 (swipe)\n");
   fprintf(stderr, "       -s s  : sampling frequency (kHz)        [%g]\n",
           SAMPLE_FREQ);
   fprintf(stderr, "       -p p  : frame shift                     [%d]\n",
           FRAME_SHIFT);
   fprintf(stderr, "       -t t  : voiced/unvoiced threshold       [%g]\n",
           THRESH);
   fprintf(stderr, "               (used only for swipe algorithm)\n");
   fprintf(stderr, "       -L L  : minimum fundamental             [%g]\n",
           LOW);
   fprintf(stderr, "               frequency to search for (Hz)\n");
   fprintf(stderr, "       -H H  : maximum fundamental             [%g]\n",
           HIGH);
   fprintf(stderr, "               frequency to search for (Hz)\n");
   fprintf(stderr, "       -o o  : output format                   [%d]\n",
           OTYPE);
   fprintf(stderr, "                 0 (pitch)\n");
   fprintf(stderr, "                 1 (f0)\n");
   fprintf(stderr, "                 2 (log(f0))\n");
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       waveform (%s)             \n", FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       pitch, f0 or log(f0) (%s)\n", FORMAT);
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n", PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}


int main(int argc, char **argv)
{
   int i, j, length, frame_shift = FRAME_SHIFT,
       atype = ATYPE, otype = OTYPE, alpha, beta, fnum;
   unsigned long next = SEED;
   double *x, thresh = THRESH, timestep, p, fsp, sample_freq = SAMPLE_FREQ, L =
       LOW, H = HIGH;
   FILE *fp = stdin;
   float_list *top, *cur, *prev;
   char *message = (char *) malloc(sizeof(char) * STR_LEN);
   char *cGet_f0(float_list * flist, float sample_freq,
                 int length, int frame_shift,
                 int minF0, int maxF0, int fnum, int otype);

   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc)
      if (**++argv == '-') {
         switch (*(*argv + 1)) {
         case 'a':
            atype = atoi(*++argv);
            --argc;
            break;
         case 's':
            sample_freq = atof(*++argv);
            --argc;
            break;
         case 'p':
            frame_shift = atoi(*++argv);
            --argc;
            break;
         case 't':
            thresh = atof(*++argv);
            --argc;
            break;
         case 'L':
            L = atof(*++argv);
            --argc;
            break;
         case 'H':
            H = atof(*++argv);
            --argc;
            break;
         case 'o':
            otype = atoi(*++argv);
            --argc;
            break;
         case 'h':
            usage(0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else {
         fp = getfp(*argv, "rb");
      }
   sample_freq *= 1000.0;

   x = dgetmem(1);
   top = prev = (float_list *) malloc(sizeof(float_list));
   length = 0;
   prev->next = NULL;
   while (freadf(x, sizeof(*x), 1, fp) == 1) {
      p = (double) nrandom(&next);
      cur = (float_list *) malloc(sizeof(float_list));
      if (atype == 0)
         cur->f = (float) x[0] + (float) (p * NOISEMASK);
      else
         cur->f = (float) x[0];
      length++;
      prev->next = cur;
      cur->next = NULL;
      prev = cur;
   }
   if (atype == 0) {
      fnum = (int) (ceil((double) length / (double) frame_shift));
      fsp = sample_freq * (FSP / (double) frame_shift);
      alpha = (int) (ALPHA * fsp + 0.5);
      beta = (int) ((BETA_1 / L - BETA_2) * fsp / BETA_3 + 0.5);
      if (beta < 0)
         beta = 0;
      for (i = 0; i < (alpha + beta + 3) * frame_shift; i++) {
         p = (double) nrandom(&next);
         cur = (float_list *) malloc(sizeof(float_list));
         cur->f = (float) (p * NOISEMASK);
         length++;
         prev->next = cur;
         cur->next = NULL;
         prev = cur;
      }
   }

   if (atype == 0) {
      message =
          cGet_f0(top->next, (float) sample_freq, length, frame_shift, (int) L,
                  (int) H, fnum, otype);
      if (message != NULL) {
         fprintf(stderr, "%s : %s\n", cmnd, message);
         usage(1);
      }
   } else {
      timestep = (double) frame_shift / sample_freq;
      swipe(top->next, length, L, H, thresh, timestep, sample_freq, otype);
   }
   return (0);
}
