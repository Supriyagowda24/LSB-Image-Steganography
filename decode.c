#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Read and validate Decode args from argv */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];
    decInfo->output_fname = argv[3]; /* NULL if not provided */

    return e_success;
}

/*
 * Decode a byte from the LSBs of 8 image bytes
 * Description: reverse of encode_byte_to_lsb. Take the LSB of each of the 8
 *              bytes; the first one becomes the highest bit, so the byte is
 *              rebuilt by shifting left and OR-ing in the next LSB.
 */

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    char result = 0;

    for (int i = 0; i < 8; i++)
    {
        result = (char)((result << 1) | (image_buffer[i] & 1));
    }

    *data = result;

    return e_success;
}

/*
 * Decode a size (int) from the LSBs of 32 image bytes
 * Description: reverse of encode_size_to_lsb, rebuilds all 32 bits of the int
 */

Status decode_size_from_lsb(int *size, char *imageBuffer)
{
    int result = 0;

    for (int i = 0; i < 32; i++)
    {
        result = (result << 1) | (imageBuffer[i] & 1);
    }

    *size = result;

    return e_success;
}

/*
 * Open the stego image
 * Return Value: e_success or e_failure if the file cannot be opened
 * Description: opens in binary mode and skips the 54 byte BMP header, because
 *              the hidden data starts right after it.
 */

Status open_stego_file(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,
                "ERROR: Unable to open file %s\n",
                decInfo->stego_image_fname);
        return e_failure;
    }

    printf("INFO: Opened %s\n",
           decInfo->stego_image_fname);

    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        fprintf(stderr,
                "ERROR: Unable to skip BMP header\n");

        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

/*
 * Decode and verify the magic string
 * Input: expected magic string ("#*")
 * Return Value: e_success if it matches, e_failure if the image does not
 *               contain our hidden data (or is too short)
 * Description: decodes one character at a time (8 image bytes each) and
 *              compares it with the expected character.
 */

Status decode_magic_string(const char *magic_string,
                           DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    unsigned int len = strlen(magic_string);

    for (unsigned int i = 0; i < len; i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        decode_byte_from_lsb(&ch, buffer);

        if (ch != magic_string[i])
        {
            return e_failure;
        }
    }

    return e_success;
}

/*
 * Decode secret file extension
 * Description: first decodes the extension size (32 image bytes), checks it is
 *              sane (1 to 4 characters, so it fits extn_output_file), then
 *              decodes that many characters and null terminates them.
 */

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char size_buffer[32];
    int extn_size;

    if (fread(size_buffer, 1, 32,
              decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decode_size_from_lsb(&extn_size, size_buffer);

    if (extn_size <= 0 ||
        extn_size >= (int)sizeof(decInfo->extn_output_file))
    {
        return e_failure;
    }

    char buffer[8];
    char ch;

    for (int i = 0; i < extn_size; i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        decode_byte_from_lsb(&ch, buffer);
        decInfo->extn_output_file[i] = ch;
    }

    decInfo->extn_output_file[extn_size] = '\0';

    return e_success;
}

/*
 * Open (create) the output file
 * Description: uses the name given by the user; if none was given, the
 *              default name is "decoded" + the extension read from the image
 *              (e.g. decoded.txt). Opened in binary mode.
 */

Status open_output_file(DecodeInfo *decInfo)
{
    if (decInfo->output_fname != NULL)
    {
        snprintf(decInfo->final_output_fname,
                 sizeof(decInfo->final_output_fname),
                 "%s",
                 decInfo->output_fname);
    }
    else
    {
        printf("INFO: Output File not mentioned. Creating decoded%s as default\n",
               decInfo->extn_output_file);

        snprintf(decInfo->final_output_fname,
                 sizeof(decInfo->final_output_fname),
                 "decoded%s",
                 decInfo->extn_output_file);
    }

    decInfo->fptr_output =
        fopen(decInfo->final_output_fname, "wb");

    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr,
                "ERROR: Unable to create file %s\n",
                decInfo->final_output_fname);
        return e_failure;
    }

    printf("INFO: Opened %s\n",
           decInfo->final_output_fname);

    return e_success;
}

/*
 * Decode secret file size
 * Description: reads 32 image bytes and rebuilds the int, which tells us how
 *              many data bytes to decode next
 */

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;

    if (fread(buffer, 1, 32,
              decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decode_size_from_lsb(&size, buffer);

    if (size < 0)
    {
        return e_failure;
    }

    decInfo->size_output_file = size;

    return e_success;
}

/*
 * Decode secret file data
 * Description: repeats size_output_file times: read 8 image bytes, rebuild one
 *              byte from their LSBs, write it to the output file.
 */

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (long i = 0;
         i < decInfo->size_output_file;
         i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        decode_byte_from_lsb(&ch, buffer);

        if (fwrite(&ch, 1, 1,
                   decInfo->fptr_output) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}

/*
 * Perform the decoding
 * Description: runs the whole decode procedure in this order:
 *   open stego image -> verify magic string -> decode extension ->
 *   open output file -> decode secret size -> decode secret data
 */

Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n========================================\n");
    printf("        Decoding Procedure Started\n");
    printf("========================================\n\n");

    printf("INFO: Opening required files\n");

    if (open_stego_file(decInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to open stego image file\n");
        return e_failure;
    }

    printf("INFO: Decoding Magic String Signature\n");

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: \"%s\" does not appear to contain valid stego data\n",
                decInfo->stego_image_fname);

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    int output_was_provided =
        (decInfo->output_fname != NULL);

    if (!output_was_provided)
    {
        printf("INFO: Decoding Output File Extension\n");
    }

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to decode file extension\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }

    if (!output_was_provided)
    {
        printf("INFO: Done\n\n");
    }

    if (open_output_file(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    printf("INFO: Done. Opened all required files\n\n");

    printf("INFO: Decoding %s File Size\n",
           decInfo->final_output_fname);

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to decode file size\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Decoding %s File Data\n",
           decInfo->final_output_fname);

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to decode file data\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);

        return e_failure;
    }

    printf("INFO: Done\n");

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("\n========================================\n");
    printf("        Decoding Done Successfully\n");
    printf("========================================\n");

    return e_success;
}
