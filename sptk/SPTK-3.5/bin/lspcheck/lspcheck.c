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
*    Check stability and rearrange LSP                                  *
*                                                                       *
*                                         1996.6  K.Koishida            *
*                                         2009.9  A.Saito               *
*       usage:                                                          *
*               lspcheck [ options ] [ infile ] > stdout                *
*       options:                                                        *
*               -m m     :  order of LSP                [25]            *
*               -s s     :  sampling frequency          [10]            *
*               -k       :  input & output gain         [FALSE]         *
*               -i i     :  input format                [0]             *
*               -o o     :  output format               [i]             *
*                             0 (normalized frequency <0...pi>)         *   
*                             1 (normalized frequency <0...0.5>)        *
*                             2 (frequency (kHz))                       *
*                             3 (frequency (Hz))                        *
*               -r r     :  rearrange LSP               [FALSE]         *
*                           distance between two consecutive LSPs       *
*                                                       [r]             *
*                           extend the distance (if it is smaller       *
*                           than [r]*pi/m)    s.t. (0 < [r] < 1)        *
*       infile:                                                         *
*               LSP                                                     *
*                       , f(1), ..., f(m),                              *
*       stdout:                                                         *
*               frame number of irregular LSP of                        *
*               rearranged LSP if -r option is specified                *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: lspcheck.c,v 1.27 2011/12/20 15:27:29 uratec Exp $";


/*  Standard C Libraries  */
#include <stdio.h>
#include <stdlib.h>

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
#define ORDER  25
#define ITYPE  0
#define OTYPE  -1
#define SAMPLING 10.0
#define ARRANGE  FA
#define GAIN  TR
#define ALPHA 0.0001

char *BOOL[] = { "FALSE", "TRUE" };

/*  Command Name  */
char *cmnd;


void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - check stability and rearrange LSP\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -m m  : order of LPC        [%d]\n", ORDER);
   fprintf(stderr, "       -s s  : sampling frequency  [%g]\n", SAMPLING);
   fprintf(stderr, "       -k    : input & output gain [%s]\n", BOOL[GAIN]);
   fprintf(stderr, "       -i i  : input format        [%d]\n", ITYPE);
   fprintf(stderr, "       -o o  : output format       [i]\n");
   fprintf(stderr, "                 0 (normalized frequency <0...pi>)\n");
   fprintf(stderr, "                 1 (normalized frequency <0...0.5>)\n");
   fprintf(stderr, "                 2 (frequency (kHz))\n");
   fprintf(stderr, "                 3 (frequency (Hz))\n");
   fprintf(stderr, "       -r r  : rearrange LSP       [%s]\n", BOOL[ARRANGE]);
   fprintf(stderr,
           "               check the distance between two consecutive LSPs\n");
   fprintf(stderr,
           "               and extend the distance (if it is smaller than [r]*pi/m)\n");
   fprintf(stderr, "               s.t. (0 < [r] < 1)\n");
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       LSP (%s)                 [stdin]\n", FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       frame number of irregular LSP or\n");
   fprintf(stderr, "       rearranged LSP (%s) if -r option is specified\n",
           FORMAT);
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
   int m = ORDER, itype = ITYPE, otype = OTYPE, i, num;
   Boolean arrange = ARRANGE, gain = GAIN;
   FILE *fp = stdin;
   double *lsp, *lsp1, alpha = ALPHA, sampling = SAMPLING;


   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc)
      if (**++argv == '-') {
         switch (*(*argv + 1)) {
         case 'm':
            m = atoi(*++argv);
            --argc;
            break;
         case 's':
            sampling = atof(*++argv);
            --argc;
            break;
         case 'i':
            itype = atoi(*++argv);
            --argc;
            break;
         case 'o':
            otype = atoi(*++argv);
            --argc;
            break;
         case 'r':
            alpha = atof(*++argv);
            if (alpha <= 0 || alpha >= 1) {
               fprintf(stderr, "%s : Invalid option 'r'!\n", cmnd);
               usage(0);
            }
            --argc;
            arrange = 1 - arrange;
            break;
         case 'k':
            gain = 1 - gain;
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
   if (otype < 0)
      otype = itype;

   lsp = dgetmem(m + m + gain);
   lsp1 = lsp + m + gain;


   num = 0;


   while (freadf(lsp, sizeof(*lsp), m + gain, fp) == m + gain) {
      if (itype == 0)
         for (i = gain; i < m + gain; i++)
            lsp1[i] = lsp[i] / PI2;
      else if (itype == 1)
         for (i = gain; i < m + gain; i++)
            lsp1[i] = lsp[i];
      else if (itype == 2 || itype == 3)
         for (i = gain; i < m + gain; i++)
            lsp1[i] = lsp[i] / sampling;

      if (itype == 3)
         for (i = gain; i < m + gain; i++)
            lsp1[i] = lsp[i] / 1000;

      if (lspcheck(lsp1 + gain, m) == -1) {
         if (!arrange) {
            fprintf(stderr, "[ unstable frame number : %d ]\n", num);
            for (i = 0; i < m + gain; i++)
               fprintf(stderr, "%f\n", lsp[i]);
            fprintf(stderr, "\n");
         }
      }
      if (arrange)
         lsparrange(lsp1 + gain, m, alpha, itype, sampling);

      if (otype == 0)
         for (i = gain; i < m + gain; i++)
            lsp1[i] *= PI2;
      else if (otype == 2 || otype == 3)
         for (i = gain; i < m + gain; i++)
            lsp1[i] *= sampling;

      if (otype == 3)
         for (i = gain; i < m + gain; i++)
            lsp1[i] *= 1000;

      if (gain == 1)
         lsp1[0] = lsp[0];

      fwritef(lsp1, sizeof(*lsp1), m + gain, stdout);
      num++;
   }
   putchar('\n');
   return (0);
}
