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
*    Binary File Cut                                                    *
*                                                                       *
*               1990.3  K.Tokuda                                        *
*               1996.5  K.Koishida                                      *
*                                                                       *
*       usage:                                                          *
*               bcut [options] [infile] > stdout                        *
*       options:                                                        *
*               -s s     :  start number                        [0]     *
*               -e e     :  end number                          [EOF]   *
*               -l l     :  block length                        [1]     *
*               -n n     :  block order                         [l-1]   *
*               +type    :  data type                           [f]     *
*                           c (char)           C  (unsigned char)       *
*                           s (short)          S  (unsigned short)      *
*                           i (int)            I  (unsigned int)        *
*                           i3 (int, 3byte)    I3 (unsigned int, 3byte) *
*                           l (long)           L  (unsigned long)       *
*                           le (long long)     LE (unsigned long long)  *
*                           f (float)          d  (double)              *
*                           de (long double)                            *
*       infile:                                                         *
*               data sequence                                   [stdin] *
*       stdout:                                                         *
*               cut data sequence                                       *
*       note:                                                           *
*               When both -l and -n are specified,                      *
*               latter argument is adopted.                             *
*                                                                       *
************************************************************************/

static char *rcs_id = "$Id: bcut.c,v 1.22 2011/04/27 13:46:38 mataki Exp $";


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

#if defined(WIN32)
#  include "SPTK.h"
#else
#  include <SPTK.h>
#endif

/*  Default Values  */
#define START 0
#define END -1
#define LENG 1

#define SIGNED_INT3 FA
#define UNSIGNED_INT3 FA

/*  Command Name  */
char *cmnd;


void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - binary file cut\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -s s  : start number [%d]\n", START);
   fprintf(stderr, "       -e e  : end number   [EOF]\n");
   fprintf(stderr, "       -l l  : block length [%d]\n", LENG);
   fprintf(stderr, "       -n n  : block order  [l-1]\n");
   fprintf(stderr, "       +type : data type    [f]\n");
   fprintf(stderr,
           "                c  (char, %dbyte)         C  (unsigned char, %dbyte)\n",
           sizeof(char), sizeof(unsigned char));
   fprintf(stderr,
           "                s  (short, %dbyte)        S  (unsigned short, %dbyte)\n",
           sizeof(short), sizeof(unsigned short));
   fprintf(stderr,
           "                i3 (int, 3byte)          I3 (unsigned int, 3byte)\n");
   fprintf(stderr,
           "                i  (int, %dbyte)          I  (unsigned int, %dbyte)\n",
           sizeof(int), sizeof(unsigned int));
   fprintf(stderr,
           "                l  (long, %dbyte)         L  (unsigned long, %dbyte)\n",
           sizeof(long), sizeof(unsigned long));
   fprintf(stderr,
           "                le (long long, %dbyte)    LE (unsigned long long, %dbyte)\n",
           sizeof(long long), sizeof(unsigned long long));
   fprintf(stderr,
           "                f  (float, %dbyte)        d  (double, %dbyte)\n",
           sizeof(float), sizeof(double));
   fprintf(stderr,
           "                de (long double, %dbyte)\n", sizeof(long double));
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       data sequence        [stdin]\n");
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       cut data sequence\n");
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

int main(int argc, char **argv)
{
   size_t size = sizeof(float);
   int n = LENG;
   long long start = START, end = END, ptr;
   FILE *fp = stdin;
   char *s, c;
   Boolean int3flg = SIGNED_INT3, uint3flg = UNSIGNED_INT3;
   long int y = 0;
   long long xl = 0;
   unsigned long long xul = 0;
   long double xd = 0.0;
   long double x;
   char *tmp;

   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;
   while (--argc)
      if (*(s = *++argv) == '-') {
         c = *++s;
         switch (c) {
         case 's':
            start = atol(*++argv);
            --argc;
            break;
         case 'e':
            end = atol(*++argv);
            --argc;
            break;
         case 'l':
            n = atol(*++argv);
            --argc;
            break;
         case 'n':
            n = atol(*++argv) + 1;
            --argc;
            break;
         case 'h':
            usage(0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else if (*s == '+') {
         c = *++s;
         switch (c) {
         case 'c':
            size = sizeof(char);
            break;
         case 's':
            size = sizeof(short);
            break;
         case 'i':
            if (*(s + 1) == '3') {
               size = 3;
               int3flg = TR;
               (*argv)++;
            } else {
               size = sizeof(int);
            }
            break;
         case 'l':
            if (*(s + 1) == 'e') {
               size = sizeof(long long);
               (*argv)++;
            } else {
               size = sizeof(long);
            }
            break;
         case 'C':
            size = sizeof(unsigned char);
            break;
         case 'S':
            size = sizeof(unsigned short);
            break;
         case 'I':
            if (*(s + 1) == '3') {
               size = 3;
               uint3flg = TR;
               (*argv)++;
            } else {
               size = sizeof(unsigned int);
            }
            break;
         case 'L':
            if (*(s + 1) == 'E') {
               size = sizeof(unsigned long long);
               (*argv)++;
            } else {
               size = sizeof(unsigned long);
            }
            break;
         case 'f':
            size = sizeof(float);
            break;
         case 'd':
            if (*(s + 1) == 'e') {
               size = sizeof(long double);
               (*argv)++;
            } else {
               size = sizeof(double);
            }
            break;
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else
         fp = getfp(*argv, "rb");

   ptr = start * n;
   while (ptr--)
      if (freadx(&x, size, 1, fp) != 1)
         return (0);

   ptr = (end - start + 1) * n;
   while (end == -1 || ptr--) {
      if (freadx(&x, size, 1, fp) != 1)
         break;
      if (int3flg == TR || uint3flg == TR) {
         y = *(int *) &x & 0x00FFFFFF;
         if (int3flg == TR && y >> 23 == 1)
            y = y | 0xFF000000;
         fwritex(&y, size, 1, stdout);
      } else
         fwritex(&x, size, 1, stdout);
   }

   return (0);
}
