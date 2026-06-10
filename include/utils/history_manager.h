#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

void init_history(void);
void add_to_history(const char *cmd);
const char* get_history_previous(void);
const char* get_history_next(void);
void free_history(void);

#endif