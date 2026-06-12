#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/builtins/cd.h"

int builtin_cd(char **args) {
    // si args[1] es NULL, significa que el usuario ingresó solo "cd".
    char *target_dir = args[1];

    if (target_dir == NULL) {
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            fprintf(stderr, "ucvsh: cd: HOME environment variable not set\n");
            return 1;
        }
    } 

    // ejecutar el cambio de directorio
    if (chdir(target_dir) != 0) {
        perror("ucvsh: cd"); // error por defecto del sistema.
        return 1;
    }

    return 0;
}