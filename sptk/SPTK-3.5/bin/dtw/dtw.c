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
*    Dynamic Time Warping                                                  *
*                                                                          *
*                                      2011.12 Akira Tamamori              *
*       usage:                                                             *
*               dtw [ options ] reffile [ infile ] > stdout                *
*       options:                                                           *
*               -m M      : order of vector                        [0]     *
*               -l L      : length of vector                       [m+1]   *
*               -t T      : number of test vectors                 [N/A]   *
*               -r R      : number of reference vectors            [N/A]   *
*               -n N      : type of norm use for calculation       [2]     *
*                           of local distance                              *
*                             1 : L1-norm                                  *
*                             2 : L2-norm                                  *
*               -p P      : local path constraint                  [5]     *
*               -s sfile  : output score of dynamic time warping   [FALSE] *
*               -v vfile  : output concatenated test/reference     [FALSE] *
*                           data sequence along the Vitebi path            *
*       infile:                                                            *
*              test vector sequence                                        *
*                  x_1(1), ..., x_1(L), x_2(1), ..., x_2(L), ...           *
*       reffile:                                                           *
*              reference vector sequence                                   *
*                  y_1(1), ..., y_1(L), y_2(1), ..., y_2(L), ...           *
*       stdout:                                                            *
*              concatenated test/reference vector sequence                 *
*              along the Vitebi path                                       *
*                  x_1(1), ..., x_1(L), y_1(1), ..., y_1(L), ...           *
*                                                                          *
***************************************************************************/

static char *rcs_id = "$Id: dtw.c,v 1.2 2011/12/19 06:00:34 mataki Exp $";

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
#define  LENG  1
#define  T     -1

/*  Command Name  */
char *cmnd;

#define PATH_NG FA
#define PATH_OK TR

enum Norm { L1 = 1, L2 };
enum PATH { I = 1, II, III, IV, V, VI, VII };

typedef struct _DTW_Cell {
   double local;                /* local cost */
   double global;               /* global cost */
   int backptr[2];              /* back pointer for Viterbi path */
   Boolean is_region;
} DTW_Cell;

typedef struct _DATA {
   double *input;
   int total;
   int dim;                     /* dimension of input vector */
   int *viterbi;                /* Viterbi path */
} DATA;

typedef struct _WEIGHT {
   double *val;                 /* weight on the local path */
} WEIGHT;

typedef struct _DTW_Table {
   DTW_Cell **cell;
   DATA data[2];                /* two comparative data */
   int vit_leng;                /* length of Viterbi path */
   enum PATH path;              /* type of local constraint */
   WEIGHT weight;
} DTW_Table;

typedef struct _float_list {
   float *f;
   struct _float_list *next;
} float_list;

static int ROUND(double dat);
void InitDTW(DTW_Table * table, int leng,
             double *input, double *input2,
             int total1, int total2, enum PATH path);
double *DTW(DTW_Table * table, enum Norm norm_type);
void CalcLocalCost(DTW_Table * table, enum Norm norm_type);
void CheckEnabledRegion(DTW_Table * table);
void CheckEnabledRegion_TYPE_I(DTW_Table * table);
void CheckEnabledRegion_TYPE_II(DTW_Table * table);
void CheckEnabledRegion_TYPE_III(DTW_Table * table);
void CheckEnabledRegion_TYPE_IV(DTW_Table * table);
void CheckEnabledRegion_TYPE_V(DTW_Table * table);
void CheckEnabledRegion_TYPE_VI(DTW_Table * table);
void CheckEnabledRegion_TYPE_VII(DTW_Table * table);
void RecursiveCalc(DTW_Table * table);
void RecursiveCalc_TYPE_I(DTW_Table * table);
void RecursiveCalc_TYPE_II(DTW_Table * table);
void RecursiveCalc_TYPE_III(DTW_Table * table);
void RecursiveCalc_TYPE_IV(DTW_Table * table);
void RecursiveCalc_TYPE_V(DTW_Table * table);
void RecursiveCalc_TYPE_VI(DTW_Table * table);
void RecursiveCalc_TYPE_VII(DTW_Table * table);
static void BackTrace(DTW_Table * table);
static double *Concat(DTW_Table * table);

static int ROUND(double dat)
{
   return (int) (dat + 0.5);
}

