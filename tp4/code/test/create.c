/* create.c
 *      Programa simple para probar la llamadas a sistema Create, Open, Write y Close.
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

    // crea el archivo
    Create(name);

    // abre el archivo
    int fd = Open(name);

    // contenido a grabar
    char content[12];
    content[0] = 'H';
    content[1] = 'o';
    content[2] = 'l';
    content[3] = 'a';
    content[4] = ' ';
    content[5] = 'M';
    content[6] = 'u';
    content[7] = 'n';
    content[8] = 'd';
    content[9] = 'o';
    content[10] = '!';
    content[11] = '\0';

    // escribe en el archivo
    Write(content, 12, fd);

    // cierra el archivo
    Close(fd);

    return 0;
}
