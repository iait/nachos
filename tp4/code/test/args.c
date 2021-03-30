/* args.c
 *      Programa simple que imprime los argumentos recibidos en main en
 *      la consola, con el nombre del programa inclusive.
 */

#include "syscall.h"

int
StringLength(char *s)
{
    int i;
    for (i = 0; s[i] != '\0'; i++);
    return i;
}

void
PrintString(char *s)
{
    int len = StringLength(s);
    Write(s, len, ConsoleOutput);
}

void
PrintChar(char c)
{
    Write(&c, 1, ConsoleOutput);
}

int
main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < argc; i++) {
        if (i != 0) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');
}
