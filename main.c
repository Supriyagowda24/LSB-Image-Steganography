/*
NAME: SUPRIYA J
DATE: 16/09/2026
DESCRIPTION: This program is about a Steganography project using BMP images.
             It can encode a secret text file into a BMP image by modifying
             the LSB (Least Significant Bit) of image pixels.
             The program can decode the hidden text from the stego BMP image
             back to a readable text file.
             It can encode and decode magic strings to verify the presence
             of secret data.
             It supports hiding the secret file name extension and size
             along with the content.
             The program is implemented using file handling, structures,
             functions, pointers, and bitwise operations in C.
             Command prompt messages indicate the process.
*/

#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "decode.h"
#include "types.h"

/* Function prototypes */
OperationType check_operation_type(char *);
void print_encode_usage(char *argv[]);
void print_decode_usage(char *argv[]);
void print_usage(char *argv[]);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv);
        return e_failure;
    }

    /* Find out whether the user wants to encode (-e) or decode (-d) */
    OperationType op_type = check_operation_type(argv[1]);

    if (op_type == e_encode)
    {
        /* Encoding needs at least:
           program -e image secret */
        if (argc < 4)
        {
            print_encode_usage(argv);
            return e_failure;
        }

        EncodeInfo encInfo;

        /* Validate the arguments, then run the encoding */
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_failure)
            {
                return e_failure;
            }
        }
        else
        {
            print_encode_usage(argv);
            return e_failure;
        }
    }
    else if (op_type == e_decode)
    {
        /* Decoding needs at least:
           program -d stego_image */
        if (argc < 3)
        {
            print_decode_usage(argv);
            return e_failure;
        }

        DecodeInfo decInfo;

        /* Validate the arguments, then run the decoding */
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_failure)
            {
                return e_failure;
            }
        }
        else
        {
            print_decode_usage(argv);
            return e_failure;
        }
    }
    else
    {
        /* Option was neither -e nor -d */
        print_usage(argv);
        return e_failure;
    }

    return e_success;
}

/*
 * Check operation type
 * Input: the option string from argv[1]
 * Output: e_encode for "-e",
 *         e_decode for "-d",
 *         otherwise e_unsupported
 */

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }

    if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }

    return e_unsupported;
}

/* Print how to run encoding */
void print_encode_usage(char *argv[])
{
    printf("%s: Encoding: %s -e <.bmp file> <.txt file> [output file]\n",
           argv[0], argv[0]);
}

/* Print how to run decoding */
void print_decode_usage(char *argv[])
{
    printf("%s: Decoding: %s -d <.bmp file> [output file]\n",
           argv[0], argv[0]);
}

/* Print both usages */
void print_usage(char *argv[])
{
    print_encode_usage(argv);
    print_decode_usage(argv);
}