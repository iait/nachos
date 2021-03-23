/* printB.c
 *
 */

#include "syscall.h"

int main()
{
    char content[1];
    content[0] = 'B';

    int i;
    for (i = 0; i < 10; i++) {
        Write(content, 1, 1);
    }

    Exit(2);
}
