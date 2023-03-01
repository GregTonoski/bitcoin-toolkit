/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"
#include "mods/input.h"
#include "mods/base58.h"
#include "mods/base32.h"
#include "mods/output.h"
#include "mods/opts.h"
#include "mods/error.h"

static int output_type_p2pkh = 0;
static int output_type_p2wpkh = 0;

int btk_address_main(output_list *output, opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    char output_str[BUFSIZ];
    PubKey pubkey = NULL;
    PrivKey privkey = NULL;

    assert(opts);

    if (opts->output_type_p2pkh) { output_type_p2pkh = opts->output_type_p2pkh; }
    if (opts->output_type_p2wpkh) { output_type_p2wpkh = opts->output_type_p2wpkh; }

    // Default to P2PKH
    if (!output_type_p2pkh && !output_type_p2wpkh)
    {
        output_type_p2pkh = 1;
    }

    privkey = malloc(privkey_sizeof());
    ERROR_CHECK_NULL(privkey, "Memory allocation error.");

    pubkey = malloc(pubkey_sizeof());
    ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

    if (opts->input_type_wif)
    {
         r = privkey_from_wif(privkey, (char *)input);
        ERROR_CHECK_NEG(r, "Could not calculate private key from input.");
        r = pubkey_get(pubkey, privkey);
        ERROR_CHECK_NEG(r, "Could not calculate public key.");
    }
    else if (opts->input_type_hex)
    {
        r = pubkey_from_hex(pubkey, (char *)input);
        ERROR_CHECK_NEG(r, "Could not calculate public key from input.");
    }
    else
    {
        r = pubkey_from_guess(pubkey, input, input_len);
        if (r < 0)
        {
            error_clear();
            ERROR_CHECK_NEG(r, "Invalid or missing input type specified.");
        }
    }

    if (output_type_p2wpkh)
    {
        // Avoid uncompressed pubkey error if we are streaming and p2pkh is specified.
        if (pubkey_is_compressed(pubkey) || !opts->output_stream || !output_type_p2pkh)
        {
            memset(output_str, 0, BUFSIZ);

            r = address_get_p2wpkh(output_str, pubkey);
            ERROR_CHECK_NEG(r, "Could not calculate P2WPKH address.");

            *output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
            ERROR_CHECK_NULL(*output, "Memory allocation error.");
        }
    }

    if (output_type_p2pkh)
    {
        memset(output_str, 0, BUFSIZ);

        r = address_get_p2pkh(output_str, pubkey);
        ERROR_CHECK_NEG(r, "Could not calculate P2PKH address.");

        *output = output_append_new_copy(*output, output_str, strlen(output_str) + 1);
        ERROR_CHECK_NULL(*output, "Memory allocation error.");
    }

    free(pubkey);
    free(privkey);

    return 1;
}

int btk_address_requires_input(opts_p opts)
{
    assert(opts);

    return 1;
}

int btk_address_init(opts_p opts)
{
    assert(opts);

    return 1;
}

int btk_address_cleanup(opts_p opts)
{
    assert(opts);
    
    return 1;
}