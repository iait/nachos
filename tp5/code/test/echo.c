/* echo.c
 *      Imprime una línea de texto en consola.
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
    for (i = 1; i < argc; i++) {
        if (i != 0) {
            PrintChar(' ');
        }
        PrintString(argv[i]);
    }
    PrintChar('\n');

    Exit(0);
}
