/* console.c
 *      Programa simple para probar la consola.
 *      Espera a que aparezca algo en la entrada estándar.
 *      Escribir en consola "Hola".
 *      Este programa replica en la consola ese saludo.
 *
 */

#include "syscall.h"

int main()
{

    // lee de la entrada estándar
    char content[5];
    int read = Read(content, 5, 0);

    // escribe en la salida estándar
    Write(content, 5, 1);



    return 0;
}
