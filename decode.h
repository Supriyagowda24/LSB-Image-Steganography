#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding the secret file from the stego image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output (decoded secret) file info */
    char *output_fname;
    FILE *fptr_output;
    char extn_output_file[5];
    char final_output_fname[100];
    long size_output_file;

} DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open the stego image and skip its 54 byte header */
Status open_stego_file(DecodeInfo *decInfo);

/* Open (create) the output file */
Status open_output_file(DecodeInfo *decInfo);

/* Decode and verify the magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data and write it to the output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from the LSBs of 8 image bytes */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decode a size (int) from the LSBs of 32 image bytes */
Status decode_size_from_lsb(int *size, char *imageBuffer);

#endif
