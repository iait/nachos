/* read.c
 *      Programa simple para probar la llamada a sistema Read.
 *      Intenta leer el contenido del archivo creado en el programa create.c
 *      Abre el archivo test.txt
 *      Lee el contenido del archivo
 *      Cierra el archivo
 *
 */

#include "syscall.h"

int main()
{
    // nombre del archivo
    char name[9];
    name[0] = 't';
    name[1] = 'e';
    name[2] = 's';
    name[3] = 't';
    name[4] = '.';
    name[5] = 't';
    name[6] = 'x';
    name[7] = 't';
    name[8] = '\0';

    // abre el archivo
    int fd = Open(name);

    // lee el contenido
    char content[12];
    int read = Read(content, 12, fd);

    // cierra el archivo
    Close(fd);

    return read;
}
