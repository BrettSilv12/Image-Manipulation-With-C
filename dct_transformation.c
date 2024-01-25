#include "dct_transformation.h"


signed signed_conversion(float val);

/* y_to_dct
 * purpose: perform dct transformations on passed-in y values, and 
 * return the dct values as a struct Dct.
 * input: float y1, float y2, float y3, float y4
 * output: struct Dct that holds transformed dct values
 */
Dct y_to_dct(float y1, float y2, float y3, float y4)
{
    float a = (y4 + y3 + y2 + y1) / 4.0;
    float b = (y4 + y3 - y2 - y1) / 4.0;
    float c = (y4 - y3 + y2 - y1) / 4.0;
    float d = (y4 - y3 - y2 + y1) / 4.0;

    /* convert a to unsigned values */
    /* a needs to b*/
    /* since a is stored as a 9-bit unsigned integer, there are 511 intervals */
    unsigned au = (int)(a * 511);
    if (au == 511) {
        au = 510;
    }

    /* convert b,c,d to signed values */
    signed bs = signed_conversion(b);
    signed cs = signed_conversion(c);
    signed ds = signed_conversion(d);

    Dct dct = malloc(sizeof(struct Dct));
    dct->a = au;
    dct->b = bs;
    dct->c = cs;
    dct->d = ds;

    return dct;
}

/* dct_to_y
 * purpose: perform inverse dct transformations on passed-in struct dct, and
 * return a Hanson sequence that holds the four y values
 * input: Dct dct
 * output: Hanson sequence that holds the four y values
 */
Hanson_Seq dct_to_y(Dct dct)
{
    Hanson_Seq yvals = Seq_new(4*sizeof(float*));

    unsigned as = dct->a;
    signed bu = dct->b;
    signed cu = dct->c;
    signed du = dct->d;

    float a, b, c, d;

    /* find the corresponding value of a,b,c,d */
    a = as / 511.0;

    b = (int)bu / 31.0 * 0.6;
    c = (int)cu / 31.0 * 0.6;
    d = (int)du / 31.0 * 0.6;

    float *y1 = ((float*)malloc(sizeof(float)));
    *y1 = a - b - c + d;
    float *y2 = ((float*)malloc(sizeof(float)));
    *y2 = a - b + c - d;
    float *y3 = ((float*)malloc(sizeof(float)));
    *y3 = a + b - c - d;
    float *y4 = ((float*)malloc(sizeof(float)));
    *y4 = a + b + c + d;

    Seq_addhi(yvals, y1);
    Seq_addhi(yvals, y2);
    Seq_addhi(yvals, y3);
    Seq_addhi(yvals, y4);

    return yvals;
}

/* signed_conversion
 * purpose: return signed number versions of b, c, d values
 * input: float val
 * output: signed value that corresponds to the input
 */
signed signed_conversion(float val)
{
    if (val > 0.3) {
        val = 0.3;
    } else if (val < -0.3) {
        val = -0.3;
    }

    signed val_s = (int)((val / 0.6) * 31.0);

    if (val_s == 16) {
        val_s = 15;
    }

    return val_s;
}

/* dummy_dct
 * purpose: initialize a dct struct 
 * input: None
 * output: Struct Dct that holds placeholder values
 */
Dct dummy_dct()
{
    Dct dct_new = malloc(sizeof(*dct_new));
    dct_new->a = 0;
    dct_new->b = 0;
    dct_new->c = 0;
    dct_new->d = 0;
    
    return dct_new;
}