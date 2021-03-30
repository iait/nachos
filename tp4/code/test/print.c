/* print.c
 *      Recibe como argumento un caracter y lo imprime 10 veces.
 */

#include "syscall.h"

int main(int argc, char *argv[])
{

    int i;
    for (i = 0; i < 10; i++) {
        Write(argv[1], 1, ConsoleOutput);
    }

    Exit(0);
}
