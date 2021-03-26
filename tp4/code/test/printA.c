/* printA.c
 *      Imprime 10 letras A en la salida estándar.
 */

#include "syscall.h"

int main()
{

    int i;
    for (i = 0; i < 10; i++) {
        Write("A", 1, ConsoleOutput);
    }

    Exit(1);
}
