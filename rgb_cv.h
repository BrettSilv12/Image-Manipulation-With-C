#ifndef RGB_CV_H
#define RGB_CV_H
#include <stdlib.h>
#include <stdio.h>
#include "pnm.h"

/* composite struct */
typedef struct composite {
    float y;
    float pb;
    float pr;
} *composite;

/* convert rgb values to component video values */
composite rgb_to_cv(Pnm_rgb pixel_rgb, unsigned denominator);
/* convert component video values to rgb values */
Pnm_rgb cv_to_rgb(double y, double pb, double pr, unsigned denominator);

composite dummy_composite();

#endif