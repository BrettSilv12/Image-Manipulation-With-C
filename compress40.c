#include <stdint.h>
#include <stdlib.h>

#include "uarray2.h"
#include <a2plain.h>

#include "pnm.h"
#include "assert.h"
#include "seq.h"

#include "uarray2.h"
#include "rgb_cv.h"
#include "average_chroma.h"
#include "dct_transformation.h"
#include "bitpack.h"
#define Hanson_Seq Seq_T
/****************/
/* PIXEL STRUCT */
/****************/
/*
    This struct can hold all values
    necessary to each pixel in a raster.
    The compress40.c code then can grab 
    only the necessary and important info 
    from these structs to each module 
*/
typedef struct pixel {
    /* Our Pnm_rgb structs that hold Red, Green, and Blue values */
    /* (defined in Pnm.h) */
    Pnm_rgb pnm_rgb;
    /* Our composite structs that hold y, pb, and pr values */
    /* (defined in rgb_cv.h) */
    composite composite;
    unsigned avg_pb;
    unsigned avg_pr;
    /* Our Dct structs, that hold our a,b,c,d values */
    /* (defined in dct_transformation.h) */
    Dct dct;
    uint64_t word;
} *pixel;


/* Helper functions */
/* File I/O */
Hanson_Seq init_block(Pnm_ppm data, unsigned width, unsigned height);
struct Pnm_ppm read_header_decompress(FILE *input);
void read_input_decompress(FILE *input, struct Pnm_ppm pixmap);
void print_output_compress(UArray2_T whole_graph, struct Pnm_ppm pixmap);
void print_output_decompress(struct Pnm_ppm pixmap);

/* Compression */
void rgb_to_cv_help(Hanson_Seq block, unsigned denominator);
void compress_chroma_help(Hanson_Seq block);
void compress_dct_help(Hanson_Seq block);
void compress_bitpack_help(Hanson_Seq block);

/* Decompression */
void cv_to_rgb_help(struct Pnm_ppm pixmap);
void decompress_chroma_help(struct Pnm_ppm pixmap);
void decompress_dct_help(struct Pnm_ppm pixmap);
void decompress_bitpack_help(struct Pnm_ppm pixmap);

/* compress40
 * purpose: read in a file and write its compressed form to standard output
 * input: FILE *input
 * output: none
 */  
void compress40(FILE *input) 
{
    assert(input != NULL);
    /* read in file */
    Pnm_ppm data = Pnm_ppmread(input, uarray2_methods_plain);
    assert(data != NULL);
    assert(data->height > 0 && data->width > 0);
    /* trim data dimensions */
    data->height -= data->height % 2;
    data->width -= data->width % 2;
    /* Initialize 2D array that holds pixel structs for output */
    UArray2_T whole_graph = data->methods->new((int)data->width,
                         (int)data->height, sizeof(struct pixel));
    assert(whole_graph != NULL);
    /* Traverse the graph in 2*2 block order */
    for (unsigned height = 0; height < data->height; height += 2) {
        for (unsigned width = 0; width < data->width; width += 2) {
            /* Initialize array that stores the 4 pixel structs in a block */
            Hanson_Seq block = init_block(data, width, height);
            rgb_to_cv_help(block, data->denominator);
            compress_chroma_help(block);
            compress_dct_help(block);
            compress_bitpack_help(block);
            /*
                Add our block of pixels to new 2d array in block-order
                so that we can reuse the "block" sequence, but
                not lose data
             */
            for (int i = 0; i < Seq_length(block); i++) {
                pixel p = ((pixel)Seq_get(block, i));
                pixel slot = ((pixel)UArray2_at(whole_graph, width + i % 2, 
                                    height + i / 2));
                *slot = *p;
            }
            /* Free up the 2x2 block to be reused */
            for(int i = 0; i < Seq_length(block); i++) {
                free(Seq_get(block, i));
            }
            Seq_free(&block);
        }
    }
    print_output_compress(whole_graph, *data);
    /* Free all data */
    for (int width = 0; width < (int)data->width; width++) {
        for (int height = 0; height < (int)data->height; height++) {
            free(((pixel)data->methods->at(whole_graph, width, height))->
                                                                composite);
            free(((pixel)data->methods->at(whole_graph, width, height))->dct);
        }
    }
    Pnm_ppmfree(&data);
    UArray2_free(&whole_graph);
    return;
}

