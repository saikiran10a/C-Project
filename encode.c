#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definition to read and validate CLA */
/* 
 * Inputs: Command line  arguments
 * Output: stego.bmp
 * Return Value: e_success or e_failure
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if ( strcmp( strstr(argv[2], "." ), ".bmp") == 0 )
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }
    if ( strcmp (strstr(argv[3], "."), ".txt") == 0 )
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    if ( argv[4] != NULL )
    {
        if ( strcmp (strstr(argv[4], "."), ".bmp") == 0 )
        {
            encInfo -> stego_image_fname = argv[4];
        }
        else
        {
            return e_failure;
        }
    }
    else 
    {
        encInfo -> stego_image_fname = "stego.bmp";
    }
    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
/* Function definiton to open files */
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
/* Get the file size using fseek by moving file pointer to last position */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}
/* Function definition to check capacity */
/* 
 * Inputs: source file and secret file
 * Output: Image capacity
 * Return Value: e_success or e_failure
 */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    
    if ( encInfo -> image_capacity > 16/*magicstring*/ + 32 + 32/*secret file extn*/ + 32 + (encInfo -> size_secret_file * 8) )
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
/* Function definition to copy bmp file header from source image to destination image  
 * Inputs: source image file
 * Output: stego image
 * Return Value: e_success
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];
    rewind(fptr_src_image);
    fread(str, sizeof(char), 54, fptr_src_image);
    fwrite(str, sizeof(char), 54, fptr_dest_image);
    return e_success;
}
/* Function definition to encode magic string  
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success or e_failure
 */
Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image( magic_string, strlen(magic_string), encInfo -> fptr_src_image, encInfo -> fptr_stego_image, encInfo );
    return e_success;
}
/* Function definition to encode data to image
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: none
 */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    for ( int i = 0; i < size; i++)
    {
        fread(encInfo -> image_data, sizeof(char), 8, fptr_src_image);
        encode_byte_to_lsb( data[i], encInfo -> image_data);
        fwrite(encInfo -> image_data, sizeof(char), 8, fptr_stego_image);
    }
}
/* Function definition to encode magic string
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: none
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    uint mask = 1 << 7;
    for ( int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
}
/* Function definition to encode size of secret file extension 
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[32];
    fread(str, sizeof(char), 32, fptr_src_image);
    encode_size_to_lsb(size, str);
    fwrite(str, sizeof(char), 32, fptr_dest_image);
    return e_success;
}
/* Function definition to encode size to lsb 
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status encode_size_to_lsb(int size, char *buffer)
{
    uint mask = 1 << 31;
    for ( int i = 0; i < 32; i++)
    {
        buffer[i] = (buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
}
/* Function definition to encode secret file extension 
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image( file_extn, strlen(file_extn), encInfo -> fptr_src_image, encInfo -> fptr_stego_image, encInfo);
    return e_success;
}
/* Function definition to encode secret file size 
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, sizeof(char), 32, encInfo -> fptr_src_image);
    encode_size_to_lsb(file_size, str);
    fwrite(str, sizeof(char), 32, encInfo -> fptr_stego_image);
    return e_success;
}
/* Function definition to encode secret file data 
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_buff[encInfo -> size_secret_file];
    rewind(encInfo -> fptr_secret);
    fread(secret_buff, sizeof(char), encInfo -> size_secret_file, encInfo -> fptr_secret);
    encode_data_to_image(secret_buff, strlen(secret_buff), encInfo -> fptr_src_image, encInfo -> fptr_stego_image, encInfo);
    return e_success;
}
/* Function definition to encode copy remaining data
 * Inputs: source image file and secret file
 * Output: stego image file
 * Return Value: e_success
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, sizeof(char), 1, fptr_src) > 0)
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }
    return e_success;
}

/* Function to encode data to stego image 
 * Check if file opened
 * Check if image file size is more than info
 * Check if bmp header is copied 
 * Check if magic string is encoded or not
 * Check if secret file extension size is encoded or not
 * Check if secret file extension is encoded or not
 * Check if secret file size is encoded or not
 * Check if secret file data is encoded or not 
 * Chech if remaining data is copied or not
 */
Status do_encoding(EncodeInfo *encInfo)
{
    if ( open_files(encInfo) == e_success)
    {
        printf("File are opened successfully\n");
        if ( check_capacity(encInfo) == e_success)
        {
            printf("Check capacity is successfully completed\n");
            if ( copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success )
            {
                printf("Copy bmp header is successfully comlepted\n");
                if ( encode_magic_string(MAGIC_STRING, encInfo) == e_success )
                {
                    printf("Encode the magic string successful\n");
                    strcpy (encInfo -> extn_secret_file, strstr(encInfo -> secret_fname, "."));
                    if (encode_secret_file_extn_size( strlen(encInfo -> extn_secret_file), encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
                    {
                        printf("Encode sizeof secret file extension is succefully completed\n");
                        if (encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
                        {
                            printf("Encode secret file extension is succefully completed\n");
                            if (encode_secret_file_size(encInfo -> size_secret_file, encInfo) == e_success)
                            {
                                printf("Encode secret file size is successfully completed\n");
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encode secret file data is successfully completed\n");
                                    if ( copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
                                    {
                                        printf("Remaining data is successfully copied\n");
                                    }
                                    else
                                    {
                                        printf("Copy remaining data is failure\n");
                                    }
                                }
                                else
                                {
                                    printf("Encode secret file data is failure\n");
                                }
                            }
                            else
                            {
                                printf("Encode secret file size is failure\n");
                            }
                        }
                        else
                        {
                            printf("Encode secret file extension is failure\n");
                        }
                    }
                    else
                    {
                        printf("Encode sizeof secret file extension is failure\n");
                    }
                }
                else
                {
                    printf("Encode the magic string failure\n");
                }
            }
            else
            {
                printf("Copy bmp header is failure\n");
            }
        }
        else
        {
            printf("Check capacity is failure\n");
        }
    }
    else
    {
        printf("Open files function failure\n");
    }
    return e_success;
}

