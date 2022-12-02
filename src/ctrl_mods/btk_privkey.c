/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include "mods/privkey.h"
#include "mods/network.h"
#include "mods/input.h"
#include "mods/error.h"
#include "mods/json.h"

#define INPUT_NEW               1
#define INPUT_WIF               2
#define INPUT_HEX               3
#define INPUT_RAW               4
#define INPUT_STR               5
#define INPUT_DEC               6
#define INPUT_BLOB              7
#define INPUT_GUESS             8
#define INPUT_SBD               9
#define OUTPUT_WIF              1
#define OUTPUT_HEX              2
#define OUTPUT_RAW              3
#define OUTPUT_DEC              4
#define OUTPUT_COMPRESS         1
#define OUTPUT_UNCOMPRESS       2
#define OUTPUT_COMPRESSION_BOTH 3
#define OUTPUT_MAINNET          1
#define OUTPUT_TESTNET          2
#define TRUE                    1
#define FALSE                   0
#define OUTPUT_BUFFER           150
#define OUTPUT_HASH_MAX         100
#define HASH_WILDCARD           "w"

#define INPUT_SET(x)            if (input_format == FALSE) { input_format = x; } else { error_log("Cannot use multiple input format flags."); return -1; }
#define OUTPUT_SET(x)           if (output_format == FALSE) { output_format = x; } else { error_log("Cannot use multiple output format flags."); return -1; }
#define COMPRESSION_SET(x)      if (output_compression == FALSE) { output_compression = x; } else { output_compression = OUTPUT_COMPRESSION_BOTH; }

int btk_privkey_output_hashes_process(char *);
int btk_privkey_output_hashes_comp(const void *, const void *);

static int input_format       = FALSE;
static int output_format      = FALSE;
static int output_compression = FALSE;
static int output_network     = FALSE;
static char *output_hashes    = NULL;
static char *output_hashes_arr[OUTPUT_HASH_MAX];

int btk_privkey_init(int argc, char *argv[])
{
	int o;
	char *command = NULL;

	command = argv[1];

	while ((o = getopt(argc, argv, "nwhrsdbxWHRCUTDMS:")) != -1)
	{
		switch (o)
		{
			// Input format
			case 'n':
				INPUT_SET(INPUT_NEW);
				break;
			case 'w':
				INPUT_SET(INPUT_WIF);
				break;
			case 'h':
				INPUT_SET(INPUT_HEX);
				break;
			case 'r':
				INPUT_SET(INPUT_RAW);
				break;
			case 's':
				INPUT_SET(INPUT_STR);
				break;
			case 'd':
				INPUT_SET(INPUT_DEC);
				break;
			case 'b':
				INPUT_SET(INPUT_BLOB);
				break;
			case 'x':
				INPUT_SET(INPUT_SBD);
				break;

			// Output format
			case 'W':
				OUTPUT_SET(OUTPUT_WIF);
				break;
			case 'H':
				OUTPUT_SET(OUTPUT_HEX);
				break;
			case 'R':
				OUTPUT_SET(OUTPUT_RAW);
				break;
			case 'D':
				OUTPUT_SET(OUTPUT_DEC);
				break;
			
			// Output Compression
			case 'C':
				COMPRESSION_SET(OUTPUT_COMPRESS);
				break;
			case 'U':
				COMPRESSION_SET(OUTPUT_UNCOMPRESS);
				break;

			// Other options
			case 'S':
				output_hashes = optarg;
				break;

			// Network Options
			case 'T':
				output_network = OUTPUT_TESTNET;
				break;
			case 'M':
				output_network = OUTPUT_MAINNET;
				break;

			// Unknown option
			case '?':
				error_log("See 'btk help %s' to read about available argument options.", command);
				if (isprint(optopt))
				{
					error_log("Invalid command option or argument required: '-%c'.", optopt);
				}
				else
				{
					error_log("Invalid command option character '\\x%x'.", optopt);
				}
				return -1;
		}
	}

	if (input_format == FALSE)
	{
		input_format = INPUT_GUESS;
	}

	if (output_format == FALSE)
	{
		output_format = OUTPUT_WIF;
	}

	return 1;
}