/* decompress40
 * purpose: read in a compressed file and write its decompressed 
 * form to standard output
 * input: FILE *input
 * output: none
 */  
void decompress40(FILE *input)
{
    assert(input != NULL);
    struct Pnm_ppm pixmap = read_header_decompress(input);
    read_input_decompress(input, pixmap);
    decompress_bitpack_help(pixmap);
    decompress_chroma_help(pixmap);
    decompress_dct_help(pixmap);
    cv_to_rgb_help(pixmap);
    print_output_decompress(pixmap);
    for (int width = 0; width < (int)pixmap.width; width++) {
        for (int height = 0; height < (int)pixmap.height; height++) {
            free(((pixel)UArray2_at(pixmap.pixels, width, height))->pnm_rgb);
            free(((pixel)UArray2_at(pixmap.pixels, width, height))->composite);
            free(((pixel)UArray2_at(pixmap.pixels, width, height))->dct);
        }
    }
    pixmap.methods->free(&pixmap.pixels);

    return;
}


/*****************************************************************************/
/*                             Helper functions                              */
/*****************************************************************************/



/*****************************/
/*                           */
/*         File I/O          */ 
/*                           */
/*****************************/

/* read_input_decompress
 * purpose: 1. reads the compressed word from file one by one
 *          2. populate the corresponding elements in pixmap with 
 *             the read-in word
 *          3. initialize dct and composite fields for each element in pixmap
 * input: FILE *input, struct Pnm_ppm pixmap
 * output: none
 */  
void read_input_decompress(FILE *input, struct Pnm_ppm pixmap)
{
    for (int height = 0; height < (int)(pixmap.height); height+=2) {
        for (int width = 0; width < (int)(pixmap.width); width+=2) {
            //uint64_t word;
            //int read = fscanf(input, "%lu ", &word);
            assert(read == 1);
            for(int row = 0; row < 2; row++) {
                for(int col = 0; col < 2; col++) {
                    Dct *currdct = &((pixel)pixmap.methods->
                            at(pixmap.pixels, width + col, height + row))->
                                                                dct;
                    *currdct = dummy_dct();
                    composite *currcomp = &((pixel)pixmap.methods->
                            at(pixmap.pixels, width + col, height + row))->
                                                                composite;
                    *currcomp = dummy_composite();
                    uint64_t *word_slot = &(((pixel)pixmap.methods->
                            at(pixmap.pixels, width + col, height + row))->
                                                                word);
                    

                    uint64_t word = 0;
                    uint64_t one_byte;

                    /*Bytes are written to the disk in big-endian order.*/
                    for (int w = 24; w >= 0; w = w - 8){
                        //NEED TO ASK IF BIN IMAGES WILL HAVE WORDS SEPARATED BY SPACES OR NOT
                            one_byte = getc(fp);
                            one_byte = one_byte << w;
                            word = word + one_byte;
                    }
                    *word_slot = word;
                }
            }
        }
    }
}

/* read_header_decompress
 * purpose: 1. reads header of the passed-in compressed file
 *          2. initialize a Pnm_ppm struct pixmap. set its width 
 *             and height fields to the value in he header
 *          3. set the elements of the pixmap->pixels to be of pixel type
 * input: FILE *input
 * output: struct Pnm_ppm
 */  
