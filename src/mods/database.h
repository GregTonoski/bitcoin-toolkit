/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef DATABASE_H
#define DATABASE_H 1

int database_open(char *);
int database_is_open(void);
int database_get(unsigned char **, size_t *, char *, size_t);
void database_close(void);

#endif
