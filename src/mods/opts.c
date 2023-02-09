/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "opts.h"
#include "error.h"
#include "assert.h"

int opts_init(opts_p opts)
{
    assert(opts);

    opts->input_format_list = 0;
    opts->input_format_binary = 0;
    opts->input_type_wif = 0;
    opts->input_type_hex = 0;
    opts->input_type_raw = 0;
    opts->input_type_string = 0;
    opts->input_type_decimal = 0;
    opts->input_type_binary = 0;
    opts->input_type_sbd = 0;
    opts->input_type_vanity = 0;
    opts->output_format_list = 0;
    opts->output_format_qrcode = 0;
    opts->output_format_binary = 0;
    opts->output_type_wif = 0;
    opts->output_type_hex = 0;
    opts->output_type_decimal = 0;
    opts->output_type_raw = 0;
    opts->output_type_p2pkh = 0;
    opts->output_type_p2wpkh = 0;
    opts->output_stream = 0;
    opts->output_grep = NULL;
    opts->compression_on = 0;
    opts->compression_off = 0;
    opts->network_test = 0;
    opts->rehashes = NULL;
    opts->host_name = NULL;
    opts->host_port = 0;
    opts->create = 0;
    opts->input_path = NULL;
    opts->output_path = NULL;
    opts->subcommand = NULL;

    return 1;
}

int opts_get(opts_p opts, int argc, char *argv[], char *opts_string)
{
    int r, o;

    assert(opts);
    assert(opts_string);

    r = opts_init(opts);
    ERROR_CHECK_NEG(r, "Could not initialize opts.");

    if (argc > 2 && argv[2][0] != '-')
    {
        opts->subcommand = argv[2];
    }

    // Turn off getopt errors. I print my own errors.
    opterr = 0;

    while ((o = getopt(argc, argv, opts_string)) != -1)
    {
        switch (o)
        {
            case 'l':
                opts->input_format_list = 1;
                break;
            case 'b':
                opts->input_format_binary = 1;
                opts->input_type_binary = 1;
                break;

            case 'w':
                opts->input_type_wif = 1;
                break;
            case 'h':
                opts->input_type_hex = 1;
                break;
            case 'r':
                opts->input_type_raw = 1;
                opts->input_format_binary = 1;
                break;
            case 's':
                opts->input_type_string = 1;
                break;
            case 'd':
                opts->input_type_decimal = 1;
                break;
            case 'x':
                opts->input_type_sbd = 1;
                break;
            case 'v':
                opts->input_type_vanity = 1;
                break;

            case 'L':
                opts->output_format_list = 1;
                break;
            case 'Q':
                opts->output_format_qrcode = 1;
                break;
            case 'B':
                opts->output_format_binary = 1;
                break;

            case 'W':
                opts->output_type_wif = 1;
                break;
            case 'H':
                opts->output_type_hex = 1;
                break;
            case 'D':
                opts->output_type_decimal = 1;
                break;
            case 'R':
                opts->output_type_raw = 1;
                opts->output_format_binary = 1;
                break;
            case 'P':
                opts->output_type_p2pkh = 1;
                break;
            case 'E':
                opts->output_type_p2wpkh = 1;
                break;

            case 'S':
                opts->output_stream = 1;
                break;

            case 'G':
                opts->output_grep = optarg;
                break;

            case 'C':
                opts->compression_on = 1;
                break;
            case 'U':
                opts->compression_off = 1;
                break;

            case 'T':
                opts->network_test = 1;
                break;

            case 'X':
                opts->rehashes = optarg;
                break;

            case 'n':
                opts->host_name = optarg;
                break;
            case 'p':
                opts->host_port = atoi(optarg);
                break;

            case 'c':
                opts->create = 1;
                break;

            case 'f':
                opts->input_path = optarg;
                break;
            case 'F':
                opts->output_path = optarg;
                break;

            case '?':
                if (isprint(optopt))
                {
                    error_log("Invalid option or missing option argument: '-%c'.", optopt);
                }
                else
                {
                    error_log("Invalid option character '\\x%x'.", optopt);
                }
                return -1;
        }
    }

    return 1;
}