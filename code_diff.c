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
    pthread_t pthd[2];
    struct ptxt *old, *new;
    pthread_create(&pthd[0], NULL, ptxt_init, argv[1]);
    pthread_create(&pthd[1], NULL, ptxt_init, argv[2]);
    pthread_join(pthd[0], &old);
    pthread_join(pthd[1], &new);
#if DEBUG
    puts("Both are joined.");
#endif
#if DEBUG
    puts("old->hash:");
    for (int i = 0; i < old->line; i++)
        printf("%u\n", old->hash[i]);
    puts("new->hash:");
    for (int i = 0; i < new->line; i++)
        printf("%u\n", new->hash[i]);
#endif
    struct dp *dp = ptxt_distance(old, new);
    dp_generate_ops(dp, old, new);
    dp_free_table(dp);  // since the ops are generated, the table is useless.
#if DEBUG
    puts("operations:");
    for (int i = 0; dp->operations[i] != -1; i++)
        printf("%hd ", dp->operations[i]);
    putchar('\n');
#endif
    print_result(old, new, dp);
    dp_destroy(dp);
    dp = NULL;
    ptxt_destroy(old);
    old = NULL;
    ptxt_destroy(new);
    new = NULL;
    return 0;
}