struct Pnm_ppm read_header_decompress(FILE *input)
{
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", &width,
                                                                    &height);
    assert(read == 2);
    assert(width > 0 && height > 0);
    int c = getc(input);
    assert(c == '\n');
    struct Pnm_ppm pixmap = { .width = width, .height = height
    , .denominator = 256, .pixels = UArray2_new(width, height, 
                                        sizeof(struct pixel))
    , .methods = uarray2_methods_plain
    };
    assert(pixmap.pixels != NULL);
    return pixmap;
}

/* init_block
 * purpose: initialize a Hanson sequence of size 4, which holds the 4 
 *          elements for each block 
 * input: Pnm_ppm data, unsigned width, unsigned height
 * output: a Hanson sequence that holds the 4 elements for each block 
 */  
Hanson_Seq init_block(Pnm_ppm data, unsigned width, unsigned height)
{
    Hanson_Seq block = Seq_new(4 * sizeof(pixel));
    assert(block != NULL);

    for (unsigned h = 0; h < 2; h++) {
        for (unsigned w = 0; w < 2; w++) {
            pixel p = malloc(sizeof(*p));
            Pnm_rgb rgb = UArray2_at(data->pixels, (int)width + w,
                                                    (int)height + h);
            p->pnm_rgb = rgb;
            Seq_addhi(block,p);
        }
    }

    return block;
}

/* print_output_compress
 * purpose: write the compressed word to standard out. 
 *          helper functoin for compress40.
 * input: UArray2_T whole_graph, struct Pnm_ppm pixmap
 * output: none
 */  
void print_output_compress(UArray2_T whole_graph, struct Pnm_ppm pixmap)
{
    fprintf(stdout, "COMP40 Compressed image format 2\n%u %u\n", 
            pixmap.methods->width(whole_graph), 
            pixmap.methods->height(whole_graph));
    for(int i = 0; i < pixmap.methods->height(whole_graph); i+=2) {
        for(int j = 0; j < pixmap.methods->width(whole_graph); j+=2) {
            uint32_t outputword = (uint32_t)(((pixel)(pixmap.methods->
                                        at(whole_graph, j, i)))->word);
            /* 
                Here we reverse the Endian-order of the word, by starting
                at the end and reading out one byte at a time 
            */
            for(w = 24; w >= 0; w = w - 8){
                unsigned char out_byte = (outputword >> w);
                putchar(out_byte);
            }
            /*
            fprintf(stdout, "%lu ", ((pixel)(pixmap.methods->
                                        at(whole_graph, j, i)))->word);
            fprintf(stdout, "    ");
            */
        }
    }
}

/* print_output_decompress
 * purpose: write the decompressed rgb values to standard out.
 *          helper function for decompress40.
 * input: struct Pnm_ppm pixmap
 * output: none
 */  
void print_output_decompress(struct Pnm_ppm pixmap)
{
    fprintf(stdout, "P3\n%u %u\n%u\n", pixmap.width, pixmap.height,
                                                pixmap.denominator);
    for(int i = 0; i < (int)(pixmap.width * pixmap.height); i++) {
        int width = i % (int)pixmap.width;
        int height = i / (int)pixmap.width;
        Pnm_rgb cell_rgb = ((pixel)UArray2_at(pixmap.pixels, width, height))->
                                                                    pnm_rgb;                                                      
        fprintf(stdout, "%d %d %d  ", cell_rgb->red, cell_rgb->green,
                                                            cell_rgb->blue);
    }
}




/*****************************/
/*                           */
/*        Compression        */ 
/*                           */
/*****************************/

/* rgb_to_cv_help
 * purpose: call rgb_to_cv module to convert rgb values to cv values
 * input: Hanson_Seq block, unsigned denominator
 * output: none
 */ 
void rgb_to_cv_help(Hanson_Seq block, unsigned denominator) 
{
    for(int i = 0; i < Seq_length(block); i++) {
        composite *currpix = &((pixel)Seq_get(block, i))->composite;
        *currpix = (rgb_to_cv(((pixel)Seq_get(block, i))->pnm_rgb, 
                    denominator));
    }
}

