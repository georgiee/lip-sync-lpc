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
*    Mel-cepstral analysis using 2nd order all-pass filter              *
*                                                                       *
*                                     1998.1  C.Miyajima                *
*                                                                       *
*       usage:                                                          *
*               smcep [ options ] [infile] > stdout                     *
*       options:                                                        *
*               -a a     :  all-pass constant                [0.35]     *
*               -t t     :  emphasized frequency t*pi(rad)   [0]        *
*               -m m     :  order of mel cepstrum            [25]       *
*               -l l     :  frame length                     [256]      *
*               -L L     :  ifft size for making matrices    [1024]     *
*               -q q     :  Input format                     [0]        *
*                             0 (windowed data sequence)                *
*                             1 (20*log|f(w)|)                          *
*                             2 (ln|f(w)|)                              *
*                             3 (|f(w)|)                                *
*                             4 (|f(w)|^2)                              *
*               (level 2)                                               *
*               -i i     :  minimum iteration                [2]        *
*               -j j     :  maximum iteration                [30]       *
*               -d d     :  end condition                    [0.001]    *
*               -e e     :  initial value for log-periodgram [0]        *
*               -f f     :  mimimum value of the determinant            *
*                           of the normal matrix            [0.000001]  *  
*      infile:                                                          *
*              data sequence                                            *
*                      , x(0), x(1), ..., x(L-1),                       *
*      stdout:                                                          *
*              mel cepstrum                                             *
*                      , c~(0), c~(1), ..., c~(M),                      *
*      require:                                                         *
*              smcep()                                                  *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: smcep.c,v 1.26 2011/04/27 13:46:43 mataki Exp $";


/*  Standard C Libralies  */
#include <stdio.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#  ifndef HAVE_STRRCHR
#     define strrchr rindex
#  endif
#endif

#include <stdlib.h>
#include <math.h>

#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

/*  Default Values  */
#define ALPHA  0.35
#define THETA  0.00
#define ORDER  25
#define FLENG  256
#define FFTSZ  256 * 4
#define ITYPE  0
#define MINITR 2
#define MAXITR 30
#define END    0.001
#define EPS    0.0
#define MINDET 0.000001

/*  Command Name  */
char *cmnd;


void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr,
           " %s - mel-cepstral analysis using 2nd order all-pass filter\n",
           cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -a a  : all-pass constant                [%g]\n",
           ALPHA);
   fprintf(stderr, "       -t t  : emphasized frequency  t*pi(rad)  [%g]\n",
           THETA);
   fprintf(stderr, "       -m m  : order of mel cepstrum            [%d]\n",
           ORDER);
   fprintf(stderr, "       -l l  : frame length                     [%d]\n",
           FLENG);
   fprintf(stderr, "       -L L  : ifft size for making matrices    [%d]\n",
           FFTSZ);
   fprintf(stderr, "       -q q  : input format                     [%d]\n",
           ITYPE);
   fprintf(stderr, "                 0 (windowed sequence\n");
   fprintf(stderr, "                 1 (20*log|f(w)|)\n");
   fprintf(stderr, "                 2 (ln|f(w)|)\n");
   fprintf(stderr, "                 3 (|f(w)|)\n");
   fprintf(stderr, "                 4 (|f(w)|)^2\n");
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "     (level 2)\n");
   fprintf(stderr, "       -i i  : minimum iteration                [%d]\n",
           MINITR);
   fprintf(stderr, "       -j j  : maximum iteration                [%d]\n",
           MAXITR);
   fprintf(stderr, "       -d d  : end condition                    [%g]\n",
           END);
   fprintf(stderr, "       -e e  : initial value for log-periodgram [%g]\n",
           EPS);
   fprintf(stderr, "       -f f  : mimimum value of the determinant [%g]\n",
           MINDET);
   fprintf(stderr, "               of the normal matrix\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       windowed sequences (%s)    [stdin]\n", FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       mel-cepstrum (%s)\n", FORMAT);
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
   int m = ORDER, flng = FLENG, itype = ITYPE, fftsz = FFTSZ, itr1 =
       MINITR, itr2 = MAXITR, flag = 0;
   FILE *fp = stdin;
   double *mc, *x, a = ALPHA, t = THETA, end = END, e = EPS, f = MINDET;

   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc)
      if (**++argv == '-') {
         switch (*(*argv + 1)) {
         case 'a':
            a = atof(*++argv);
            --argc;
            break;
         case 't':
            t = atof(*++argv);
            --argc;
            break;
         case 'm':
            m = atoi(*++argv);
            --argc;
            break;
         case 'l':
            flng = atoi(*++argv);
            --argc;
            break;
         case 'L':
            fftsz = atoi(*++argv);
            --argc;
            break;
         case 'q':
            itype = atoi(*++argv);
            --argc;
            break;
         case 'i':
            itr1 = atoi(*++argv);
            --argc;
            break;
         case 'j':
            itr2 = atoi(*++argv);
            --argc;
            break;
         case 'd':
            end = atof(*++argv);
            --argc;
            break;
         case 'e':
            e = atof(*++argv);
            --argc;
            break;
         case 'f':
            f = atof(*++argv);
            --argc;
            break;
         case 'h':
            usage(0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else
         fp = getfp(*argv, "rb");

   t *= M_PI;

   x = dgetmem(flng + m + 1);
   mc = x + flng;

   while (freadf(x, sizeof(*x), flng, fp) == flng) {
      flag = smcep(x, flng, mc, m, fftsz, a, t, itr1, itr2, end, e, f, itype);
      fwritef(mc, sizeof(*mc), m + 1, stdout);
   }

   return (0);
}
