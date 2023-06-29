#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function definition to decode the CLA */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	/*Check if the argument[2] is .bmp file or not */
	if ( strcmp(strstr(argv[2], "."), ".bmp") == e_success )
	{
		decInfo -> stego_image_fname = argv[2];
	}
	else
	{
		return e_failure;
	}
	/*Check if the argument[3] is .txt file or not */
	if ( argv[3] != NULL )
	{
		if ( strcmp(strstr(argv[3], "."), ".txt") == e_success )
		{
			decInfo -> secret_fname = argv[3];
		}
		else
		{
			return e_failure;
		}
	}
	/* Default .txt file if its not passed */
	else
	{
		decInfo -> secret_fname = "decoded.txt";
	}
	return e_success;
}
/* Function definition to open files */
Status open_decode_files(DecodeInfo *decInfo)
{
	decInfo -> fptr_stego_image = fopen(decInfo -> stego_image_fname, "r");
	// Do Error handling
	if (decInfo->fptr_stego_image == NULL)
  	{
   	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> stego_image_fname);
   	return e_failure;
   }
	decInfo -> fptr_secret = fopen(decInfo -> secret_fname, "w");
	// Do Error handling
  	if (decInfo-> fptr_secret == NULL)
  	{
   	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> secret_fname);
    	return e_failure;
  	}
  	return e_success;
}
/* Function definition to decode byte to lsb */
Status decode_byte_to_lsb(char *str, int index, int datatype_size, DecodeInfo *decInfo)
{
	uint lsb, mask = 00;
	//Loop to get lsb of each byte
	for ( int i = 0; i < datatype_size; i++)
	{
		lsb = (str[i] & 01) << (datatype_size - 1 - i);
		mask = mask | lsb;
	}
	decInfo -> image_buffer[index] = mask;
}
/* Function definition to decode data from image */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image, DecodeInfo *decInfo)
{
	char str[8];
	for (int i = 0; i < size; i++)
	{
		fread(str, sizeof(char), 8, decInfo -> fptr_stego_image);
		//Call byte from lsb to get all lsb to get the character 
		decode_byte_to_lsb(str, i, 8, decInfo);
		if (decInfo -> image_buffer[i] != data[i])
		{
			return e_failure;
		}
	}
	return e_success;
}
/* Function definition to decode magic string */
Status decode_magic_string( char *magic_string, DecodeInfo *decInfo)
{
	if (decode_data_from_image( magic_string, strlen(magic_string), decInfo -> fptr_stego_image, decInfo) == e_success)
	{
		return e_success;
	}
}
/* Function definition to decode size of secret file extension */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
	char file_ext_size[32];
	//Take 32 bytes to decode since integer is of 32 bytes
	fread(file_ext_size, sizeof(char), 32, decInfo -> fptr_stego_image);
	decode_byte_to_lsb(file_ext_size, 0, 32, decInfo); 
	if ( decInfo -> size_extn_file = decInfo -> image_buffer[0] )
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}
/* Function definition to decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
	char str[4] = ".txt";
	decode_data_from_image( str, strlen(str), decInfo -> fptr_stego_image, decInfo);
}
/* Function definition to decode size of secret file */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
	char file_size[32];
	fread(file_size, sizeof(char), 32, decInfo -> fptr_stego_image);
	decode_byte_to_lsb(file_size, 0, 32, decInfo); 
	if ( decInfo -> size_secret_file = decInfo -> image_buffer[0] )
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}
/* Function definition to decode secret file data */ 
Status decode_secret_data(DecodeInfo *decInfo)
{
	for (int i = 0; i < decInfo -> size_secret_file; i++)
	{
			char str[8];
			fread(str, sizeof(char), 8, decInfo -> fptr_stego_image);
			uint lsb, mask = 00;
			for ( int i = 0; i < 8; i++)
			{
				lsb = (str[i] & 01) << (7 - i);
				mask = mask | lsb;
			}
			//Print all data to decode.txt file
			fputc(mask,decInfo->fptr_secret);
	}
	return e_success;
}

/* Function to start decoding 
 1.Check function to open files
 2.Check function to decode magic string
 3.Check function to decode size of secret file extension
 4.Check function to decode secret file extension
 5.Check function to decode size of secret file data
 6.Check function to decode secret data
*/
Status do_decoding(DecodeInfo *decInfo)
{
	if (open_decode_files(decInfo) == e_success) 
	{
		printf("Open decode files opened succussfully\n");
		/* As magic string starts from 54th byte so move file pointer position to that byte */
		fseek(decInfo -> fptr_stego_image, 54, SEEK_SET);
		if (decode_magic_string(MAGIC_STRING, decInfo) == e_success )
		{
			printf("Decode magic string succussfull\n");
			if (decode_secret_file_extn_size(decInfo) == e_success)
			{
				printf("Decode secret file extension size successfull\n");
				if ( decode_secret_file_extn ( decInfo) == e_success )
				{
					printf("Decode secret file extension is successfull\n");
					if (decode_secret_file_size(decInfo) == e_success )
					{
						printf("Decode size of secret file is successfull\n");
						if(decode_secret_data(decInfo) == e_success) 
						{
							printf("Decode secret file data is successfull\n");
						}
						else
						{
							printf("Decode secret file data is failed\n");	
							return e_failure;
						}
					}
					else
					{
						printf("Decode size of secret file failed\n");
					}
				}
				else
				{
					printf("Decode secret file extension failed\n");
					return e_failure;
				}
			}
			else
			{
				printf("Decode secret file extension size failed\n");
				return e_failure;
			}
		} 
		else
		{
			printf("Decode magic string failed\n");
			return e_failure;
		}
	}
	else
	{
		printf("Open decode files failed\n");
		return e_failure;
	}
	return e_success;
}

