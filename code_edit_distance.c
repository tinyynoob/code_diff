#include <pthread.h>
#include <stdio.h>
#include "ptxt.h"

#define DEBUG 1

int main(int argc, char *argv[])
{
#if DEBUG
    putchar('\n');
#endif
    if (argc < 2) {
        perror("Incompatible argument number\n");
        return 1;
    }
    FILE *oldfile = fopen(argv[1], "r");
    FILE *newfile = fopen(argv[2], "r");
    pthread_t pthd[2];
    struct ptxt *old, *new;
    pthread_create(&pthd[0], NULL, ptxt_init, oldfile);
    pthread_create(&pthd[1], NULL, ptxt_init, newfile);
    pthread_join(pthd[0], &old);
    pthread_join(pthd[1], &new);
#if DEBUG
    puts("Both are joined.");
#endif
    fclose(oldfile);
    fclose(newfile);
#if DEBUG
    putchar(old->text[0][0]);
    for (int i = 0; i < old->line; i++)
        puts(old->text[i]);
#endif

    ptxt_destroy(old);
    ptxt_destroy(new);
    return 0;
}