void InitDTW(DTW_Table * table, int leng,
             double *input1, double *input2,
             int total1, int total2, enum PATH path)
{

   DTW_Cell *tmpcell = NULL;
   int i, j, k, tmp;
   int size[2] = { total1, total2 };
   void usage(int status);

   table->data[0].input = input1;
   table->data[1].input = input2;

   table->cell = (DTW_Cell **) malloc(sizeof(DTW_Cell *) * size[0]);
   if (table->cell == NULL) {
      fprintf(stderr, "ERROR: Can't allocate memory at InitDTW() !\n");
      fflush(stderr);
      usage(EXIT_FAILURE);
   }
   tmpcell = (DTW_Cell *) malloc(sizeof(DTW_Cell) * size[0] * size[1]);
   if (tmpcell == NULL) {
      fprintf(stderr, "ERROR: Can't allocate memory at InitDTW() !\n");
      fflush(stderr);
      usage(EXIT_FAILURE);
   }

   for (i = 0, j = 0; i < size[0]; i++, j += size[1]) {
      table->cell[i] = tmpcell + j;
      for (k = 0; k < size[1]; k++) {
         table->cell[i][k].is_region = PATH_OK;
      }
   }

   for (i = 0; i < 2; i++) {
      table->data[i].total = size[i];
      table->data[i].dim = leng;
      table->data[i].viterbi =
          (int *) malloc(sizeof(int) * (size[0] + size[1]));
      if (table->data[i].viterbi == NULL) {
         fprintf(stderr, "ERROR: Can't allocate memory at InitDTW() !\n");
         fflush(stderr);
         usage(EXIT_FAILURE);
      }
   }

   table->path = path;

   switch (table->path) {
   case I:
      table->weight.val = dgetmem(2);
      table->weight.val[0] = 1.0;
      table->weight.val[1] = 1.0;
      break;
   case II:
      table->weight.val = dgetmem(3);
      table->weight.val[0] = 1.0;
      table->weight.val[1] = 2.0;
      table->weight.val[2] = 1.0;
      break;
   case III:
      table->weight.val = dgetmem(2);
      table->weight.val[0] = 1.0;
      table->weight.val[1] = 2.0;
      break;
   case IV:
      table->weight.val = dgetmem(3);
      table->weight.val[0] = 1.0;
      table->weight.val[1] = 2.0;
      table->weight.val[2] = 3.0;
      break;
   case V:
      table->weight.val = dgetmem(5);
      table->weight.val[0] = 2.0;
      table->weight.val[1] = 1.0;
      table->weight.val[2] = 2.0;
      table->weight.val[3] = 2.0;
      table->weight.val[4] = 1.0;
      break;
   case VI:
      table->weight.val = dgetmem(3);
      table->weight.val[0] = 3.0;
      table->weight.val[1] = 2.0;
      table->weight.val[2] = 3.0;
      break;
   case VII:
      table->weight.val = dgetmem(6);
      table->weight.val[0] = 2.0;
      table->weight.val[1] = 1.0;
      table->weight.val[2] = 2.0;
      table->weight.val[3] = 2.0;
      table->weight.val[4] = 2.0;
      table->weight.val[5] = 3.0;
      break;
   default:
      break;
   }
}

double *DTW(DTW_Table * table, enum Norm norm_type)
{
   CheckEnabledRegion(table);

   CalcLocalCost(table, norm_type);

   RecursiveCalc(table);

   BackTrace(table);

   return (Concat(table));
}

