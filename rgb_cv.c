#include "rgb_cv.h"

/* rgb_to_cv
 * purpose: transform the passed-in rgb values to percent form with
 * regards to the denominator, and convert rgb values to component video values
 * input: Pnm_rgb pixel_rgb, unsigned denominator
 * output: Struct composite that holds the three component video 
 * values y, pb, and pr
 */
composite rgb_to_cv(Pnm_rgb pixel_rgb, unsigned denominator)
{
    composite composite_new = malloc(sizeof(*composite_new));
    double red = pixel_rgb->red / (double)denominator;
    double green = pixel_rgb->green / (double)denominator;
    double blue = pixel_rgb->blue / (double)denominator;
    composite_new->y = 0.299 * red + 0.587 * green + 0.114 * blue;
    composite_new->pb = -0.168736 * red - 0.331264 * green + 0.5 * blue;
    composite_new->pr = 0.5 * red - 0.418688 * green - 0.081312 * blue;
    
    return composite_new;
}

/* cv_to_rgb
 * purpose: convert passed-in component video values to rgb values
 * input: double y, double pb, double pr, unsigned denominator
 * output: Struct Pnm_rgb that holds the three rgb values red, green, and blue
 */
Pnm_rgb cv_to_rgb(double y, double pb, double pr, unsigned denominator)
{
    Pnm_rgb rgb = malloc(sizeof(*rgb));
    double red = (double)denominator * (1.0 * y + 0.0 * pb + 1.402 * pr);
    double green = (double)denominator * (1.0 * y - 0.344136 * pb - 0.714136 * pr);
    double blue = (double)denominator * (1.0 * y + 1.772 * pb + 0.0 * pr);
    if (red < 0) {
        red = 0;
    }
    if (green < 0) {
        green = 0;
    }
    if (blue < 0) {
        blue = 0;
    }

    if (red > (double)denominator) {
        red = 0;
    }
    if (green > (double)denominator) {
        green = 0;
    }
    if (blue > (double)denominator) {
        blue = 0;
    }
    rgb->red = red;
    rgb->green = green;
    rgb->blue = blue;
    return rgb;
}

/* dummy_composite
 * purpose: initialize a composite struct 
 * input: None
 * output: Struct composite that holds placeholder values
 */
composite dummy_composite()
{
    composite composite_new = malloc(sizeof(*composite_new));
    composite_new->y = 0.0;
    composite_new->pb = 0.0;
    composite_new->pr = 0.0;
    
    return composite_new;
}