/* compress_chroma_help
 * purpose: call average_chroma module to calculage average chroma values
 * input: Hanson_Seq block
 * output: none
 */ 
void compress_chroma_help(Hanson_Seq block)
{
    double pb1 = ((pixel)Seq_get(block, 0))->composite->pb;
    double pb2 = ((pixel)Seq_get(block, 1))->composite->pb;
    double pb3 = ((pixel)Seq_get(block, 2))->composite->pb;
    double pb4 = ((pixel)Seq_get(block, 3))->composite->pb;
    double pr1 = ((pixel)Seq_get(block, 0))->composite->pr;
    double pr2 = ((pixel)Seq_get(block, 1))->composite->pr;
    double pr3 = ((pixel)Seq_get(block, 2))->composite->pr;
    double pr4 = ((pixel)Seq_get(block, 3))->composite->pr;
    unsigned avgpb = get_average_chroma_index(pb1, pb2, pb3, pb4);
    unsigned avgpr = get_average_chroma_index(pr1, pr2, pr3, pr4);
    for(int i = 0; i < Seq_length(block); i++) {
        ((pixel)Seq_get(block, i))->avg_pb = avgpb;
        ((pixel)Seq_get(block, i))->avg_pr = avgpr;
    }

}

/* compress_dct_help
 * purpose: call dct_transformation module to convert y values to dct values
 * input: Hanson_Seq block
 * output: none
 */ 
void compress_dct_help(Hanson_Seq block)
{
    float y1 = ((pixel)Seq_get(block, 0))->composite->y;
    float y2 = ((pixel)Seq_get(block, 1))->composite->y;
    float y3 = ((pixel)Seq_get(block, 2))->composite->y;
    float y4 = ((pixel)Seq_get(block, 3))->composite->y;
    for(int i = 0; i < Seq_length(block); i++) {
        Dct currblock = y_to_dct(y1, y2, y3, y4);
        Dct *currpix = &((pixel)Seq_get(block, i))->dct;
        *currpix = currblock;
    }
}

/* compress_bitpack_help
 * purpose: call bitpack module to pack dct and average chroma values to a 
 *          single word
 * input: Hanson_Seq block
 * output: none
 */ 
void compress_bitpack_help(Hanson_Seq block)
{
    unsigned a = ((pixel)Seq_get(block, 0))->dct->a;
    signed b = ((pixel)Seq_get(block, 0))->dct->b;
    signed c = ((pixel)Seq_get(block, 0))->dct->c;
    signed d = ((pixel)Seq_get(block, 0))->dct->d;
    unsigned pb = ((pixel)Seq_get(block, 0))->avg_pb;
    unsigned pr = ((pixel)Seq_get(block, 0))->avg_pr;
    uint64_t currblock = 0;
    currblock = Bitpack_newu(currblock, 9, 23, a);
    currblock = Bitpack_news(currblock, 5, 18, b);
    currblock = Bitpack_news(currblock, 5, 13, c);
    currblock = Bitpack_news(currblock, 5, 8, d);
    currblock = Bitpack_newu(currblock, 4, 4, pb);
    currblock = Bitpack_newu(currblock, 4, 0, pr);
    for(int i = 0; i < Seq_length(block); i++) {
        ((pixel)Seq_get(block, i))->word = currblock;
    }
}




/*****************************/
/*                           */
/*       Decompression       */ 
/*                           */
/*****************************/

/* cv_to_rgb_help
 * purpose: call rgb_to_cv module to convert cv values to rgb values
 * input: struct Pnm_ppm pixmap
 * output: none
 */ 
