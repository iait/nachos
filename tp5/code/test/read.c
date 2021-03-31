/* read.c
 *      Programa simple para probar la llamada a sistema Read.
 *      Intenta leer el contenido del archivo creado en el programa create.c
 *      Abre el archivo test.txt
 *      Lee el contenido del archivo y lo escribe en consola
 *      Cierra el archivo
 *
 */

#include "syscall.h"

int main()
{

    // abre el archivo
    int fd = Open("test.txt");

    // lee el contenido
    char content[12];
    int read = Read(content, 12, fd);

    // imprime el contenido en consola
    Write(content, 12, ConsoleOutput);

    // cierra el archivo
    Close(fd);

    return read;
}
