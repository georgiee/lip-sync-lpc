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

/***************************************************************************
*                                                                          *
*    Calculation of Principal Component Score                              *
*                                                                          *
*                                           2009.9 A.Tamamori              *
*                                                                          *
*       usage:                                                             *
*                pcas [ options ] pcafile [ infile ] > stdout              *
*       options:                                                           *
*                -l    : length of vector                       [3]        *
*                -p n  : order of pricipal component            [2]        *
*       infile:                                                            *
*                test data vectors                              [stdin]    *
*       pcafile:                                                           *
*                eigen vectors and mean vector of training data            *
*       stdout:                                                            *
*                principal component scores                                *
*                                                                          *
****************************************************************************/

static char *rcs_id = "$Id: pcas.c,v 1.3 2011/12/12 12:51:28 mataki Exp $";

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

/* Defalut Values */
#define LENG           3
#define PCA_ORDER      2

/* Command Name */
char *cmnd;

void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - Compute Pricipal Component Score\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] pcafile [ infile ] > stdout\n", cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr, "       -l L  : dimentinality of vectors         [%d]\n",
           LENG);
   fprintf(stderr, "       -n N  : order of principal component     [%d]\n",
           PCA_ORDER);
   fprintf(stderr, "       -h    : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "       test data vectors (float)       [stdin]\n");
   fprintf(stderr, "  pcafile:\n");
   fprintf(stderr, "       eigen vectors and mean vector (float)\n");
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       Principal component score (flaot)\n");
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n", PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}

double **malloc_matrix(int row, int col)
{
   double **m;
   double *mtmp;
   int i, j;

   mtmp = dgetmem(row * col);
   if ((m = (double **) malloc(sizeof(double *) * row)) == NULL) {
      fprintf(stderr, "Can't malloc in %s\n", cmnd);
      exit(EXIT_FAILURE);
   }
   for (i = 0; i < row; i++) {
      m[i] = &(mtmp[i * col]);
   }

   return m;
}

int main(int argc, char *argv[])
{
   FILE *fp = stdin, *fpca = NULL;
   int i, j, k, n = -1;
   int leng = LENG, total, order = PCA_ORDER;
   int eigen_num;
   double *mean = NULL;
   double *test_data = NULL;
   double **e_vec = NULL;
   double *z = NULL;            /* principal component score */

   if ((cmnd = strrchr(argv[0], '/')) == NULL)
      cmnd = argv[0];
   else
      cmnd++;

   while (--argc) {
      if ((**++argv) == '-') {
         switch (*(*argv + 1)) {
         case 'l':
            leng = atoi(*++argv);
            --argc;
            break;
         case 'n':
            order = atoi(*++argv);
            --argc;
            break;
         case 'h':
            usage(EXIT_SUCCESS);
         default:
            fprintf(stderr, "%s : Invalid option '%s'!\n", cmnd, *argv);
            usage(EXIT_FAILURE);
         }
      } else if (fpca == NULL)
         fpca = getfp(*argv, "rb");
      else
         fp = getfp(*argv, "rb");
   }

   if (order > leng) {
      fprintf(stderr, "\n %s (Error) order of pricipal component"
              " must be less than dimentionality of vector.\n", cmnd);
      usage(EXIT_FAILURE);
   }

   /* Read eigen vectors and mean vector */
   if (fpca == NULL) {
      fprintf(stderr, "\n %s (Error) PCA file name required.\n", cmnd);
      usage(EXIT_FAILURE);
   }

   /* Count number of eigen vectors and mean vector */
   fseek(fpca, 0L, SEEK_END);
   eigen_num = (int) (ftell(fpca) / (int) leng / (double) sizeof(float));
   rewind(fpca);

   e_vec = malloc_matrix(eigen_num, leng);
   mean = dgetmem(leng);

   freadf(mean, sizeof(double), leng, fpca);
   for (i = 0; i < eigen_num - 1; i++)
      freadf(e_vec[i], sizeof(double), leng, fpca);

   /* Count of test data vectors */
   fseek(fp, 0L, SEEK_END);
   total = (int) (ftell(fp) / (int) leng / (double) sizeof(float));
   rewind(fp);
   if (total == 0) {
      fprintf(stderr, "%s: No input data !\n", cmnd);
      usage(EXIT_FAILURE);
   }

   /* allocate memory for test data */
   test_data = dgetmem(leng * total);
   fillz(test_data, leng, sizeof(double));

   /* read test data */
   freadf(test_data, sizeof(double), leng * total, fp);


   /* allocate memory for pricipal component score */
   z = dgetmem(order * total);
   fillz(z, order, sizeof(double));

   /* calculate pricipal component score */
   for (i = 0; i < total; i++)
      for (j = 0; j < order; j++)
         for (k = 0; k < leng; k++)
            z[i * order + j] +=
                e_vec[j][k] * (test_data[i * leng + k] - mean[k]);

   /* output principal component score */
   for (i = 0; i < total; i++)
      fwritef(z + i, sizeof(*z), order, stdout);

   return 0;
}
