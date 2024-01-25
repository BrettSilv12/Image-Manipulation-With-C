#ifndef AVERAGE_CHROMA_H
#define AVERAGE_CHROMA_H
#include <stdlib.h>
#include <stdio.h>
#define Hanson_Seq Seq_T

/* get the index of average chroma value */
unsigned get_average_chroma_index(double p1, double p2, double p3, double p4);

/* get quantized average chroma value from a given index*/
double get_average_chroma_value(unsigned p1);

#endif