#ifndef TERMINAL_MANAGER_H
#define TERMINAL_MANAGER_H

#include <stddef.h> // standard definitions.

void enable_raw_mode(void);
void disable_raw_mode(void);
int read_input_raw(char *buffer, size_t size);

#endif