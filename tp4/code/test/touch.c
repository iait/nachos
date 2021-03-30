/* touch.c
 *      Crea un archivo con el nombre proporcionado.
 *
 */

#include "syscall.h"

#define ERROR "Error: se debe proporcionar un argumento con el nombre del archivo a crear."

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        Write(ERROR, sizeof(ERROR) - 1, ConsoleOutput);
        Exit(1);
    }

    Create(argv[1]);

    Exit(0);
}
