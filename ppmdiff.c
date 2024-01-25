#include <stdlib.h>
#include "pnm.h"
#include <a2plain.h>
#include <stdio.h>
#include <string.h>
#include "assert.h"
#include <math.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))


double pnmdiff(Pnm_ppm p1, Pnm_ppm p2) 
{
    int minwidth = min(p1->width, p2->width);
    int minheight = min(p1->height, p2->height);

    if ((pow((int)p1->height - (int)p2->height, 2) > 1) ||
       (pow((int)p1->width - (int)p2->width, 2) > 1)) {
        fprintf(stdout, "1.0");
        fprintf(stderr, "dimension difference is larger than 1.\n");
        exit(1);
    }
    assert(pow((int)p1->height - (int)p2->height, 2) <= 1);
    
    double total_diff = 0;
    
    for (int i = 0; i < minheight; i++) {
        for (int j = 0; j < minwidth; j++) {
            Pnm_rgb p1pix = p1->methods->at(p1->pixels, j, i);
            Pnm_rgb p2pix = p2->methods->at(p2->pixels, j, i);

            double red_diff = pow(((double) p1pix->red / p1-> denominator - 
                                  (double) p2pix->red / p2-> denominator), 2);
            double green_diff = pow(((double) p1pix->green / p1-> denominator - 
                                    (double) p2pix->green / p2-> denominator), 2);
            double blue_diff = pow(((double)p1pix->blue / p1->denominator - 
                                   (double) p2pix->blue / p2-> denominator), 2);

            double diffsums = (red_diff + green_diff + blue_diff);
            total_diff += diffsums;
        }
    }
    return sqrt(total_diff / (3 * minwidth * minheight));
}

int main (int argc, char* argv[])
{

    /* default to UArray2 methods */
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);

    FILE *file1 = NULL;
    FILE *file2 = NULL;
    /* can take up to 2 arguments */
    if (argc != 3) {
        fprintf(stderr, "Incorrect number of arguments\n");
    /* 2 arguments are received */
    } else if (argc == 3) {

        /* check if we need to read from std in*/
        if (strcmp(argv[1], "-") == 0 && strcmp(argv[2], "-") == 0) {
            fprintf(stderr, "cannot take two stdins\n");
        }

        else if (strcmp(argv[1], "-") == 0) {
            file1 = stdin;
            file2 = fopen(argv[2], "r");
        }
        
        else if (strcmp(argv[2], "-") == 0) {
            file1 = fopen(argv[1], "r");
            file2 = stdin;
        }

        else {
            file1 = fopen(argv[1], "r");
            file2 = fopen(argv[2], "r");
        }

        assert(file1 != NULL && file2 != NULL);
        Pnm_ppm p1 = Pnm_ppmread(file1, methods);
        Pnm_ppm p2 = Pnm_ppmread(file2, methods);

        double diff = pnmdiff(p1, p2);
        
        /* round to the nearest 4th digit after decimal point*/
        printf("diff is: %0.4f\n", diff);

        Pnm_ppmfree(&p1);
        Pnm_ppmfree(&p2);

        if (file1 != NULL) {
            fclose(file1);
        }

        if (file2 != NULL) {
            fclose(file2);
        }
    }
}