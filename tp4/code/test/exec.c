/* exec.c
 *
 */

#include "syscall.h"

int main()
{

    char printAStr[14];
    printAStr[0] = '.';
    printAStr[1] = '.';
    printAStr[2] = '/';
    printAStr[3] = 't';
    printAStr[4] = 'e';
    printAStr[5] = 's';
    printAStr[6] = 't';
    printAStr[7] = '/';
    printAStr[8] = 'p';
    printAStr[9] = 'r';
    printAStr[10] = 'i';
    printAStr[11] = 'n';
    printAStr[12] = 't';
    printAStr[13] = 'A';
    printAStr[14] = '\0';

    SpaceId printAId = Exec(printAStr);

    char printBStr[14];
    printBStr[0] = '.';
    printBStr[1] = '.';
    printBStr[2] = '/';
    printBStr[3] = 't';
    printBStr[4] = 'e';
    printBStr[5] = 's';
    printBStr[6] = 't';
    printBStr[7] = '/';
    printBStr[8] = 'p';
    printBStr[9] = 'r';
    printBStr[10] = 'i';
    printBStr[11] = 'n';
    printBStr[12] = 't';
    printBStr[13] = 'B';
    printBStr[14] = '\0';

    SpaceId printBId = Exec(printBStr);

    Join(printAId);
    Join(printBId);

    char content[1];
    content[0] = '\n';
    Write(content, 1, 1);
    Halt();

    return 0;
}
