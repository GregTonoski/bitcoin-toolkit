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
#include "mods/json.h"
#include "mods/opts.h"
#include "mods/error.h"

int btk_address_vanity_match(char *, char *);
int btk_address_get_vanity_estimate(long int *, long int);

static int output_type_p2pkh = 0;
static int output_type_p2wpkh = 0;

int btk_address_main(opts_p opts, unsigned char *input, size_t input_len)
{
    int r;
    char output_str[BUFSIZ];
    char output_str2[BUFSIZ];
    PubKey pubkey = NULL;
    PrivKey privkey = NULL;

    assert(opts);

    if (opts->output_type_p2pkh) { output_type_p2pkh = opts->output_type_p2pkh; }
    if (opts->output_type_p2wpkh) { output_type_p2wpkh = opts->output_type_p2wpkh; }

    if (output_type_p2pkh && output_type_p2wpkh && opts->input_type_vanity)
    {
        error_log("Cannot use multiple output types when generating a vanity address.");
        return -1;
    }

    if (opts->input_type_wif && opts->input_type_hex)
    {
        error_log("Can not use multiple input type options.");
        return -1;
    }

    // Default to P2PKH
    if (!output_type_p2pkh && !output_type_p2wpkh)
    {
        output_type_p2pkh = 1;
    }

    privkey = malloc(privkey_sizeof());
    ERROR_CHECK_NULL(privkey, "Memory allocation error.");

    pubkey = malloc(pubkey_sizeof());
    ERROR_CHECK_NULL(pubkey, "Memory allocation error.");

    restart:

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
    else if (opts->input_type_vanity)
    {
         r = privkey_new(privkey);
        ERROR_CHECK_NEG(r, "Could not generate a new private key.");
        r = pubkey_get(pubkey, privkey);
        ERROR_CHECK_NEG(r, "Could not calculate public key.");
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
        memset(output_str, 0, BUFSIZ);

        r = address_get_p2wpkh(output_str, pubkey);
        ERROR_CHECK_NEG(r, "Could not calculate P2WPKH address.");

        if (!opts->input_type_vanity)
        {
            r = json_add(output_str);
            ERROR_CHECK_NEG(r, "Error while generating JSON.");
        }
    }

    if (output_type_p2pkh)
    {
        memset(output_str, 0, BUFSIZ);

        r = address_get_p2pkh(output_str, pubkey);
        ERROR_CHECK_NEG(r, "Could not calculate P2PKH address.");

        if (!opts->input_type_vanity)
        {
            r = json_add(output_str);
            ERROR_CHECK_NEG(r, "Error while generating JSON.");
        }
    }

    if (opts->input_type_vanity)
    {
        ERROR_CHECK_TRUE(opts->input_type_wif, "Cannot use wif and vanity input types together.");
        ERROR_CHECK_TRUE(opts->input_type_hex, "Cannot use hex and vanity input types together.");

        r = btk_address_vanity_match((char *)input, output_str);
        ERROR_CHECK_NEG(r, "Error matching vanity string.");

        if (r == 0)
        {
            goto restart;
        }

        r = json_add(output_str);
        ERROR_CHECK_NEG(r, "Error while generating JSON.");

        memset(output_str2, 0, BUFSIZ);

        r = privkey_to_wif(output_str2, privkey);
        ERROR_CHECK_NEG(r, "Could not convert private key to WIF format.");
        r = json_add(output_str2);
        ERROR_CHECK_NEG(r, "Error while generating JSON.");
    }

    free(pubkey);
    free(privkey);

    return 1;
}

int btk_address_vanity_match(char *input_str, char *output_str)
{
    int r, offset;
    size_t i, input_len;
    long int perms_total = 0, estimate;
    static long int perms_checked = 0;

    input_len = strlen(input_str);

    if (output_type_p2wpkh)
    {
        for (i = 0; i < input_len; ++i)
        {
            r = base32_get_raw(input_str[i]);
            ERROR_CHECK_NEG(r, "Input error.");
        }
        offset = 4;
    }
    else
    {
        for (i = 0; i < input_len; ++i)
        {
            r = base58_ischar(input_str[i]);
            ERROR_CHECK_FALSE(r, "Input error. Must only contain base58 characters.");
        }
        offset = 1;
    }

    perms_total = 1;
    for (i = 0; i < input_len; ++i)
    {
        if (isalpha(input_str[i]))
        {
            perms_total *= 29;
        }
        else
        {
            perms_total *= 58;
        }
    }

    r = btk_address_get_vanity_estimate(&estimate, perms_total);
    ERROR_CHECK_NEG(r, "Error computing estimated time left.");

    if (strncasecmp(input_str, output_str + offset, input_len) == 0)
    {
        return 1;
    }

    perms_checked++;
        
    if (perms_checked % 10000 == 0)
    {
        r = btk_address_get_vanity_estimate(&estimate, perms_checked);
        ERROR_CHECK_NEG(r, "Error computing estimated time left.");
        
        fprintf(stderr, "Estimated Seconds Remaining: %ld\n", estimate);
        fflush(stderr);
    }

    return 0;
}

int btk_address_get_vanity_estimate(long int *e, long int p)
{
    long double estimate;
    static long int perms = 0;
    static time_t start_time = 0;
    time_t elapsed_time = 0;

    if (!perms)
    {
        perms = p;
        start_time = time(NULL);
        return 1;
    }
    
    ERROR_CHECK_FALSE(perms, "Can not generate time estimate. Did not initialize permutations.");

    elapsed_time = time(NULL) - start_time;

    estimate = (long double)perms / (long double)p;
    estimate = (estimate * (long double)elapsed_time);
    estimate -= elapsed_time;
    if (estimate < 0)
    {
        *e = 0;
    }
    else
    {
        *e = (long int)estimate;
    }

    return 1;
}

int btk_address_requires_input(opts_p opts)
{
    assert(opts);

    return 1;
}