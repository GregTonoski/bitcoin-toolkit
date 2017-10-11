#include <stdlib.h>
#include <stdint.h>
#include "txinput.h"
#include "hex.h"

TXInput txinput_from_rawhex(char *hex) {
	int i;
	TXInput r;
	char tx_index[9];
	char script_size[17];
	int script_size_length;
	
	// Transaction hash being spent
	for (i = 0; i < 32; ++i, hex += 2) {
		r.tx_hash[i] = hex_to_dec(hex[0], hex[1]);
	}
	
	/*
	
	// Output index of transcaction hash
	for (i = 0; i < 8; ++i, ++hex) {
		tx_index[i] = hex[0];
	}
	tx_index[i] = '\n';
	r.index = (int)strtol(tx_index, NULL, 16);
	
	// Unlocking Script Size
	if (hex_to_dec(hex[0], hex[1]) <= 0xfc) {
		r.script_size = hex_to_dec(hex[0], hex[1]);
		hex += 2;
	} else {
		if (hex_to_dec(hex[0], hex[1]) == 0xfd) {
			script_size_length = 2 * 2;
		} else if (hex_to_dec(hex[0], hex[1]) == 0xfe) {
			script_size_length = 4 * 2;
		} else if (hex_to_dec(hex[0], hex[1]) == 0xff) {
			script_size_length = 8 * 2;
		}
		
		hex += 2;
		for (i = 0; i < script_size_length; ++i, hex += 2) {
			script_size[i] = hex_to_dec(hex[0], hex[1]);
		}
		script_size[i] = '\0';
		r.script_size = (uint64_t)strtol(script_size, NULL, 16);
	}
	
	// Unlocking Script
	if ((script = malloc(r.script_size)) == NULL) {
		// TODO - Handle error here
	}
	for (i = 0; i < r.script_size; ++i, hex += 2) {
		
	}

	*/
	
	return r;
}
