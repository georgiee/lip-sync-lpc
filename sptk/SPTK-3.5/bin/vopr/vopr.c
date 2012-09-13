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
*    Execute Vector Operations                                          *
*                                                                       *
*                                               1988.6  T.Kobayashi     *
*                                               1996.5  K.Koishida      *
*       usage:                                                          *
*               vopr [ options ] [ [ file1 ] [ infile ]  > stdout       *
*       options:                                                        *
*               -l l     :  length of vector                [1]         *
*               -n n     :  order of vector                 [l-1]       *
*               -i       :  specified file contains a and b [FALSE]     *
*               -a       :  addition       (a + b)          [FALSE]     * 
*               -s       :  subtraction    (a - b)          [FALSE]     *
*               -m       :  multiplication (a * b)          [FALSE]     *
*               -d       :  division       (a / b)          [FALSE]     *
*               -ATAN2   :  atan2          atan2(b,a)       [FALSE]     *
*        notice:                                                        *
*               When both -l and -n are specified,                      *
*               latter argument is adopted.                             *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: vopr.c,v 1.23 2011/04/27 13:46:44 mataki Exp $";


/*  Standard C Libraries  */
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
#include <ctype.h>
#include <math.h>

#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

/*  Default Values  */
#define LENG 1
#define INV  FA

char *BOOL[] = { "FALSE", "TRUE" };

/*  Command Name  */
char *cmnd;


void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - execute vector operations\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ file1 ] [ infile ] > stdout\n",
           cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -l l   : length of vector                [%d]\n",
           LENG);
   fprintf(stderr, "       -n n   : order of vector                 [l-1]\n");
   fprintf(stderr, "       -i     : specified file contains a and b [%s]\n",
           BOOL[INV]);
   fprintf(stderr, "       -a     : addition       (a + b)          [FALSE]\n");
   fprintf(stderr, "       -s     : subtraction    (a - b)          [FALSE]\n");
   fprintf(stderr, "       -m     : multiplication (a * b)          [FALSE]\n");
   fprintf(stderr, "       -d     : division       (a / b)          [FALSE]\n");
   fprintf(stderr, "       -ATAN2 : atan2          atan2(b,a)       [FALSE]\n");
   fprintf(stderr, "       -h     : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       data vectors (%s)                     [stdin]\n",
           FORMAT);
   fprintf(stderr, "  file1:\n");
   fprintf(stderr, "       data vectors (%s)\n", FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       data vectors after operations (float)\n");
   fprintf(stderr, "  note:\n");
   fprintf(stderr, "       When both -l and -n are specified,\n");
   fprintf(stderr, "       latter argument is adopted.\n");
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n", PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}


int opr = ' ', leng = LENG;
Boolean inv = INV;

int main(int argc, char **argv)
{
   int nfiles = 0;
   FILE *fp1, *fp2;
   char *s, c, *infile[4];
   int vopr(FILE * fp1, FILE * fp2);

   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc) {
      if (*(s = *++argv) == '-') {
         c = *++s;
         if (*++s == '\0' && (c == 'l' || c == 'n')) {
            s = *++argv;
            --argc;
         }
         switch (c) {
         case 'l':
            leng = atoi(s);
            break;
         case 'n':
            leng = atoi(s) + 1;
            break;
         case 'i':
            inv = 1 - inv;
            break;
         case 'a':
         case 'd':
         case 'm':
         case 's':
         case 'A':
            opr = c;
            break;
         case 'h':
            usage(0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else
         infile[nfiles++] = s;
   }

   if (nfiles == 0)
      vopr(stdin, stdin);
   else {
      fp1 = getfp(infile[0], "rb");
      if (nfiles == 1) {
         if (inv)
            vopr(fp1, fp1);
         else
            vopr(stdin, fp1);
      } else {
         fp2 = getfp(infile[1], "rb");
         vopr(fp1, fp2);
      }
   }

   return (0);
}

int vopr(FILE * fp1, FILE * fp2)
{
   double *a, *b;
   int k;

   a = dgetmem(leng + leng);
   b = a + leng;

   if (fp1 != fp2 && leng > 1) {
      if (freadf(b, sizeof(*b), leng, fp2) != leng)
         return (1);
   }
   while (freadf(a, sizeof(*a), leng, fp1) == leng) {
      if (fp1 == fp2 || leng == 1) {
         if (freadf(b, sizeof(*b), leng, fp2) != leng)
            return (1);
      }

      switch (opr) {
      case 'a':
         for (k = 0; k < leng; ++k)
            a[k] += b[k];
         break;
      case 's':
         for (k = 0; k < leng; ++k)
            a[k] -= b[k];
         break;
      case 'm':
         for (k = 0; k < leng; ++k)
            a[k] *= b[k];
         break;
      case 'd':
         for (k = 0; k < leng; ++k)
            a[k] /= b[k];
         break;
      case 'A':
         for (k = 0; k < leng; ++k)
            a[k] = atan2(b[k], a[k]);
         break;
      default:
         break;
      }

      fwritef(a, sizeof(*a), leng, stdout);
   }
   return (0);
}
