/*
 * Copyright (c) 2023 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef STUB_H
#define STUB_H 1

typedef struct leveldb_t leveldb_t;
typedef struct leveldb_cache_t leveldb_cache_t;
typedef struct leveldb_comparator_t leveldb_comparator_t;
typedef struct leveldb_env_t leveldb_env_t;
typedef struct leveldb_filelock_t leveldb_filelock_t;
typedef struct leveldb_filterpolicy_t leveldb_filterpolicy_t;
typedef struct leveldb_iterator_t leveldb_iterator_t;
typedef struct leveldb_logger_t leveldb_logger_t;
typedef struct leveldb_options_t leveldb_options_t;
typedef struct leveldb_randomfile_t leveldb_randomfile_t;
typedef struct leveldb_readoptions_t leveldb_readoptions_t;
typedef struct leveldb_seqfile_t leveldb_seqfile_t;
typedef struct leveldb_snapshot_t leveldb_snapshot_t;
typedef struct leveldb_writablefile_t leveldb_writablefile_t;
typedef struct leveldb_writebatch_t leveldb_writebatch_t;
typedef struct leveldb_writeoptions_t leveldb_writeoptions_t;

enum { leveldb_no_compression = 0, leveldb_snappy_compression = 1 };

leveldb_options_t* leveldb_options_create();
void leveldb_options_set_compression(leveldb_options_t*, int);
void leveldb_options_set_create_if_missing(leveldb_options_t*, unsigned char);
void leveldb_options_set_error_if_exists(leveldb_options_t*, unsigned char);
leveldb_t* leveldb_open(const leveldb_options_t* options, const char* name, char** errptr);
leveldb_readoptions_t* leveldb_readoptions_create();
leveldb_iterator_t* leveldb_create_iterator(leveldb_t* db, const leveldb_readoptions_t* options);
void leveldb_iter_seek_to_first(leveldb_iterator_t*);
void leveldb_iter_seek(leveldb_iterator_t*, const char* k, size_t klen);
unsigned char leveldb_iter_valid(const leveldb_iterator_t*);
void leveldb_iter_next(leveldb_iterator_t*);
const char* leveldb_iter_key(const leveldb_iterator_t*, size_t* klen);
const char* leveldb_iter_value(const leveldb_iterator_t*, size_t* vlen);
char* leveldb_get(leveldb_t* db, const leveldb_readoptions_t* options, const char* key, size_t keylen, size_t* vallen, char** errptr);
leveldb_writeoptions_t* leveldb_writeoptions_create();
void leveldb_put(leveldb_t* db, const leveldb_writeoptions_t* options, const char* key, size_t keylen, const char* val, size_t vallen, char** errptr);
void leveldb_writeoptions_destroy(leveldb_writeoptions_t*);
void leveldb_readoptions_destroy(leveldb_readoptions_t*);
void leveldb_iter_destroy(leveldb_iterator_t*);
void leveldb_close(leveldb_t* db);

#endif