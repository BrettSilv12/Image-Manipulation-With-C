#include "average_chroma.h"
#include "arith40.h"

/* get_average_chroma_index
 * purpose: return the index of the average value of the four given 
 * chroma values
 * input: double p1, double p2, double p3, double p4. Four chroma values of 
 * type double.
 * output: the index of the average value of the four given chroma values
 */
unsigned get_average_chroma_index(double p1, double p2, double p3, double p4)
{
    float average =  (p1 + p2 + p3 + p4) / 4.0;
    return Arith40_index_of_chroma(average); 
}

/* get_average_chroma_value
 * purpose: return the quantized value corresponding to the given index
 * input: unsigned p1
 * output: the quantized value corresponding to the given index
 */
double get_average_chroma_value(unsigned p1)
{
    return Arith40_chroma_of_index(p1);
}