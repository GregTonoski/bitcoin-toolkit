/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef INPUT_H
#define INPUT_H 1

#include "mods/cJSON/cJSON.h"

typedef struct input_item *input_item;
struct input_item {
    unsigned char *data;
    size_t len;
    input_item next;
};

int input_get(input_item *);
int input_get_line(input_item *);
int input_get_json(input_item *);
input_item input_append_item(input_item, input_item);
void input_free(input_item);

#endif