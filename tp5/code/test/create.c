/* create.c
 *      Programa simple para probar las llamadas a sistema Create, Open, Write y Close.
 *      Crea un archivo llamado test.txt
 *      Abre el archivo recién creado
 *      Escribe "Hola Mundo!" en el archivo
 *      Cierra el archivo.
 *
 */

#include "syscall.h"

int main()
{

    // nombre del archivo
    char *name = "test.txt";

    // crea el archivo
    Create(name);

    // abre el archivo
    int fd = Open(name);

    // escribe en el archivo
    Write("Hola Mundo!", 12, fd);

    // cierra el archivo
    Close(fd);

    return 0;
}
