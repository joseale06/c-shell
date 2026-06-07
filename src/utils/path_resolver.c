#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/utils/path_resolver.h"

char* resolve_path(char *cmd) {
    // consideramos si el usuario introdujo una ruta explícita (absoluta o relativa)
    if (strchr(cmd, '/') != NULL) {
        if (access(cmd, X_OK) == 0) {
            // se emplea strdup para devolver una copia en el Heap y permitir
            // validaciones posteriores.
            return strdup(cmd); 
        }
        return NULL; // si la ruta no existe o no es ejecutable.
    }

    // si no se tiene una ruta, se buscan en los directorios de la variable PATH
    char *path_env = getenv("PATH");
    if (path_env == NULL) return NULL; 
    
    // se crea una copia del PATH, ya que strtok modifica la cadena original.
    char *path_copy = strdup(path_env);
    if (path_copy == NULL) {
        perror("Error de memoria al gestionar el PATH");
        return NULL;
    }

    // separamos la copia usando mediante el caracter ':'
    char *directory = strtok(path_copy, ":");
    
    while (directory != NULL) {
        // se calcula la memoria exacta: dir + '/' + comando + '\0'
        size_t lenght = strlen(directory) + strlen(cmd) + 2;
        char *full_path = malloc(lenght);

        if (full_path == NULL) {
            free(path_copy);
            return NULL;
        }

        // concatena de forma segura (ej. "/usr/bin" + "/" + "ls")
        snprintf(full_path, lenght, "%s/%s", directory, cmd);

        // verificación de existencia del binario en la ruta procesada,
        // además de los permisos de ejecución
        if (access(full_path, X_OK) == 0) {
            free(path_copy); 
            return full_path;
        }

        free(full_path);
        directory = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}
