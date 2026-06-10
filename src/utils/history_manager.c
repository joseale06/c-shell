#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils/history_manager.h"

#define MAX_HISTORY 100 // capacidad máxima de comandos en el historial por sesión.
static char history_path[512]; // almacena la ruta absoluta ~/.ucvsh_history.
static char *history_array[MAX_HISTORY];
static int history_count = 0;
static int current_history_index = -1; // -1 representa la entrada actual.

void init_history() {
    //busca en el OS la variable de entorno.
    char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
        // se construye la ruta /home/usr/.ucv_history
        snprintf(history_path, sizeof(history_path), "%s/.ucvsh_history", home_dir);
        
        // limpia el historial al iniciar (modo "w").
        FILE *file = fopen(history_path, "w");
        if (file != NULL) fclose(file);
    }

    // inicializar estructuras de control.
    history_count = 0;
    current_history_index = 0;
    for (int i = 0; i < MAX_HISTORY; i++) 
        history_array[i] = NULL;
}

void add_to_history(const char *cmd) {
    if (history_path[0] == '\0') return;

    FILE *file = fopen(history_path, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", cmd);
        fclose(file);
    }

    if (history_count < MAX_HISTORY) {
        history_array[history_count] = strdup(cmd);
        if (history_array[history_count] != NULL) history_count++;
    } else {
        // si el historial se llena, se rota el búfer liberando el más viejo.
        free(history_array[0]);
        for (int i = 1; i < MAX_HISTORY; i++) 
            history_array[i - 1] = history_array[i];
        
        history_array[MAX_HISTORY - 1] = strdup(cmd);
    }
    // cada vez que el usuario ejecuta un comando nuevo, el índice de navegación 
    // se resetea, colocándose al final de la lista.
    current_history_index = history_count;
}

// # --------------------------------------------------------------------------------

const char* get_history_previous() {
    // si el historial está vacío o se llegó al comando más antiguo (índice 0).
    if (history_count == 0 || current_history_index <= 0) {
        return NULL; 
    }
    
    current_history_index--;
    return history_array[current_history_index];
}

const char* get_history_next() {
    // si ya estamos en el último comando ejecutado y volvemos a presionar hacia abajo,
    // significa que queremos regresar a la línea vacía original para escribir algo nuevo.
    if (current_history_index >= history_count - 1) {
        current_history_index = history_count;
        return ""; // retorna cadena vacía para limpiar la pantalla.
    }
    
    current_history_index++;
    return history_array[current_history_index];
}

void free_history() {
    // se libera cada string reservado individualmente con strdup.
    for (int i = 0; i < history_count; i++) {
        if (history_array[i] != NULL) {
            free(history_array[i]);
            history_array[i] = NULL;
        }
    }
    history_count = 0;
}