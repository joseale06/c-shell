# ucvsh - Intérprete de Comandos (shell)

Este es un proyecto académico para la materia Sistemas Operativos de la Universidad Central de Venezuela (UCV).

## Objetivo

El objetivo principal de `ucvsh` es diseñar e implementar un intérprete de línea de comandos (shell) modular y funcional en C. Este proyecto busca profundizar en la comprensión de la interfaz del sistema operativo (POSIX) mediante la gestión directa de procesos, manipulación de la terminal a bajo nivel, resolución de rutas y evaluación de flujos de control lógico. La shell soporta comandos internos (built-ins), ejecución concurrente, procesos en segundo plano, historial persistente y la concatenación condicional/secuencial a través de operadores estándar.

## Arquitectura del sistema

El proyecto está diseñado bajo un modelo de separación estricta de responsabilidades, dividiendo el código en cuatro capas principales:

* **Núcleo Lógico** (`src/core/`):
    * `main.c`: Contiene el bucle REPL principal, evalúa el directorio de trabajo para imprimir un prompt dinámico y orquesta el ciclo de vida del sistema.
    * `parser.c`: Es un analizador léxico con gestión de memoria dinámica escalable. Traduce la entrada de texto en estructuras de datos (`CommandStruct`), aislando comandos, argumentos y con la posibilidad de manejar cadenas entre comillas y delimitadores.
    * `executor.c`: Aporta el control lógico. Recorre la lista de comandos y decide el flujo de ejecución evaluando códigos de estado frente a operadores (&&, ||, ;). Además, intercepta comandos internos e inicializa las tuberías (pipes).

* **Gestión de Procesos** (`src/process/`):
    * `runner.c`: Espera indicaciones de executor.c. Interactúa con las llamadas al sistema POSIX (fork, execv, waitpid, dup2) para procesos individuales, manejando además la duplicación de descriptores de archivo para tuberías.
    * `control.c`: Supervisor del entorno, encargado de gestionar señales del sistema operativo y controlar trabajos asíncronos.

* **Comandos Internos** (`src/builtins/`): Módulo que centraliza los comandos que deben ejecutarse dentro del propio proceso de la shell (cd, pwd, clear, exit, jobs).

* **Herramientas de Soporte** (`src/utils/`):
    * `path_resolver.c`: Localiza y valida binarios ejecutables escaneando la variable de entorno PATH usando la llamada del sistema access().
    * `terminal_manager.c`: Manipula la configuración de la terminal a bajo nivel (raw mode). Desactiva el modo canónico y el eco automático para interceptar teclas de control en tiempo real.
    * `history_manager.c`: Gestiona el historial de la sesión utilizando una estructura circular en memoria y lo sincroniza persistentemente en disco (~/.ucvsh_history), permitiendo la navegación con flechas direccionales.

## Flujo del programa

El ciclo de vida de `ucvsh` opera mediante un bucle principal que interactúa directamente con el usuario y el kernel del sistema operativo. El flujo de ejecución se divide en las siguientes etapas:

* **Lectura e interfaz:** La shell es configurada en 'raw mode', despliega el prompt personalizado (`ucvsh >`) y captura las pulsaciones del usuario byte a byte.

* **Análisis léxico y sintáctico (parsing):** Una vez confirmada la línea de entrada, es enviada al módulo analizador (parser.c).
    * El parser divide la entrada en un arreglo de estructuras (`CommandStruct`) basándose en la presencia de operadores lógicos, secuenciales y tuberías (`&&`, `||`, `;`, `|`).
    * Internamente, cada bloque de comando se tokeniza para separar el ejecutable de sus argumentos.
    * Se verifica la presencia del operador & al final de los argumentos para determinar si el proceso debe ejecutarse de forma asíncrona.

* **Orquestación y control de flujo:** El módulo executor.c itera sobre el arreglo parseado.
    * Intercepta los comandos nativos (built-ins) y los ejecuta inmediatamente en el proceso principal.
    * Para comandos externos, evalúa el operador previo y el código de retorno (last_status). Basado en reglas lógicas, decide si saltar la ejecución o proceder.
    * Si el comando involucra una tubería (|), el orquestador crea los descriptores de archivo con pipe() y los prepara para el proceso hijo.

* **Resolución de rutas:** Previo a la ejecución de cada comando individual, la shell delega en el módulo `path_resolver` la tarea de buscar el binario correspondiente dentro de las variables de entorno del sistema operativo. 

* **Gestión de Procesos y Ejecución:**
    * Se emplea la llamada al sistema `fork()` para bifurcar la shell y crear un proceso hijo. El proceso hijo invoca `execv()` para reemplazar su espacio de memoria con el binario resuelto.
    * El proceso padre evalúa la estructura del comando: si es un proceso en primer plano, utiliza `waitpid()` para suspender su propia ejecución hasta que el hijo termine y recolecta su código de estado. Si el comando está marcado para ejecutarse en segundo plano, el padre continúa el ciclo inmediatamente.

* **Liberación de Memoria:** Al finalizar, se libera dinámicamente toda la memoria asignada al arreglo de estructuras, argumentos y rutas para evitar fugas de memoria antes de volver a solicitar una nueva entrada y continuar el ciclo REPL.

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