int btk_privkey_main(void)
{
	int r, i;
	int N = 0;
	int output_len;
	int hash_count = 0;
	int hash_count_total = 0;
	PrivKey key = NULL;
	unsigned char *input_uc = NULL;
	char *input_sc = NULL;
	char output[OUTPUT_BUFFER];
	unsigned char uc_output[OUTPUT_BUFFER];

	json_init();

	// This loop provides support for list processing
	do
	{
	
		key = malloc(privkey_sizeof());
		if (key == NULL)
		{
			error_log("Memory allocation error.");
			return -1;
		}

		switch (input_format)
		{
			case INPUT_NEW:
				r = privkey_new(key);
				if (r < 0)
				{
					error_log("Could not generate a new private key.");
					return -1;
				}

				break;
			case INPUT_WIF:
				r = input_get_str(&input_sc, NULL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_wif(key, input_sc);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_HEX:
				r = input_get_str(&input_sc, NULL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_hex(key, input_sc);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_RAW:
				r = input_get_from_pipe(&input_uc);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_raw(key, input_uc, r);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_STR:
				r = input_get_str(&input_sc, NULL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_str(key, input_sc);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_DEC:
				r = input_get_str(&input_sc, NULL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_dec(key, input_sc);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_BLOB:
				r = input_get_from_pipe(&input_uc);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_blob(key, input_uc, r);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_SBD:
				r = input_get_str(&input_sc, NULL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_sbd(key, input_sc);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
			case INPUT_GUESS:
				r = input_get(&input_uc, NULL, INPUT_GET_MODE_ALL);
				if (r < 0)
				{
					error_log("Could not get input.");
					return -1;
				}

				r = privkey_from_guess(key, input_uc, r);
				if (r < 0)
				{
					error_log("Could not calculate private key from input.");
					return -1;
				}

				break;
		}

		if (!key)
		{
			error_log("Could not calculate private key from input.");
			return -1;
		}

		if (privkey_is_zero(key))
		{
			error_log("Invalid private key. Key value cannot be zero.");
			return -1;
		}

		switch (output_compression)
		{
			case FALSE:
				break;
			case OUTPUT_COMPRESS:
				privkey_compress(key);
				break;
			case OUTPUT_UNCOMPRESS:
				privkey_uncompress(key);
				break;
			case OUTPUT_COMPRESSION_BOTH:
				privkey_uncompress(key);
				break;
		}

		switch (output_network)
		{
			case FALSE:
				break;
			case OUTPUT_MAINNET:
				network_set_main();
				break;
			case OUTPUT_TESTNET:
				network_set_test();
				break;
		}

		N = 0;
		hash_count = 0;
		hash_count_total = 0;

		for (i = 0; i < OUTPUT_HASH_MAX; i++)
		{
			output_hashes_arr[i] = NULL;
		}

		if (output_hashes != NULL)
		{
			r = btk_privkey_output_hashes_process(input_sc);
			if (r < 0)
			{
				error_log("Error while processing hash argument [-S].");
				return -1;
			}
			else if (r == 0)
			{
				// TODO - Why am i returning a successful value here?
				return 1;
			}

			if (input_format == INPUT_STR || input_format == INPUT_BLOB)
			{
				hash_count_total = 1;
			}
		}

		do
		{
			if (output_hashes_arr[N] != NULL)
			{
				hash_count = atoi(output_hashes_arr[N]) - hash_count_total;
				for (i = 0; i < hash_count; i++)
				{
					r = privkey_rehash(key);
					if (r < 0)
					{
						error_log("Unable to hash private key.");
						return -1;
					}
				}
				hash_count_total += hash_count;
				N++;
			}

			do
			{
				memset(output, 0, OUTPUT_BUFFER);
				memset(uc_output, 0, OUTPUT_BUFFER);

				switch (output_format)
				{
					case OUTPUT_WIF:
						r = privkey_to_wif(output, key);
						if (r < 0)
						{
							error_log("Could not convert private key to WIF format.");
							return -1;
						}
						r = json_add(output);
						if (r < 0)
						{
							error_log("Error while generating JSON.");
							return -1;
						}
						break;
					case OUTPUT_HEX:
						r = privkey_to_hex(output, key, output_compression);
						if (r < 0)
						{
							error_log("Could not convert private key to hex format.");
							return -1;
						}
						r = json_add(output);
						if (r < 0)
						{
							error_log("Error while generating JSON.");
							return -1;
						}
						break;
					case OUTPUT_RAW:
						r = privkey_to_raw(uc_output, key, output_compression);
						if (r < 0)
						{
							error_log("Could not convert private key to raw format.");
							return -1;
						}
						output_len = r;
						for (i = 0; i < output_len; ++i)
						{
							putchar(uc_output[i]);
						}
						break;
					case OUTPUT_DEC:
						r = privkey_to_dec(output, key);
						if (r < 0)
						{
							error_log("Could not convert private key to decimal format.");
							return -1;
						}
						r = json_add(output);
						if (r < 0)
						{
							error_log("Error while generating JSON.");
							return -1;
						}
						break;
				}

				if (output_compression == OUTPUT_COMPRESSION_BOTH)
				{
					if (privkey_is_compressed(key))
					{
						privkey_uncompress(key);
						break;
					}
					else
					{
						privkey_compress(key);
					}
				}
			}
			while (output_compression == OUTPUT_COMPRESSION_BOTH);
		}
		while (output_hashes_arr[N] != NULL);

		if (input_sc != NULL)
		{
			free(input_sc);
		}
		if (input_uc != NULL)
		{
			free(input_uc);
		}

		free(key);

	}
	while (input_available());

	if (output_format != OUTPUT_RAW)
	{
		json_print();
		json_free();
	}

	return 1;
}

int btk_privkey_cleanup(void)
{
	return 1;
}

int btk_privkey_output_hashes_process(char *input_str)
{
	size_t i, j, len, N = 0;
	size_t output_hashes_len = 0;
	char *wild = NULL;

	// Save a pointer to the start of output_hashes so that it can be reset
	// after processing.
	output_hashes_len = strlen(output_hashes);

	if (output_hashes != NULL)
	{
		// Parsing string
		output_hashes_arr[N] = strtok(output_hashes, ",");
		while (output_hashes_arr[N] != NULL)
		{
			output_hashes_arr[++N] = strtok(NULL, ",");
		}

		// Valid token check
		for (i = 0; i < N; i++)
		{
			if (output_hashes_arr[i][0] == '-')
			{
				error_log("Argument cannot contain a negative number: %s", output_hashes_arr[i]);
				return -1;
			}
			else if (strcmp(output_hashes_arr[i], HASH_WILDCARD) == 0)
			{
				if (input_format != INPUT_STR && input_format != INPUT_DEC)
				{
					error_log("Can not use wildcard '%s' with current input mode.", output_hashes_arr[i]);
					return -1;
				}

				for (j = strlen(input_str); isdigit(input_str[j-1]); j--)
				{
					if (j == 0)
					{
						break;
					}
				}
				if (strlen(input_str + j) > 0)
				{
					wild = input_str + j;
				}
			}
			else
			{
				len = strlen(output_hashes_arr[i]);
				for (j = 0; j < len; j++)
				{
					if (!isdigit(output_hashes_arr[i][j]))
					{
						error_log("Argument contains unexpected character: %c", output_hashes_arr[i][j]);
						return -1;
					}
				}
			}
		}

		// Max number for wildcard is 1000000. Ignore anything greater.
		if (wild != NULL && atoi(wild) > 1000000)
		{
			wild = NULL;
		}

		// Duplicate check
		for (i = 0; i < N; i++)
		{
			for (j = i + 1; j < N; j++)
			{
				if (strcmp(output_hashes_arr[i], output_hashes_arr[j]) == 0)
				{
					error_log("Argument cannot contain duplicate numbers: %s", output_hashes_arr[i]);
					return -1;
				}
			}
			if (wild != NULL && strcmp(output_hashes_arr[i], wild) == 0)
			{
				wild = NULL;
			}
		}

		// Sort
		qsort(output_hashes_arr, N, sizeof(char *), btk_privkey_output_hashes_comp);

		// If all we have is a wildcard and no wildcard value,
		// return zero which tells the caller not to print any output.
		if (strcmp(output_hashes_arr[0], HASH_WILDCARD) == 0 && wild == NULL)
		{
			return 0;
		}

		// Wildcard substitute
		if (strcmp(output_hashes_arr[N-1], HASH_WILDCARD) == 0)
		{
			if (wild != NULL)
			{
				output_hashes_arr[N-1] = wild;
			}
			else
			{
				output_hashes_arr[N-1] = NULL;
			}
		}
	}

	// Undo the strtok changes to output_hashes for use in next item in input
	// list (if exists).
	for (i = 0; i < output_hashes_len; i++)
	{
		if (output_hashes[i] == '\0')
		{
			output_hashes[i] = ',';
		}
	}

	return 1;
}

int btk_privkey_output_hashes_comp(const void *i, const void *j)
{
	if (strcmp(*(char **)i, HASH_WILDCARD) == 0)
	{
		return 1;
	}
	if (strcmp(*(char **)j, HASH_WILDCARD) == 0)
	{
		return -1;
	}
	return (atoi(*(char **)i) - atoi(*(char **)j));
}

