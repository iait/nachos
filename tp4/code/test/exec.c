/* exec.c
 *      Simple programa para la prueba de Exec y Join.
 *      Ejecuta concurrentemente los programas printA y printB.
 *      Si se corre con la opción -rs se puede ver como se intercalan las letras.
 */

#include "syscall.h"

int main()
{

    SpaceId printAId = Exec("../test/printA");
    SpaceId printBId = Exec("../test/printB");

    Join(printAId);
    Join(printBId);

    Write("\n", 1, ConsoleOutput);
    Halt();

    return 0;
}
