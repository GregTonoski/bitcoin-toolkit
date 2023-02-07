/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef INPUT_H
#define INPUT_H 1

#include "mods/cJSON/cJSON.h"

int input_get(unsigned char **, size_t *);
int input_get_line(unsigned char **);
int input_get_json(cJSON **);

#endif