# ucvsh - Intérprete de Comandos (shell)

Este es un proyecto académico para la materia Sistemas Operativos de la Universidad Central de Venezuela (UCV).

## Objetivo

El objetivo principal de `ucvsh` es diseñar e implementar un intérprete de línea de comandos (shell) modular y funcional en C. Este proyecto busca profundizar en la comprensión de la interfaz del sistema operativo (POSIX) mediante la gestión directa de procesos, resolución de rutas y la evaluación de flujos de control lógico. La shell soporta la ejecución concurrente, procesos en segundo plano y la concatenación condicional de comandos a través de operadores estándar.

## Arquitectura del sistema

El proyecto está diseñado bajo un modelo de separación estricta de responsabilidades, dividiendo el código en tres capas principales:

* **Núcleo Lógico** (`src/core/`):
        * `main.c`: Contiene el bucle REPL principal y orquesta el sistema.
        * `parser.c`: Es un analizador léxico con gestión de memoria dinámica escalable. Traduce la entrada de texto en estructuras de datos (`CommandStruct`), aislando comandos, argumentos y con la posibilidad de manejar cadenas entre comillas, además de rutas.
        * `executor.c`: Aporta el control lógico. Recorre la lista de comandos y decide el flujo de ejecución evaluando códigos de estado frente a operadores (&&, ||, ;).

* **Gestión de Procesos** (`src/process/`):
        * `runner.c`: Espera indicaciones de executor.c. Interactúa con las llamadas al sistema POSIX (fork, execv, waitpid) para un comando aislado.
        * `control.c` (en desarrollo): Supervisor del entorno, encargado de gestionar señales (ej. Ctrl+C) y controlar trabajos asíncronos.

* **Herramientas** (`src/utils/`):
        * `path_resolver.c`: Localiza y valida los binarios ejecutables escaneando las variables de entorno (PATH).

## Flujo del programa

El ciclo de vida de `ucvsh` opera mediante un bucle principal que interactúa directamente con el usuario y el kernel del sistema operativo. El flujo de ejecución se divide en las siguientes etapas:

* **Lectura:** La shell despliega el prompt personalizado (`ucvsh >`) y captura la línea de texto ingresada por el usuario a través de la entrada estándar.

* **Análisis léxico y sintáctico (parsing):** La cadena de texto es enviada al módulo analizador (parser.c).

    * El parser divide la entrada en un arreglo de estructuras (`CommandStruct`) basándose en la presencia de operadores lógicos y secuenciales (`&&`, `||`, `;`).
    * Internamente, cada bloque de comando se tokeniza para separar el ejecutable de sus argumentos.
    * Se verifica la presencia del operador & al final de los argumentos para determinar si el proceso debe ejecutarse de forma asíncrona.

* **Resolución de rutas:** Previo a la ejecución de cada comando individual, la shell delega en el módulo `path_resolver` la tarea de buscar el binario correspondiente dentro de las variables de entorno del sistema operativo.

* **Control de flujo lógico:** El motor principal itera sobre el arreglo de comandos parseados. Antes de ejecutar un nodo, evalúa el código de salida del proceso inmediatamente anterior (`last_status`) y el operador que los une (`prev_op`). Basado en reglas lógicas, decide si el comando actual debe ejecutarse o ser omitido.

* **Gestión de Procesos y Ejecución:**

    * Se emplea la llamada al sistema `fork()` para bifurcar la shell y crear un proceso hijo. El proceso hijo invoca `execv()` para reemplazar su espacio de memoria con el binario resuelto.
    * El proceso padre evalúa la estructura del comando: si es un proceso en primer plano, utiliza `waitpid()` para suspender su propia ejecución hasta que el hijo termine y recolecta su código de estado. Si el comando está marcado para ejecutarse en segundo plano, el padre continúa el ciclo inmediatamente.

* **Liberación de Memoria:** Al finalizar el procesamiento de la línea, se libera dinámicamente toda la memoria asignada al arreglo de estructuras y sus argumentos para evitar fugas de memoria antes de volver a solicitar una nueva entrada.

## Compilación y ejecución

Este proyecto utiliza make para automatizar y gestionar el proceso de compilación, asegurando que todos los módulos se enlacen correctamente según la arquitectura del proyecto. 

Para compilar y ejecutar el proyecto, se emplea el siguiente comando:

```bash
    # Compila y ejecuta el código fuente a partir de las reglas del Makefile.
   make run
```

Opcionalmente, para limpiar el entorno de trabajo:

```bash
    # Elimina los archivos objeto (.o) temporales y el ejecutable generado.
   make clean
```
