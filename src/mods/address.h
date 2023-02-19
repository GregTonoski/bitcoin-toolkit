/*
 * Copyright (c) 2022 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef ADDRESS_H
#define ADDRESS_H 1

#include "pubkey.h"

int address_get_p2pkh(char *, PubKey);
int address_get_p2wpkh(char *, PubKey);
int address_from_wif(char *, char *);
int address_from_str(char *, char *);
int address_from_rmd160(char *, unsigned char *);
int address_from_sha256(char *, unsigned char *);
int address_from_p2sh_script(char *, unsigned char *);

#endif