void CheckEnabledRegion_TYPE_I(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;
   for (j = 0; j < Ty; j++) {
      for (i = 0; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
   }
}

void CheckEnabledRegion_TYPE_II(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;
   for (j = 0; j < Ty; j++) {
      for (i = 0; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
   }
}

void CheckEnabledRegion_TYPE_III(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;

   table->cell[0][0].is_region = PATH_OK;

   for (j = 1; j < Ty; j++) {
      table->cell[0][j].is_region = PATH_NG;
   }
   for (i = 1; i <= Tx - Ty; i++) {
      table->cell[i][0].is_region = PATH_OK;
   }
   for (; i < Tx; i++) {
      table->cell[i][0].is_region = PATH_NG;
   }
   for (j = 1; j < Ty; j++) {
      for (i = 1; i < j; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
      for (i = j; i <= Tx - Ty + j; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
      for (; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
}

void CheckEnabledRegion_TYPE_IV(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;

   table->cell[0][0].is_region = PATH_OK;

   for (j = 1; j < Ty; j++) {
      table->cell[0][j].is_region = PATH_NG;
   }

   for (i = 1; i < Tx - Ty / 2; i++) {
      table->cell[i][0].is_region = PATH_OK;
   }
   for (; i < Tx; i++) {
      table->cell[i][0].is_region = PATH_NG;
   }
   for (i = 1; i < Tx - Ty / 2 + 1; i++) {
      table->cell[i][1].is_region = PATH_OK;
   }
   for (; i < Tx; i++) {
      table->cell[i][1].is_region = PATH_NG;
   }

   for (j = 2; j < Ty; j++) {
      for (i = 0; i < ROUND((double) j / 2); i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
      for (; i < Tx - Ty / 2 + ROUND((double) j / 2); i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
      for (; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   table->cell[Tx - 1][Ty - 1].is_region = PATH_OK;
}

void CheckEnabledRegion_TYPE_V(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;

   table->cell[0][0].is_region = PATH_OK;

   for (j = 1; j < Ty; j++) {
      for (i = 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
   }

   for (i = 1; i < Tx; i++) {
      table->cell[i][0].is_region = PATH_NG;
   }

   for (j = 1; j < Ty; j++) {
      table->cell[0][j].is_region = PATH_NG;
   }

   for (j = 1; j < Ty - 2; j++) {
      for (i = 2 * j + 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = 1; i < Tx - 2; i++) {
      for (j = 2 * i + 1; j < Ty; j++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }

   /* Backward pruning */
   for (j = Ty - 1; j > 0; j--) {
      for (i = Tx - 1 + 2 * (j - Ty) + 1; i > 0; i--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = Tx - 1; i > 0; i--) {
      for (j = Ty - 1 + 2 * (i - Tx) + 1; j > 0; j--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
}

void CheckEnabledRegion_TYPE_VI(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;

   table->cell[0][0].is_region = PATH_OK;

   for (j = 1; j < Ty; j++) {
      for (i = 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
   }

   for (i = 1; i < Tx; i++) {
      table->cell[i][0].is_region = PATH_NG;
   }

   for (j = 1; j < Ty; j++) {
      table->cell[0][j].is_region = PATH_NG;
   }

   for (j = 1; j < Ty - 2; j++) {
      for (i = 2 * j + 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = 1; i < Tx - 2; i++) {
      for (j = 2 * i + 1; j < Ty; j++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }

   /* Backward pruning */
   for (j = Ty - 1; j > 0; j--) {
      for (i = Tx - 1 + 2 * (j - Ty) + 1; i > 0; i--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = Tx - 1; i > 0; i--) {
      for (j = Ty - 1 + 2 * (i - Tx) + 1; j > 0; j--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
}

void CheckEnabledRegion_TYPE_VII(DTW_Table * table)
{
   int i, j, Tx = table->data[0].total, Ty = table->data[1].total;

   table->cell[0][0].is_region = PATH_OK;

   for (j = 1; j < Ty; j++) {
      for (i = 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_OK;
      }
   }

   for (i = 1; i < Tx; i++) {
      table->cell[i][0].is_region = PATH_NG;
   }

   for (j = 1; j < Ty; j++) {
      table->cell[0][j].is_region = PATH_NG;
   }

   for (j = 1; j < Ty - 2; j++) {
      for (i = 2 * j + 1; i < Tx; i++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = 1; i < Tx - 2; i++) {
      for (j = 2 * i + 1; j < Ty; j++) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }

   /* Backward pruning */
   for (j = Ty - 1; j > 0; j--) {
      for (i = Tx - 1 + 2 * (j - Ty) + 1; i > 0; i--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
   for (i = Tx - 1; i > 0; i--) {
      for (j = Ty - 1 + 2 * (i - Tx) + 1; j > 0; j--) {
         table->cell[i][j].is_region = PATH_NG;
      }
   }
}

void CheckEnabledRegion(DTW_Table * table)
{
   switch (table->path) {
   case I:                     /* horizontal and vertical */
      CheckEnabledRegion_TYPE_I(table);
      break;
   case II:                    /* horizontal, oblique and vertical */
      CheckEnabledRegion_TYPE_II(table);
      break;
   case III:                   /* horizontal and oblique */
      CheckEnabledRegion_TYPE_III(table);
      break;
   case IV:                    /* horizontal, oblique1, oblique2 */
      CheckEnabledRegion_TYPE_IV(table);
      break;
   case V:                     /* default */
      CheckEnabledRegion_TYPE_V(table);
      break;
   case VI:
      CheckEnabledRegion_TYPE_VI(table);
      break;
   case VII:
      CheckEnabledRegion_TYPE_VII(table);
      break;
   default:
      break;
   }
}

void CalcLocalCost(DTW_Table * table, enum Norm norm_type)
{
   int i, j, d, D = table->data[0].dim;
   double sum;

   switch (norm_type) {
   case L1:
      for (i = 0; i < table->data[0].total; i++) {
         for (j = 0; j < table->data[1].total; j++) {
            if (table->cell[i][j].is_region == PATH_OK) {
               for (d = 0, sum = 0.0; d < D; d++) {
                  sum += fabs(table->data[0].input[i * D + d] -
                              table->data[1].input[j * D + d]);
               }
               table->cell[i][j].local = sum;
            }
         }
      }
      break;
   case L2:
      for (i = 0; i < table->data[0].total; i++) {
         for (j = 0; j < table->data[1].total; j++) {
            if (table->cell[i][j].is_region == PATH_OK) {
               for (d = 0, sum = 0.0; d < D; d++) {
                  sum += pow((table->data[0].input[i * D + d] -
                              table->data[1].input[j * D + d]), 2);
               }
               table->cell[i][j].local = sqrt(sum);
            }
         }
      }
      break;
   default:
      for (i = 0; i < table->data[0].total; i++) {
         for (j = 0; j < table->data[1].total; j++) {
            if (table->cell[i][j].is_region == PATH_OK) {
               for (d = 0, sum = 0.0; d < D; d++) {
                  sum += pow((table->data[0].input[i * D + d] -
                              table->data[1].input[j * D + d]), 2);
               }
               table->cell[i][j].local = sqrt(sum);
            }
         }
      }
      break;
   }
}

void RecursiveCalc_TYPE_I(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local;

   for (i = 1; i < Tx; i++) {
      table->cell[i][0].global = table->cell[i - 1][0].global
          +table->weight.val[0] * table->cell[i][0].local;
      table->cell[i][0].backptr[0] = i - 1;
      table->cell[i][0].backptr[1] = 0;
   }
   for (j = 1; j < Ty; j++) {
      table->cell[0][j].global = table->cell[0][j - 1].global
          +table->weight.val[1] * table->cell[0][j].local;
      table->cell[0][j].backptr[0] = 0;
      table->cell[0][j].backptr[1] = j - 1;
   }
   for (i = 1; i < Tx; i++) {
      for (j = 1; j < Ty; j++) {
         local = table->cell[i][j].local;
         if (table->cell[i - 1][j].global <table->cell[i][j - 1].global) {
            table->cell[i][j].global = table->cell[i - 1][j].global
                +table->weight.val[0] * local;
            table->cell[i][j].backptr[0] = i - 1;
            table->cell[i][j].backptr[1] = j;
         } else {
            table->cell[i][j].global = table->cell[i][j - 1].global
                +table->weight.val[1] * local;
            table->cell[i][j].backptr[0] = i;
            table->cell[i][j].backptr[1] = j - 1;
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_II(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min;

   for (i = 1; i < Tx; i++) {
      if (table->cell[i][0].is_region == PATH_OK) {
         table->cell[i][0].global = table->cell[i - 1][0].global
             +table->weight.val[0] * table->cell[i][0].local;
         table->cell[i][0].backptr[0] = i - 1;
         table->cell[i][0].backptr[1] = 0;
      }
   }
   for (i = 1; i < Tx; i++) {
      for (j = 1; j < Ty; j++) {
         local = table->cell[i][j].local;
         min = table->cell[i - 1][j].global
         +table->weight.val[0] * local;
         table->cell[i][j].backptr[0] = i - 1;
         table->cell[i][j].backptr[1] = j;
         if (min >= table->cell[i - 1][j - 1].global
             +table->weight.val[1] * local) {
            min = table->cell[i - 1][j - 1].global
            +table->weight.val[1] * local;
            table->cell[i][j].backptr[0] = i - 1;
            table->cell[i][j].backptr[1] = j - 1;
         } else if (min >= table->cell[i][j - 1].global
                    +table->weight.val[2] * local) {
            min = table->cell[i][j - 1].global
            +table->weight.val[2] * local;
            table->cell[i][j].backptr[0] = i;
            table->cell[i][j].backptr[1] = j - 1;
         }
         table->cell[i][j].global = min;
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_III(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   for (i = 1; i < Tx; i++) {
      if (table->cell[i][0].is_region == PATH_OK) {
         table->cell[i][0].global = table->cell[i - 1][0].global
             +table->weight.val[0] * table->cell[i][0].local;
         table->cell[i][0].backptr[0] = i - 1;
         table->cell[i][0].backptr[1] = 0;
      }
   }
   for (j = 1; j < Ty; j++) {
      for (i = 1; i < Tx; i++) {
         local = table->cell[i][j].local;
         if (table->cell[i][j].is_region == PATH_OK) {
            if (table->cell[i - 1][j].is_region == PATH_OK &&
                table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 1][j].global
               +table->weight.val[0] * local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j].global
                   +table->weight.val[0] * local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
            }
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_IV(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   for (i = 1; i < Tx; i++) {   /* horizontal */
      if (table->cell[i][0].is_region == PATH_OK) {
         table->cell[i][0].global = table->cell[i - 1][0].global
             +table->weight.val[0] * table->cell[i][0].local;
         table->cell[i][0].backptr[0] = i - 1;
         table->cell[i][0].backptr[1] = 0;
      }
   }

   table->cell[1][1].global = table->cell[0][0].global  /* oblique1 */
       +table->weight.val[1] * table->cell[1][1].local;
   table->cell[1][1].backptr[0] = 0;
   table->cell[1][1].backptr[1] = 0;

   for (i = 1; i < Tx - Ty / 2; i++) {
      min = table->cell[i - 1][1].global
      +table->weight.val[0] * local;
      table->cell[i][1].backptr[0] = i - 1;
      table->cell[i][1].backptr[1] = 1;
      if (min >= table->cell[i - 1][0].global +table->weight.val[1] * local) {
         min = table->cell[i - 1][0].global
         +table->weight.val[1] * local;
         table->cell[i][1].backptr[0] = i - 1;
         table->cell[i][1].backptr[1] = 0;
      }
      table->cell[i][1].global = min;
   }

   table->cell[1][2].global = table->cell[0][0].global  /* oblique2 */
       +table->weight.val[2] * table->cell[1][2].local;
   table->cell[1][2].backptr[0] = 0;
   table->cell[1][2].backptr[1] = 0;

   for (j = 2; j < Ty; j++) {
      for (i = 2; i < Tx; i++) {
         local = table->cell[i][j].local;
         if (table->cell[i][j].is_region == PATH_OK) {
            if (table->cell[i - 1][j].is_region == PATH_OK &&
                table->cell[i - 1][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j].global
               +table->weight.val[0] * local;   /* horizontal */
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * local;        /* oblique1 */
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               } else if (min >= table->cell[i - 1][j - 2].global
                          +table->weight.val[2] * local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[2] * local;        /* oblique2 */
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 1][j].global
               +table->weight.val[0] * local;   /* horizontal */
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * local;        /* oblique1 */
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 1].global
               +table->weight.val[1] * local;   /* oblique1 */
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 2].global
                   +table->weight.val[2] * local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[2] * local;        /* oblique2 */
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 2].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 2].global
                   +table->weight.val[2] * local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 2;
            }
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_V(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   table->cell[1][1].global = table->cell[0][0].global
       +table->weight.val[2] * table->cell[1][1].local;
   table->cell[1][1].backptr[0] = 0;
   table->cell[1][1].backptr[1] = 0;

   table->cell[1][2].global = table->cell[0][0].global
       +table->weight.val[3] * table->cell[1][1].local
       + table->weight.val[4] * table->cell[1][2].local;
   table->cell[1][2].backptr[0] = 0;
   table->cell[1][2].backptr[1] = 0;

   table->cell[2][1].global = table->cell[0][0].global
       +table->weight.val[0] * table->cell[1][1].local
       + table->weight.val[1] * table->cell[2][1].local;
   table->cell[2][1].backptr[0] = 0;
   table->cell[2][1].backptr[1] = 0;

   for (j = 2; j < Ty; j++) {
      for (i = 2; i < Tx; i++) {
         if (table->cell[i][j].is_region == PATH_OK) {
            if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i - 1][j].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[2] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[2] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               } else if (min >= table->cell[i - 1][j - 2].global
                          +table->weight.val[3] * table->cell[i][j - 1].local
                          + table->weight.val[4] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[3] * table->cell[i][j - 1].local
                      + table->weight.val[4] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i - 1][j].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[2] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[2] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 1].global
               +table->weight.val[2] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 2].global
                   +table->weight.val[3] * table->cell[i][j - 1].local
                   + table->weight.val[4] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[3] * table->cell[i][j - 1].local
                      + table->weight.val[4] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 2][j - 1].global
                   +table->weight.val[0] * table->cell[i][j - 1].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 1].global
                   +table->weight.val[2] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 2].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 2].global
                   +table->weight.val[3] * table->cell[i][j - 1].local
                   + table->weight.val[4] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 2;
            }
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_VI(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   table->cell[2][1].global = table->cell[0][0].global
       +table->weight.val[0] * table->cell[2][1].local;
   table->cell[2][1].backptr[0] = 0;
   table->cell[2][1].backptr[1] = 0;

   table->cell[1][1].global = table->cell[0][0].global
       +table->weight.val[1] * table->cell[1][1].local;
   table->cell[1][1].backptr[0] = 0;
   table->cell[1][1].backptr[1] = 0;

   table->cell[1][2].global = table->cell[0][0].global
       +table->weight.val[2] * table->cell[1][2].local;
   table->cell[1][2].backptr[0] = 0;
   table->cell[1][2].backptr[1] = 0;

   for (j = 2; j < Ty; j++) {
      for (i = 2; i < Tx; i++) {
         if (table->cell[i][j].is_region == PATH_OK) {
            if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               } else if (min >= table->cell[i - 1][j - 2].global
                          +table->weight.val[2] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[2] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 1].global
               +table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 2].global
                   +table->weight.val[2] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[2] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 2][j - 1].global
                   +table->weight.val[0] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 1].global
                   +table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 2].is_region == PATH_OK) {
               table->cell[i][j].global = table->cell[i - 1][j - 2].global
                   +table->weight.val[2] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 2;
            }
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc_TYPE_VII(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   table->cell[1][1].global = table->cell[0][0].global
       +table->weight.val[0] * table->cell[1][1].local;
   table->cell[1][1].backptr[0] = 0;
   table->cell[1][1].backptr[1] = 0;

   table->cell[1][2].global = table->cell[0][0].global
       +table->weight.val[1] * table->cell[1][2].local;
   table->cell[1][2].backptr[0] = 0;
   table->cell[1][2].backptr[1] = 0;

   table->cell[2][1].global = table->cell[0][0].global
       +table->weight.val[2] * table->cell[2][1].local;
   table->cell[2][1].backptr[0] = 0;
   table->cell[2][1].backptr[1] = 0;

   for (j = 2; j < Ty; j++) {
      for (i = 2; i < Tx; i++) {
         if (table->cell[i][j].is_region == PATH_OK) {
            if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                table->cell[i - 2][j - 2].is_region == PATH_OK &&
                table->cell[i - 1][j - 1].is_region == PATH_OK &&
                table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 2][j - 2].global
                   +table->weight.val[2] * table->cell[i - 1][j].local
                   + table->weight.val[3] * table->cell[i][j].local) {
                  min = table->cell[i - 2][j - 2].global
                  +table->weight.val[2] * table->cell[i - 1][j].local
                      + table->weight.val[3] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 2;
                  table->cell[i][j].backptr[1] = j - 2;
               } else if (min >= table->cell[i - 1][j - 1].global
                          +table->weight.val[4] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[1] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               } else if (min >= table->cell[i - 1][j - 2].global
                          +table->weight.val[5] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[5] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
               table->cell[i][j].global = min;
            } else if (table->cell[i - 2][j - 2].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 2].global
               +table->weight.val[2] * table->cell[i - 1][j].local
                   + table->weight.val[3] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 2;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[4] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[4] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               } else if (min >= table->cell[i - 1][j - 2].global
                          +table->weight.val[2] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[5] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                       table->cell[i - 2][j - 2].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i - 1][j].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 2][j - 2].global
                   +table->weight.val[2] * table->cell[i - 1][j].local
                   + table->weight.val[3] * table->cell[i][j].local) {
                  min = table->cell[i - 2][j - 2].global
                  +table->weight.val[2] * table->cell[i - 1][j].local
                      + table->weight.val[3] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 2;
                  table->cell[i][j].backptr[1] = j - 2;
               } else if (min >= table->cell[i - 1][j - 1].global
                          +table->weight.val[4] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[4] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 1].global
               +table->weight.val[4] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 2].global
                   +table->weight.val[5] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[5] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
            } else if (table->cell[i - 2][j - 2].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 2].global
               +table->weight.val[2] * table->cell[i - 1][j].local
                   + table->weight.val[3] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 2;
               if (min >= table->cell[i - 1][j - 2].global
                   +table->weight.val[5] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 2].global
                  +table->weight.val[5] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 2;
               }
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK &&
                       table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i - 1][j].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
               if (min >= table->cell[i - 1][j - 1].global
                   +table->weight.val[5] * table->cell[i][j].local) {
                  min = table->cell[i - 1][j - 1].global
                  +table->weight.val[4] * table->cell[i][j].local;
                  table->cell[i][j].backptr[0] = i - 1;
                  table->cell[i][j].backptr[1] = j - 1;
               }
            } else if (table->cell[i - 2][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 2][j - 1].global
               +table->weight.val[0] * table->cell[i - 1][j].local
                   + table->weight.val[1] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 2;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 1].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 1].global
               +table->weight.val[4] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 1;
            } else if (table->cell[i - 1][j - 2].is_region == PATH_OK) {
               min = table->cell[i - 1][j - 2].global
               +table->weight.val[5] * table->cell[i][j].local;
               table->cell[i][j].backptr[0] = i - 1;
               table->cell[i][j].backptr[1] = j - 2;
            }
         }
      }
   }
   table->cell[Tx - 1][Ty - 1].global /=(Tx + Ty);
}

void RecursiveCalc(DTW_Table * table)
{
   int i, j, k, Tx = table->data[0].total, Ty = table->data[1].total;
   double local, min = 0.0;

   table->cell[0][0].global = table->cell[0][0].local;
   table->cell[0][0].backptr[0] = -1;
   table->cell[0][0].backptr[1] = -1;

   switch (table->path) {
   case I:
      RecursiveCalc_TYPE_I(table);
      break;
   case II:
      RecursiveCalc_TYPE_II(table);
      break;
   case III:
      RecursiveCalc_TYPE_III(table);
      break;
   case IV:
      RecursiveCalc_TYPE_IV(table);
      break;
   case V:
      RecursiveCalc_TYPE_V(table);
      break;
   case VI:
      RecursiveCalc_TYPE_VI(table);
      break;
   case VII:
      RecursiveCalc_TYPE_VII(table);
      break;
   default:
      break;
   }
}

static void BackTrace(DTW_Table * table)
{
   int i, j, k,
       Tx = table->data[0].total, Ty = table->data[1].total, *tmp1, *tmp2;

   tmp1 = (int *) malloc(sizeof(int) * (Tx + Ty));
   tmp2 = (int *) malloc(sizeof(int) * (Tx + Ty));

   i = tmp1[0] = Tx - 1;
   j = tmp2[0] = Ty - 1;
   k = 1;
   while (table->cell[i][j].backptr[0] != -1
          && table->cell[i][j].backptr[1] != -1) {
      i = tmp1[k] = table->cell[tmp1[k - 1]][tmp2[k - 1]].backptr[0];
      j = tmp2[k] = table->cell[tmp1[k - 1]][tmp2[k - 1]].backptr[1];
      k++;
   }

   table->vit_leng = k - 1;
   for (k = 0; k < table->vit_leng; k++) {
      table->data[0].viterbi[k] = tmp1[table->vit_leng - k];
      table->data[1].viterbi[k] = tmp2[table->vit_leng - k];
   }
   free(tmp1);
   free(tmp2);
}

/* Concatenate two input along to Viterbi path */
static double *Concat(DTW_Table * table)
{
   double *concat;
   int i, j, size = table->vit_leng,
       dim = table->data[0].dim + table->data[1].dim;

   concat = dgetmem(size * dim);

   for (i = 0; i < size; i++) {
      for (j = 0; j < table->data[0].dim; j++) {
         concat[dim * i + j]
             = table->data[0].input[table->data[0].viterbi[i]
                                    * table->data[0].dim + j];
      }
      for (j = 0; j < table->data[1].dim; j++) {
         concat[dim * i + table->data[0].dim + j]
             = table->data[1].input[table->data[1].viterbi[i]
                                    * table->data[1].dim + j];
      }
   }

   return (concat);
}

double *ReadInput(FILE * fp, int dim, int *length)
{
   int i, j;
   double *x = NULL, *input;
   float_list *top, *prev, *cur, *next, *tmp;

   input = dgetmem(dim);
   top = prev = (float_list *) malloc(sizeof(float_list));
   *length = 0;
   prev->next = NULL;

   while (freadf(input, sizeof(*input), dim, fp) == dim) {
      cur = (float_list *) malloc(sizeof(float_list));
      cur->f = (float *) malloc(sizeof(float) * dim);
      for (i = 0; i < dim; i++) {
         cur->f[i] = (float) input[i];
      }
      (*length)++;
      prev->next = cur;
      cur->next = NULL;
      prev = cur;
   }

   x = dgetmem(*length * dim);
   for (i = 0, tmp = top->next; tmp != NULL; i++, tmp = tmp->next) {
      for (j = 0; j < dim; j++) {
         x[i * dim + j] = tmp->f[j];
      }
   }

   for (tmp = top->next; tmp != NULL; tmp = next) {
      cur = tmp;
      next = tmp->next;
      free(cur->f);
      free(cur);
      cur->f = NULL;
      cur = NULL;
   }
   free(top);
   free(input);
   top = NULL;
   input = NULL;

   return (x);
}

void usage(int status)
{
   fprintf(stderr, "\n");
   fprintf(stderr, " %s - Dynamic Time Warping\n", cmnd);
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       %s [ options ] tempfile [ infile ] > stdout\n",
           cmnd);
   fprintf(stderr, "  options:\n");
   fprintf(stderr,
           "       -m M      : order of vector                      [%d]\n", 0);
   fprintf(stderr,
           "       -l L      : length of vector                     [m+1]\n");
   fprintf(stderr,
           "       -t T      : number of test vectors               [N/A]\n");
   fprintf(stderr,
           "       -r R      : number of reference vectors          [N/A]\n");
   fprintf(stderr,
           "       -n N      : type of norm use for calculation     [%d]\n",
           L2);
   fprintf(stderr, "                   of local distance\n");
   fprintf(stderr, "                      N = 1 : L1-norm\n");
   fprintf(stderr, "                      N = 2 : L2-norm\n");
   fprintf(stderr,
           "       -p P      : local path constraint                [%d]\n", V);
   fprintf(stderr,
           "       -s sfile  : output score of dynamic time warping [FALSE]\n");
   fprintf(stderr, "                   to sfile \n");
   fprintf(stderr,
           "       -v vfile  : output frame number sequence         [FALSE]\n");
   fprintf(stderr, "                   along the Viterbi path\n");
   fprintf(stderr, "       -h        : print this message\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr,
           "       reference data sequence (%s)                  [stdin]\n",
           FORMAT);
   fprintf(stderr, "  stdout:\n");
   fprintf(stderr, "       concatenated test/reference data\n");
   fprintf(stderr, "       along the Viterbi path (%s)\n", FORMAT);
#ifdef PACKAGE_VERSION
   fprintf(stderr, "\n");
   fprintf(stderr, " SPTK: version %s\n", PACKAGE_VERSION);
   fprintf(stderr, " CVS Info: %s", rcs_id);
#endif
   fprintf(stderr, "\n");
   exit(status);
}

int main(int argc, char *argv[])
{
   char *infile = NULL, *infile2 = NULL, *Scorefile = NULL, *Viterbifile = NULL;
   int i, j, d, leng = LENG, total1, total2, tmp, length1 = 0, length2 = 0;
   double *x = NULL, *y = NULL, *z = NULL, *input = NULL;
   enum Norm norm_type = L2;
   enum PATH path = V;
   FILE *fp = stdin, *fp2 = NULL, *fpScore = NULL, *fpViterbi = NULL;
   Boolean outscore = FA, outViterbi = FA;
   DTW_Table table;

   if ((cmnd = strrchr(argv[0], '/')) == NULL) {
      cmnd = argv[0];
   } else {
      cmnd++;
   }

   /* Analyze options */
   while (--argc) {
      if (**++argv == '-') {
         switch (*(*argv + 1)) {
         case 'm':
            leng = atoi(*++argv) + 1;
            --argc;
            break;
         case 'l':
            leng = atoi(*++argv);
            --argc;
            break;
         case 't':
            length1 = atoi(*++argv);
            --argc;
            break;
         case 'r':
            length2 = atoi(*++argv);
            --argc;
            break;
         case 'n':
            norm_type = (enum PATH) atoi(*++argv);
            --argc;
            break;
         case 'p':
            path = (enum PATH) atoi(*++argv);
            --argc;
            break;
         case 's':
            Scorefile = *++argv;
            fpScore = getfp(Scorefile, "wb");
            outscore = TR;
            --argc;
            break;
         case 'v':
            Viterbifile = *++argv;
            fpViterbi = getfp(Viterbifile, "wb");
            outViterbi = TR;
            --argc;
            break;
         case 'h':
            usage(0);
         default:
            fprintf(stderr, "%s : Invalid option '%c'!\n", cmnd, *(*argv + 1));
            usage(1);
         }
      } else if (infile2 == NULL) {
         infile2 = *argv;
         fp2 = getfp(infile2, "rb");
      } else {
         fp = getfp(*argv, "rb");
      }
   }

   if (infile2 == NULL) {
      fprintf(stderr, "%s : The comparative file name must be specified !\n",
              cmnd);
      usage(1);
   }
   if (outscore == TR && fpScore == NULL) {
      fprintf(stderr, "%s : output file name must be specified !\n", cmnd);
      usage(1);
   }
   if (outViterbi == TR && fpViterbi == NULL) {
      fprintf(stderr, "%s : output file name must be specified !\n", cmnd);
      usage(1);
   }

   x = ReadInput(fp, leng, &total1);
   y = ReadInput(fp2, leng, &total2);

   if (length1 != 0) {          /* if -r option is specified */
      total1 = length1;
   }
   if (length2 != 0) {          /* if -t option is specified */
      total2 = length2;
   }

   if (total1 / 2 >= total2) {
      fprintf(stderr, "Can't perform DTW !\n"
              "The number of the reference vectors (= %d) must be less than "
              "the twice of the template (= 2 * %d = %d). \n",
              total1, total2, 2 * total2);
      usage(1);
   } else if (total2 / 2 >= total1) {
      fprintf(stderr, "Can't perform DTW !\n"
              "The number of the template vectors (= %d) must be less than "
              "the twice of the reference (= 2 * %d = %d). \n",
              total2, total1, 2 * total1);
      usage(1);
   }

   /* Initialize */
   InitDTW(&table, leng, x, y, total1, total2, path);

   /* Dynamic Time Warping */
   /* Output vectors are concatenated two input vectors along the Viterbi path */
   z = DTW(&table, norm_type);
   fwritef(z, sizeof(*z), table.vit_leng * 2 * leng, stdout);

   if (outscore == TR) {
      total1 = table.data[0].total - 1;
      total2 = table.data[1].total - 1;
      fwritef(&table.cell[total1][total2].global, sizeof(double), 1, fpScore);
   }
   if (outViterbi == TR) {
      for (i = 0; i < table.vit_leng; i++) {
         fwrite(table.data[0].viterbi + i, sizeof(int), 1, fpViterbi);
         fwrite(table.data[1].viterbi + i, sizeof(int), 1, fpViterbi);
      }
   }

   return (0);
}
