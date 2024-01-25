#ifndef DCT_TRANSFORMATION_H
#define DCT_TRANSFORMATION_H
#include <stdlib.h>
#include "seq.h"
#include "assert.h"
#include "stdio.h"

#define Hanson_Seq Seq_T

/* dct struct */
typedef struct Dct {
    unsigned a;
    signed b;
    signed c;
    signed d;
} *Dct;

/* transform y values to dct values */
Dct y_to_dct(float y1, float y2, float y3, float y4);
/* transform y values to dct values */
Hanson_Seq dct_to_y(Dct dct);

Dct dummy_dct();

#endif