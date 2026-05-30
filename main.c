#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024 // tamaño máximo del búfer de entrada.
#define MAX_ARGS 64     // cantidad máxima de argumentos por comando.

int main(int argc, char *argv[]) {
    char input[MAX_BUFFER];
    char *args[MAX_ARGS];

    while (1) {
        printf("\033[1;33mucvsh >\033[0m ");
        fflush(stdout); // asegura que el prompt 'ucvsh >' se imprima inmediatamente.

        // captura la entrada de forma segura
        // Si fgets() retorna NULL, significa que hubo un error o se detectó EOF (Ctrl + D)
        if(fgets(input, MAX_BUFFER, stdin) == NULL) break;

        // limpieza del salto de linea en el comando ingresado por el usuario.
        // se busca la posición de '\n' y se reemplaza por el carácter nulo '\0'.
        input[strcspn(input, "\n")] = '\0';

        // si se presiona enter (línea vacía), se vuelve al inicio.
        if (strlen(input) == 0) continue;

        //-------------------------------------------------------------------------------------
        // parser: análisis estructural y sintáctico

        int i = 0;
        char *token = strtok(input, " \t\r\n");
        /* Delimitadores: 
            espacio (' ')
            tabulación ('\t')
            salto de línea ('\n') 
            retorno de carro ('\r') (protección ante entrada con formato de Windows).

            ej. si el usuario ingresó "ls -l", input contiene:
            ['l', 's', ' ', '-', 'l', '\0'] (ya se ha eliminado el salto de linea)

            strtok leerá de izq. a der. el arreglo 'input', buscando cualquier caracter delimitador
            entonces, reemplazará los delimitadores -> ['l', 's', '\0', '-', 'l', '\0'], además,
            devuelve un puntero al inicio del array (var. token).
        */

        while (token != NULL && i < MAX_ARGS - 1) {
            args[i] = token;
            i++;
            token = strtok(NULL, " \t\r\n"); // llamadas subsecuentes a strtok usan NULL (tiene memoria interna).
        }
        // el arreglo de argumentos debe terminar en NULL para que execvp funcione posteriormente.
        args[i] = NULL; 

        // si no se capturó ningún argumento
        if (args[0] == NULL) continue;

        int run_in_background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            run_in_background = 1;
            args[i - 1] = NULL;
            i--;
        }

        #ifdef DEBUG
            printf("Comando a ejecutar: [%s]\n", args[0]);
            for (int j = 1; j < i; j++) 
                printf("Argumento %d: [%s]\n", j, args[j]);
            
            if (run_in_background) 
                printf("-> El comando se ejecutará en segundo plano (asíncrono).\n");
            
            printf("-----------------------------------\n");
        #endif
    }
    
    return 0;
}