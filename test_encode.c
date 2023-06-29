/*
Name : A Saikiran

Date : 19-03-2023

Sample input :  gcc *.c

Sample output : ./a.out -e beautiful.bmp secret.txt out.bmp

Selected encoding
Read and validate is successful
File are opened successfully
width = 1024
height = 768
Check capacity is successfully completed
Copy bmp header is successfully comlepted
Encode the magic string successful
Encode sizeof secret file extension is succefully completed
Encode secret file extension is succefully completed
Encode secret file size is successfully completed
Encode secret file data is successfully completed
Remaining data is successfully copied
Encoding is successfully completed

-------------------------------------------------------------------------
				./a.out -d out.bmp

Selected decoding
Read and validate is successful
Open decode files opened succussfully
Decode magic string succussfull
Decode secret file extension size successfull
Decode secret file extension is successfull
Decode size of secret file is successfull
Decode secret file data is successfull
Decoding is successfully completed

*/

#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "encode.h"
#include "types.h"


int main(int argc, char *argv[])
{
    if (check_operation_type(argv) == e_encode)
    {
        printf("Selected encoding\n");
        EncodeInfo encInfo;
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validate is successful\n");
            if ( do_encoding(&encInfo) == e_success )
            {
                printf("Encoding is successfully completed\n");
            }
            else
            {
                printf("Encoding is failure\n");
            }
        }
        else
        {
            printf("Read and validate is falure\n");
        }
    }
    else if (check_operation_type(argv) == e_decode)
    {
        printf("Selected decoding\n");
        DecodeInfo decInfo;
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and validate is successful\n");
            if ( do_decoding(&decInfo) == e_success )
            {
                printf("Decoding is successfully completed\n");
            }
            else
            {
                printf("Decoding is failure\n");
            }
        }
        else
        {
            printf("Read and validate is falure\n");
        }
    }
    else
    {
        printf("Please pass the correct option --> -e/-d\n");
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if ( strcmp(argv[1], "-e") == 0 )
    {
        return e_encode;
    }
    else if ( strcmp(argv[1], "-d") == 0 )
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
