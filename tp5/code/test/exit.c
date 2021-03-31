/* exit.c
 *      Programa simple para probar la llamada a sistema Exit(status).
 *
 */

#include "syscall.h"

int
main()
{
    Exit(4);
    /* not reached */
}
