#include "ptxt.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strlen

#define DEBUG 0

#if DEBUG
#include <errno.h>
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))

static inline void replace_endl(char *s)
{
    int i = 0;
    while (s[i] != '\n' && s[i] != '\0')
        i++;
    s[i] = '\0';
}

// may be choosed better
uint32_t str_hash(const char *s)
{
    uint32_t ans = 0;
    int len = strlen(s);
    for (int i = 0; i < (len >> 2); i++) {
        ans += ((uint32_t) s[i]) << 24 | (~(uint32_t) s[i + 1] << 16) |
               (~(uint32_t) s[i + 2] << 8) | ((uint32_t) s[i + 3]);
    }
    for (int i = len & ~0x3u; i < len; i++)
        ans -= s[i] << (7 * i);
    ans ^= (uint32_t) s[len >> 1] << 19 | (uint32_t) s[len >> 5];
    return ans;
}

void ptxt_init(const char *pathname)
{
    FILE *f = fopen(pathname, "r");
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
    p->hash = (uint32_t *) malloc(sizeof(u_int32_t) * p->line);

    f = fopen(pathname, "r");
    for (int i = 0; i < p->line; i++) {
        size_t n = 0;
        p->text[i] = NULL;
        ssize_t ret =
            getline(&p->text[i], &n, f);  // getline() auto allocates space
        replace_endl(p->text[i]);
        p->hash[i] = str_hash(p->text[i]);
    }
    fclose(f);
    pthread_exit(p);
}

/* Allocate struct dp.
 * Compute the distance between two ptxts.
 * For our metric, only insertion and deletion are allowed.
 */
struct dp *ptxt_distance(struct ptxt *old, struct ptxt *new)
{
    struct dp *dp = (struct dp *) malloc(sizeof(struct dp));
    dp->row = old->line + 1;
    dp->column = new->line + 1;
    dp->operations = NULL;
    dp->table = (int **) malloc(sizeof(int *) * (old->line + 1));
    for (int i = 0; i <= old->line; i++) {
        dp->table[i] = (int *) malloc(sizeof(int) * (new->line + 1));
        dp->table[i][0] = i;
    }
    for (int i = 0; i <= new->line; i++)
        dp->table[0][i] = i;

    for (int i = 1; i <= old->line; i++) {
        for (int j = 1; j <= new->line; j++) {
            if (old->hash[i - 1] ==
                new->hash[j - 1])  // if texts in the two line are identical
                dp->table[i][j] = dp->table[i - 1][j - 1];
            else
                dp->table[i][j] =
                    min(dp->table[i - 1][j], dp->table[i][j - 1]) + 1;
        }
    }
#if DEBUG
    puts("dp table:");
    for (int i = 0; i <= old->line; i++) {
        for (int j = 0; j <= new->line; j++)
            printf("%d ", dp->table[i][j]);
        putchar('\n');
    }
#endif
    dp->distance = dp->table[old->line][new->line];
    return dp;
}

/* operations:
 * -1: end
 * 0: normal expand
 * 1: insert
 * 2: delete
 */
void dp_generate_ops(struct dp *dp, struct ptxt *old, struct ptxt *new)
{
    dp->operations = (short *) malloc(sizeof(short) * (dp->row + dp->column));
    int index = dp->row + dp->column - 1;
    int r = dp->row - 1, c = dp->column - 1;
    while (r | c) {
        if (r && c) {
            if (old->hash[r - 1] == new->hash[c - 1]) {  // note this!
                dp->operations[index--] = 0;
                r--;
                c--;
            } else if (dp->table[r][c - 1] <= dp->table[r - 1][c]) {
                dp->operations[index--] = 1;
                c--;
            } else {
                dp->operations[index--] = 2;
                r--;
            }
        } else if (c) {
            dp->operations[index--] = 1;
            c--;
        } else if (r) {
            dp->operations[index--] = 2;
            r--;
        }
    }
    index = index + 1;
    int k = 0;
    while (index < dp->row + dp->column)
        dp->operations[k++] = dp->operations[index++];
    while (k < dp->row + dp->column)
        dp->operations[k++] = -1;
}

void dp_destroy(struct dp *dp)
{
    for (int i = 0; i < dp->row; i++)
        free(dp->table[i]);
    free(dp->table);
    free(dp->operations);
    free(dp);
}

void ptxt_destroy(struct ptxt *p)
{
    for (int i = 0; i < p->line; i++)
        free(p->text[i]);
    free(p->text);
    free(p->hash);
    free(p);
}

void print_result(struct ptxt *old, struct ptxt *new, struct dp *dp)
{
    puts("\033[0m================================================================================");
    int lidx1 = 0, lidx2 = 0;   // line index
    for (int i = 0; dp->operations[i] != -1; i++) {
        if (dp->operations[i] == 0) {
            printf("\033[0m  |%s\n", old->text[lidx1++]);
            lidx2++;
        } else if (dp->operations[i] == 1) {
            printf("\033[0;32m+ |%s\n", new->text[lidx2++]);
        } else if (dp->operations[i] == 2) {
            printf("\033[0;31m- |%s\n", old->text[lidx1++]);
        }
    }
    puts("\033[0m================================================================================");
}
