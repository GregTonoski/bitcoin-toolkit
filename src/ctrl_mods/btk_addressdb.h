/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTK_ADDRESSDB_H
#define BTK_ADDRESSDB_H 1

#include "mods/output.h"
#include "mods/opts.h"

int btk_addressdb_main(output_list *, opts_p, unsigned char *, size_t);
int btk_addressdb_requires_input(opts_p);

#endif