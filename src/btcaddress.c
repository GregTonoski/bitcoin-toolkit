#include <stdio.h>
#include "mods/privkey.h"
#include "mods/pubkey.h"
#include "mods/address.h"

int main(void) {
	size_t i;
	int c;
	char hex[PRIVKEY_LENGTH * 2];

	// Geting input
	for (i = 0; (c = getchar()) != EOF && i < PRIVKEY_LENGTH * 2; ++i) {
		
		if (c >= 'A' && c <= 'F') {
			c -= 55;
		}
		
		if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
			hex[i] = c;
		} else {
			fprintf(stderr, "Invalid private key - Character: %c\n", c);
			return 1;
		}
	}
	if (i != PRIVKEY_LENGTH * 2) {
		fprintf(stderr, "Invalid private key - Length: %i\n", (int)i);
		return 1;
	}
	
	printf("Address: %s\n", address_get(pubkey_get(privkey_from_hex(hex))));
	
	return 0;
}
