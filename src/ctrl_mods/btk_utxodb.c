/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <assert.h>
#include "mods/error.h"
#include "mods/input.h"
#include "mods/output.h"
#include "mods/hex.h"
#include "mods/pubkey.h"
#include "mods/script.h"
#include "mods/utxodb.h"
#include "mods/opts.h"

#define BTK_UTXODB_TX_LENGTH          32
#define BTK_UTXODB_MAX_ADDRESS_LENGTH 42
#define BTK_UTXODB_MAX_SCRIPT_LENGTH  100

int btk_utxodb_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    char address[BTK_UTXODB_MAX_ADDRESS_LENGTH];
    unsigned char input_raw[BTK_UTXODB_TX_LENGTH];
    UTXODBKey key = NULL;
    UTXODBValue value = NULL;

    (void)output;

    assert(opts);

    (void)input_len;

    ERROR_CHECK_NULL(opts->input_path, "Missing database path argument.");

    key = malloc(utxodb_sizeof_key());
    ERROR_CHECK_NULL(key, "Memory Allocation Error.");

    value = malloc(utxodb_sizeof_value());
    ERROR_CHECK_NULL(value, "Memory Allocation Error.");

    if (strlen((char *)input) != (UTXODB_TX_HASH_LENGTH * 2))
    {
        error_log("Input must be a %i byte hexidecimal string.", (UTXODB_TX_HASH_LENGTH * 2));
        return -1;
    }

    r = hex_str_to_raw(input_raw, (char *)input);
    ERROR_CHECK_NEG(r, "Can not convert hex input to raw binary.");

    r = utxodb_open(opts->input_path);
    ERROR_CHECK_NEG(r, "Could not open utxo database.");

    while ((r = utxodb_get(key, value, input_raw)) == 1)
    {
        printf("%"PRIu64",", utxodb_value_get_height(value));
        printf("%"PRIu64",", utxodb_key_get_vout(key));
        printf("%"PRIu64",", utxodb_value_get_amount(value));

        if (utxodb_value_has_address(value))
        {
            r = utxodb_value_get_address(address, value);
            ERROR_CHECK_NEG(r, "Can not get address from value.");
            
            if (r > 0)
            {
                printf("%s", address);
            }
        }

        printf("\n");
    }
    ERROR_CHECK_NEG(r, "Could not get record from utxo database.");

    utxodb_close();

    utxodb_value_free(value);
    free(key);
    free(value);

    return EXIT_SUCCESS;
}

int btk_utxodb_requires_input(opts_p opts)
{
    assert(opts);

    return 1;
}