/* printB.c
 *      Imprime 10 letras B en la salida estándar.
 */

#include "syscall.h"

int main()
{

    int i;
    for (i = 0; i < 10; i++) {
        Write("B", 1, ConsoleOutput);
    }

    Exit(2);
}
