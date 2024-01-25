#include "bitpack.h"
#include <stdlib.h>
#include "assert.h"

Except_T Bitpack_Overflow = {"Overflow packing bits"};

/* HELPER FUNCTIONS */

/* REMINDER - Shifts on unsigned values will always
    populate with 0 values. Left shifts on signed values
    will populate with zeros, but right shifts on signed
    values populate witht the right-most value (i.e., negative
    values populate with 1s, and positives with 0s)
*/

/* leftu
 * purpose: shifts an unsigned value left, width bits 
 * input: uint64_t n, unsigned width
 * output: value n that is shfited left, width bits
 */
uint64_t leftu(uint64_t n, unsigned width)
{
    if (width >= 64) {
        return 0;
    }
    uint64_t output = n << width;
    return output;
}

/* rightu
 * purpose: shifts an unsigned value right, width bits 
 * input: uint64_t n, unsigned width
 * output: value n that is shifted right, width bits
 */
uint64_t rightu(uint64_t n, unsigned width)
{
    if (width >= 64) {
        return 0;
    }
    uint64_t output = n >> width;
    return output;
}

/* lefts
 * purpose: shifts a signed value left, width bits 
 * input: int64_t n, unsigned width
 * output: value n that is shfited left, width bits
 */
int64_t lefts(int64_t n, unsigned width)
{
    if (width >= 64) {
        return 0;
    }
    int64_t output = n << width;
    return output;
}

/* rights
 * purpose: shifts a signed value right, width bits 
 * input: int64_t n, unsigned width
 * output: value n that is shifted right, width bits
 */
int64_t rights(int64_t n, unsigned width)
{
    if (width >= 64) {
        return 0;
    }
    int64_t output = n >> width;
    return output;
}

/* MODULE FUNCTIONS */

/* Bitpack_fitsu
 * purpose: checks if an unsigned field (n) fits inside of (width) bits
 * input: uint64_t n, unsigned width
 * output: true if n fits inside width bits; false otherwise 
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    if (width >= 64) {
        return true;
    } else if (width == 0) {
        return false;
    }
    uint64_t max = leftu(1, width);
    return (bool)(n < max);
}

/* Bitpack_fitss
 * purpose: checks if a signed field (n) fits inside of (width) bits
 * input: int64_t n, unsigned width
 * output: true if n fits inside width bits; false otherwise 
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    if (width >= 64) {
        return true;
    } else if (width == 0) {
        return false;
    }
    if (n >= 0) {
        return Bitpack_fitsu(2 * n, width);
    } else {
        return Bitpack_fitsu((~(n-1) * 2 - 1), width);
    }
}

/* Bitpack_getu
 * purpose: returns an unsigned field from a word
 * input: uint64_t word, unsigned width, unsigned lsb
 * output: an unsigned field from a word with width bits 
 * starting at postion lsb
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert((width + lsb) <= 64);
    if (width == 0) {
        return 0;
    }
    uint64_t mask = leftu(1, width) - 1;
    mask = leftu(mask, lsb);
    uint64_t output = (word & mask);
    output = rightu(output, lsb);
    return output;
}

/* Bitpack_gets
 * purpose: returns an signed field from a word
 * input: uint64_t word, unsigned width, unsigned lsb
 * output: a signed field from a word with width bits 
 * starting at postion lsb
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert((width + lsb) <= 64);
    if (width == 0) {
        return 0;
    }
    uint64_t output = Bitpack_getu(word, width, lsb);
    uint64_t max = leftu(1, width - 1) -1;
    if (output <= max) {
        return (int64_t)(output);
    } else {
        return (int64_t)(output - leftu(1, width));
    }
}

/* Bitpack_newu
 * purpose: adjusts one unsigned field in a word and outputs the new word
 * input: uint64_t word, unsigned width, unsigned lsb, uint64_t value
 * output: a word that is the same as the passed-in word except for the field
 * at [lsb,lsb+width] where the field is replaced by value
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value)
{
    assert((width + lsb) <= 64);
    if (!(Bitpack_fitsu(value, width))) {
        RAISE(Bitpack_Overflow);
    }
    uint64_t left_mask = leftu(leftu(1, 64 - width - lsb) - 1, width + lsb);
    uint64_t right_mask = leftu(1, lsb) - 1;
    uint64_t mask = left_mask + right_mask;
    return (word & mask) | leftu(value, lsb);
}

/* Bitpack_news
 * purpose: adjusts one signed field in a word and outputs the new word
 * input: uint64_t word, unsigned width, unsigned lsb, uint64_t value
 * output: a word that is the same as the passed-in word except for the field
 * at [lsb,lsb+width] where the field is replaced by value
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, int64_t value)
{
    assert((width + lsb) <= 64);
    if (!(Bitpack_fitss(value, width))) {
        RAISE(Bitpack_Overflow);
    }

    uint64_t left_mask = leftu(leftu(1, 64 - width - lsb) - 1, width + lsb);
    uint64_t right_mask = leftu(1, lsb) - 1;
    uint64_t mask = left_mask + right_mask;
    uint64_t val = value;
    val = rightu(leftu(val, 64 - width), 64 - width - lsb);

    return (word & mask) | val;
}