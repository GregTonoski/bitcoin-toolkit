#ifndef PRIVKEY_H
#define PRIVKEY_H 1

#define PRIVKEY_LENGTH      32
#define PRIVKEY_COMP_LENGTH 33

typedef struct {
	unsigned char data[PRIVKEY_LENGTH];
} PrivKey;

typedef struct {
	unsigned char data[PRIVKEY_COMP_LENGTH];
} PrivKeyComp;

PrivKey     privkey_new(void);
PrivKeyComp privkey_compress(PrivKey);
PrivKeyComp privkey_new_compressed(void);
char       *privkey_to_hex(PrivKey);
char       *privkey_compressed_to_hex(PrivKeyComp);

#endif