void cv_to_rgb_help(struct Pnm_ppm pixmap)
{
    for(int i = 0; i < (int)(pixmap.width * pixmap.height); i++) {
        int width = i % (int)pixmap.width;
        int height = i / (int)pixmap.width;
        double y, pb, pr;
        y = ((pixel)pixmap.methods->
                            at(pixmap.pixels, width, height))->composite->y;
        pb = ((pixel)pixmap.methods->
                            at(pixmap.pixels, width, height))->composite->pb;
        pr = ((pixel)pixmap.methods->
                            at(pixmap.pixels, width, height))->composite->pr;
        Pnm_rgb currpix = cv_to_rgb(y, pb, pr, pixmap.denominator);

        Pnm_rgb *cell_rgb = &((pixel)
                            UArray2_at(pixmap.pixels, width, height))->pnm_rgb;
        *cell_rgb = currpix;
    }
}

/* decompress_chroma_help
 * purpose: call average_chroma module to get quantized chroma values
 * input: struct Pnm_ppm pixmap
 * output: none
 */ 
void decompress_chroma_help(struct Pnm_ppm pixmap)
{
    for(int i = 0; i < (int)(pixmap.width * pixmap.height); i++) {
        int width = i % (int)pixmap.width;
        int height = i / (int)pixmap.width;
        unsigned avgpr, avgpb;
        avgpr = ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                                    avg_pr;
        avgpb = ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                                    avg_pb;
        double pb, pr;
        pb = get_average_chroma_value(avgpb);
        pr = get_average_chroma_value(avgpr);
        ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                composite->pb = pb;
        ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                composite->pr = pr;
    }
}

/* compress_dct_help
 * purpose: call dct_transformation module to convert dct values to y values
 * input: struct Pnm_ppm pixmap
 * output: none
 */ 
void decompress_dct_help(struct Pnm_ppm pixmap)
{
    for(int i = 0; i < (int)pixmap.height; i += 2) {
        for(int j = 0; j < (int)pixmap.width; j += 2) {
            Dct currblock = ((pixel)pixmap.methods->
                                            at(pixmap.pixels, j, i))->dct;
            Hanson_Seq y_vals = dct_to_y(currblock);
            ((pixel)pixmap.methods->at(pixmap.pixels, j, i))->composite->
                                            y = (*(float*)Seq_get(y_vals, 0));
            ((pixel)pixmap.methods->at(pixmap.pixels, j + 1, i))->composite->
                                            y = (*(float*)Seq_get(y_vals, 1));
            ((pixel)pixmap.methods->at(pixmap.pixels, j, i + 1))->composite->
                                            y = (*(float*)Seq_get(y_vals, 2));
            ((pixel)pixmap.methods->at(pixmap.pixels, j + 1, i + 1))->
                                            composite->
                                            y = (*(float*)Seq_get(y_vals, 3));
            for(int index = 0; index < 4; index ++) {
                free((float*)Seq_get(y_vals, index));
            }
            Seq_free(&y_vals);            
        }
    }
}

/* decompress_bitpack_help
 * purpose: call bitpack module to retrive dct and average chroma values 
 *          from the word
 * input: struct Pnm_ppm pixmap
 * output: none
 */ 
void decompress_bitpack_help(struct Pnm_ppm pixmap)
{
    signed b, c, d;
    unsigned a, pb, pr;
    for (int width = 0; width < (int)pixmap.width; width++) {
        for (int height = 0; height < (int)pixmap.height; height++) {
            uint64_t word = ((pixel)pixmap.methods->
                                    at(pixmap.pixels, width, height))->word;
            Dct currdct = ((pixel)pixmap.methods->
                                    at(pixmap.pixels, width, height))->dct;
            a = Bitpack_getu(word, 9, 23);
            b = Bitpack_gets(word, 5, 18);
            c = Bitpack_gets(word, 5, 13);
            d = Bitpack_gets(word, 5, 8);
            pb = Bitpack_getu(word, 4, 4);
            pr = Bitpack_getu(word, 4, 0);
            currdct->a = a;
            currdct->b = b;
            currdct->c = c;
            currdct->d = d;
            ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                            avg_pb = pb;
            ((pixel)pixmap.methods->at(pixmap.pixels, width, height))->
                                                            avg_pr = pr;
        }
    }   
}