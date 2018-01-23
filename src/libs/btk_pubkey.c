/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the MIT License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/base58.h"
#include "mods/mem.h"
#include "mods/assert.h"

#define INPUT_BUFFER_SIZE    (PUBKEY_UNCOMPRESSED_LENGTH * 2) + 2

/*
 * Static Function Declarations
 */
static int btk_pubkey_read_input(void);

/*
 * Static Variable Declarations
 */
static unsigned char input_buffer[INPUT_BUFFER_SIZE];
static int flag_input_raw = 0;
static int flag_input_hex = 0;
static int flag_input_wif = 0;
static int flag_output_address = 0;
static int flag_output_hex = 0;
static int flag_output_raw = 0;
static int flag_format_newline = 0;

int btk_pubkey_main(int argc, char *argv[]) {
	int o;
	PubKey key = NULL;
	
	// Check arguments
	while ((o = getopt(argc, argv, "rhwAHRN")) != -1) {
		switch (o) {
			// Input flags
			case 'r':
				flag_input_raw = 1;
				break;
			case 'h':
				flag_input_hex = 1;
				break;
			case 'w':
				flag_input_wif = 1;
				break;

			// Output flags
			case 'A':
				flag_output_address = 1;
				break;
			case 'H':
				flag_output_hex = 1;
				break;
			case 'R':
				flag_output_raw = 1;
				break;

			// Format flags
			case 'N':
				flag_format_newline = 1;
				break;

			case '?':
				if (isprint(optopt))
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				return EXIT_FAILURE;
		}
	}
	
	// Process input flags
	if (flag_input_raw) {
		int cnt;
		PrivKey priv;
		cnt = btk_pubkey_read_input();
		if (cnt < PRIVKEY_LENGTH) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		priv = privkey_from_raw(input_buffer);
		key = pubkey_get(priv);
		privkey_free(priv);
	} else if (flag_input_hex) {
		int i, cnt;
		PrivKey priv;
		cnt = btk_pubkey_read_input();
		if (cnt < PRIVKEY_LENGTH * 2) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		for (i = 0; i < cnt; ++i) {
			if ((input_buffer[i] < 'A' || input_buffer[i] > 'F') && (input_buffer[i] < '0' || input_buffer[i] > '9') && (input_buffer[i] < 'a' || input_buffer[i] > 'z')) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
		}
		priv = privkey_from_hex((char *)input_buffer);
		key = pubkey_get(priv);
		privkey_free(priv);
	} else if (flag_input_wif) {
		int i, cnt;
		PrivKey priv;
		cnt = btk_pubkey_read_input();
		// TODO - make '51' globally defined value so it can be used here and in btk_privkey
		if (cnt < 51) {
			fprintf(stderr, "Error: Invalid input.\n");
			return EXIT_FAILURE;
		}
		// TODO - this doesn't gracefully handle white space chars at the end of the WIF input
		for (i = 0; i < cnt; ++i) {
			if (!base58_ischar(input_buffer[i])) {
				fprintf(stderr, "Error: Invalid input.\n");
				return EXIT_FAILURE;
			}
		}
		input_buffer[cnt] = '\0';
		priv = privkey_from_wif((char *)input_buffer);
		key = pubkey_get(priv);
		privkey_free(priv);
	} else {
		fprintf(stderr, "Error: Must specify input flag.\n");
		return EXIT_FAILURE;
	}
	
	// Ensure we have a key
	assert(key);
	
	// Set default output flag if none specified
	if (!flag_output_address && !flag_output_hex && !flag_output_raw)
		flag_output_address = 1;
	
	// Process output flags
	if (flag_output_address) {
		printf("%s", pubkey_to_address(key));
	} else if (flag_output_hex) {
		printf("%s", pubkey_to_hex(key));
	} else if (flag_output_raw) {
		size_t i, l;
		unsigned char *r;
		r = pubkey_to_raw(key, &l);
		for (i = 0; i < l; ++i) {
			printf("%c", r[i]);
		}
		FREE(r);
	}

	// Process format flags
	if (flag_format_newline)
			printf("\n");

	// Free allocated memory
	pubkey_free(key);

	return EXIT_SUCCESS;
}

// TODO - should this be merged with the function in btk_privkey, perhaps it's own module?
static int btk_pubkey_read_input(void) {
	int i, c;

	for (i = 0; i < INPUT_BUFFER_SIZE && (c = getchar()) != EOF; ++i)
		input_buffer[i] = c;

	return i;
}