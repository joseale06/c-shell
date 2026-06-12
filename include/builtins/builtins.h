#ifndef BUILTINS_H
#define BUILTINS_H

#include "../core/parser.h"

// retorna 1 si el comando era un builtin y fue ejecutado; 0 si es comando externo.
// el resultado de la ejecución se guarda en el puntero last_status.
int try_execute_builtin(CommandStruct *cmd, int *last_status);

#endif