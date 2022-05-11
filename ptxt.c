#include "ptxt.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

#if DEBUG
#include <errno.h>
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))

// not serious yet
int str_hash(const char *s)
{
    int ans = 0;
    for (int i = 0; i < strlen(s); i++)
        ans += s[i];
    return ans;
}

void ptxt_init(const char *path)
{
    FILE *f = fopen(path, "r");
    struct ptxt *p = (struct ptxt *) malloc(sizeof(struct ptxt));
    p->line = 1;  // to contain the line with EOF
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n')
            p->line++;
    }
    fclose(f);
#if DEBUG
    printf("I am %lu, my p->line is %d.\n", pthread_self(), p->line);
#endif
    p->text = (char **) malloc(sizeof(char *) * p->line);
    p->hash = (int *) malloc(sizeof(int) * p->line);

    f = fopen(path, "r");
    for (int i = 0; i < p->line; i++) {
        size_t n = 0;
        p->text[i] = NULL;
        ssize_t ret =
            getline(&p->text[i], &n, f);  // getline() auto allocates space
        p->hash[i] = str_hash(p->text[i]);
    }
    fclose(f);
    pthread_exit(p);
}

/* Compute the distance between two ptxts.
 * Only insertion and deletion are allowed.
 */
int ptxt_distance(struct ptxt *old, struct ptxt *new)
{
    int **dp = (int **) malloc(sizeof(int *) * (old->line + 1));
    for (int i = 0; i <= old->line; i++) {
        dp[i] = (int *) malloc(sizeof(int) * (new->line + 1));
        dp[i][0] = i;
    }
    for (int i = 0; i <= new->line; i++)
        dp[0][i] = i;

    for (int i = 1; i <= old->line; i++) {
        for (int j = 1; j <= new->line; j++) {
            if (old->hash[i - 1] ==
                new->hash[j - 1])  // if texts in the two line are identical
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = min(dp[i - 1][j], dp[i][j - 1]) + 1;
        }
    }
#if DEBUG
    puts("dp table:");
    for (int i = 0; i <= old->line; i++) {
        for (int j = 0; j <= new->line; j++)
            printf("%d ", dp[i][j]);
        putchar('\n');
    }
#endif
    // return dp?
    return dp[old->line][new->line];
}

void ptxt_destroy(struct ptxt *p)
{
    for (int i = 0; i < p->line; i++)
        free(p->text[i]);
    free(p->text);
    free(p->hash);
    free(p);
}
