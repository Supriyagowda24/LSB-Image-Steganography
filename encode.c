#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);

    uint size = ftell(fptr);

    rewind(fptr);

    return size;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ext = strrchr(argv[2], '.');

    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];

    ext = strrchr(argv[3], '.');

    if (ext == NULL ||
        strlen(ext) >= sizeof(encInfo->extn_secret_file))
    {
        return e_failure;
    }

    encInfo->secret_fname = argv[3];
    strcpy(encInfo->extn_secret_file, ext);

    if (argv[4] != NULL)
    {
        ext = strrchr(argv[4], '.');

        if (ext == NULL || strcmp(ext, ".bmp") != 0)
        {
            return e_failure;
        }

        encInfo->stego_image_fname = argv[4];
        encInfo->output_file_provided = 1;
    }
    else
    {
        encInfo->stego_image_fname = "steged_img.bmp";
        encInfo->output_file_provided = 0;

        printf("INFO: Output File not mentioned. Creating %s as default\n",
               encInfo->stego_image_fname);
    }

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    /* Src Image file */
    encInfo->fptr_src_image =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,
                "ERROR: Unable to open file %s\n",
                encInfo->src_image_fname);
        return e_failure;
    }

    printf("INFO: Opened %s\n",
           encInfo->src_image_fname);

    /* Secret file */
    encInfo->fptr_secret =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr,
                "ERROR: Unable to open file %s\n",
                encInfo->secret_fname);

        fclose(encInfo->fptr_src_image);

        return e_failure;
    }

    printf("INFO: Opened %s\n",
           encInfo->secret_fname);

    /* Stego Image file */
    encInfo->fptr_stego_image =
        fopen(encInfo->stego_image_fname, "wb");

    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr,
                "ERROR: Unable to open file %s\n",
                encInfo->stego_image_fname);

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);

        return e_failure;
    }

    printf("INFO: Opened %s\n",
           encInfo->stego_image_fname);

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    printf("INFO: Checking for %s size\n",
           encInfo->secret_fname);

    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    if (encInfo->size_secret_file == 0)
    {
        fprintf(stderr,
                "ERROR: \"%s\" is empty\n",
                encInfo->secret_fname);
        return e_failure;
    }

    printf("INFO: Done. Not Empty\n");

    printf("INFO: Checking for %s capacity to handle %s\n",
           encInfo->src_image_fname,
           encInfo->secret_fname);

    uint required_bytes =
        (uint)(strlen(MAGIC_STRING) * 8) +
        32 +
        32 +
        (uint)(strlen(encInfo->extn_secret_file) * 8) +
        (uint)(encInfo->size_secret_file * 8);

    if (encInfo->image_capacity < required_bytes)
    {
        fprintf(stderr,
                "ERROR: \"%s\" doesn't have the capacity to encode \"%s\"\n",
                encInfo->src_image_fname,
                encInfo->secret_fname);
        return e_failure;
    }

    printf("INFO: Done. Found OK\n");

    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = (data >> (7 - i)) & 1;

        image_buffer[i] =
            (char)((image_buffer[i] & 0xFE) | bit);
    }

    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        int bit = (size >> (31 - i)) & 1;

        imageBuffer[i] =
            (char)((imageBuffer[i] & 0xFE) | bit);
    }

    return e_success;
}

Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    char buffer[8];

    for (unsigned int i = 0;
         i < strlen(magic_string);
         i++)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        encode_byte_to_lsb(magic_string[i], buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status encode_secret_file_extn_size(int size,
                                    EncodeInfo *encInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32,
              encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    encode_size_to_lsb(size, buffer);

    if (fwrite(buffer, 1, 32,
               encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn,
                               EncodeInfo *encInfo)
{
    char buffer[8];

    for (unsigned int i = 0;
         i < strlen(file_extn);
         i++)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        encode_byte_to_lsb(file_extn[i], buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32,
              encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    encode_size_to_lsb((int)file_size, buffer);

    if (fwrite(buffer, 1, 32,
               encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    char buffer[8];

    rewind(encInfo->fptr_secret);

    while (fread(&ch, 1, 1,
                 encInfo->fptr_secret) == 1)
    {
        if (fread(buffer, 1, 8,
                  encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        encode_byte_to_lsb(ch, buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n========================================\n");
    printf("        Encoding Procedure Started\n");
    printf("========================================\n\n");

    printf("INFO: Opening required files\n");

    if (open_files(encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to open required files\n");
        return e_failure;
    }

    printf("INFO: Done\n\n");

    if (check_capacity(encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Copying Image Header\n");

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to copy BMP header\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Encoding Magic String Signature\n");

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to encode magic string\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Encoding %s File Extenstion\n",
           encInfo->secret_fname);

    if (encode_secret_file_extn_size(
            (int)strlen(encInfo->extn_secret_file),
            encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to encode secret file extension size\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (encode_secret_file_extn(
            encInfo->extn_secret_file,
            encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to encode secret file extension\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Encoding %s File Size\n",
           encInfo->secret_fname);

    if (encode_secret_file_size(
            encInfo->size_secret_file,
            encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to encode secret file size\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Encoding %s File Data\n",
           encInfo->secret_fname);

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to encode secret file data\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n\n");

    printf("INFO: Copying Left Over Data\n");

    if (copy_remaining_img_data(
            encInfo->fptr_src_image,
            encInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr,
                "ERROR: Failed to copy remaining image data\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    printf("INFO: Done\n");

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    printf("\n========================================\n");
    printf("        Encoding Done Successfully\n");
    printf("========================================\n");

    return e_success;
}
