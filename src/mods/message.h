#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <stdint.h>

typedef struct Message *Message;

Message message_new(const char *, unsigned char *, size_t);
size_t message_serialize(Message, unsigned char **);
size_t message_deserialize(unsigned char *, Message *, size_t);
void message_free(Message);

#endif
