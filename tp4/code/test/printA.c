/* printA.c
 *
 */

#include "syscall.h"

int main()
{
    char content[1];
    content[0] = 'A';

    int i;
    for (i = 0; i < 10; i++) {
        Write(content, 1, 1);
    }

    Exit(1);
}
