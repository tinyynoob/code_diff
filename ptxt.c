#include "ptxt.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strdup

#define DEBUG 0
#if DEBUG
#include <errno.h>
#endif

#define MAXLEN 256  // maximum length of a single input line

#define min(a, b) ((a) < (b) ? (a) : (b))
#define strlen(s) mystrlen(s)

#define UNALIGNED32(X) ((uint32_t)(uintptr_t)(X) & (sizeof(uint32_t) - 1))
#define DETECT_NULL(X) (((X) -0x01010101u) & ~(X) &0x80808080u)

static inline size_t mystrlen(const char *s)
{
    size_t d = 0;
    while (UNALIGNED32(s + d)) {
        if (s[d])
            d++;
        else
            return d;
    }
    uint32_t word = *(uint32_t *) (s + d);
    while (!DETECT_NULL(word)) {
        d += 4;
        word = *(uint32_t *) (s + d);
    }
    while (s[d])
        d++;
    return d;
}

/* Replace the first newline character with '\0'.
 * If there is the newline character, return 1, else return 0.
 */
static inline int replace_endl(char *s)
{
    int d = 0;
    while (UNALIGNED32(s + d)) {
        if (s[d] == '\n') {
            s[d] = '\0';
            return 1;
        } else if (s[d] == '\0') {
            return 0;
        } else {
            d++;
        }
    }
    const uint32_t mask = 0x0A0A0A0Au;  // 4 newline character
    uint32_t word = *(uint32_t *) (s + d);
    while (!DETECT_NULL(word) && !DETECT_NULL(word ^ mask)) {
        d += 4;
        word = *(uint32_t *) (s + d);
    }
    while (s[d] != '\n' && s[d] != '\0')
        d++;
    int ret = s[d] == '\n';
    s[d] = '\0';
    return ret;
}

// the hash function may be choosed better
uint32_t str_hash(const char *s)
{
    uint32_t ans = 0;
    size_t len = strlen(s);
    for (size_t i = 0; i < (len >> 2); i++) {
        ans += ((uint32_t) s[4 * i] << 24) | ((uint32_t) ~s[4 * i + 2] << 16) |
               ((uint32_t) ~s[4 * i + 1] << 8) | ((uint32_t) s[4 * i + 3]);
        ans -= (uint32_t) s[4 * i + len & 3] << 21 | (ans & 0xFFu) << 5;
    }
    for (size_t i = len & ~0x3u; i < len; i++) {
        ans -= (uint32_t) s[i] << (7 * i);
        ans = ans >> 16 | ans << 16;
    }
    ans ^= (uint32_t) s[len >> 2] << 29 | (uint32_t) s[len >> 1] << 19 |
           (uint32_t) s[len >> 6];
    return ans;
}

void ptxt_init(const char *pathname)
{
    struct ptxt *p = (struct ptxt *) malloc(sizeof(struct ptxt));
    p->pathname = strdup(pathname);

    FILE *f = fopen(p->pathname, "r");
    p->line = 1;  // to contain the line with EOF
    char instr[MAXLEN]; // buffer
    while (fgets(
        instr, MAXLEN,
        f))  // determine p->line, may overestimate 1 if EOF not in a new line
        p->line++;
    fclose(f);
    p->text = (char **) malloc(sizeof(char *) * p->line);
    p->hash = (uint32_t *) malloc(sizeof(uint32_t) * p->line);

    f = fopen(p->pathname, "r");
    int i = 0, newline = 1;
    while (fgets(instr, MAXLEN, f)) {
        newline = replace_endl(instr);
        p->text[i] = strdup(instr);
        p->hash[i] = str_hash(p->text[i]);
        i++;
    }
    if (newline) {  // last line is empty, containing only EOF
        p->text[i] = strdup("");
        p->hash[i] = str_hash(p->text[i]);
    } else {  // line number was overestimated
        p->line--;
    }
    fclose(f);
#if DEBUG
    printf("I am %lu, my p->line is %d.\n", pthread_self(), p->line);
#endif
    pthread_exit((void *) p);
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

void dp_free_table(struct dp *dp)
{
    if (!dp)
        return;
    if (dp->table) {
        for (int i = 0; i < dp->row; i++)
            free(dp->table[i]);
        free(dp->table);
    }
    dp->table = NULL;
}

void dp_destroy(struct dp *dp)
{
    if (!dp)
        return;
    if (dp->table) {
        for (int i = 0; i < dp->row; i++)
            free(dp->table[i]);
        free(dp->table);
    }
    if (dp->operations)
        free(dp->operations);
    free(dp);
}

void ptxt_destroy(struct ptxt *p)
{
    if (!p)
        return;
    free(p->pathname);
    for (int i = 0; i < p->line; i++)
        free(p->text[i]);
    free(p->text);
    free(p->hash);
    free(p);
}

void print_result(struct ptxt *old, struct ptxt *new, struct dp *dp)
{
    printf("\nFrom %s to %s:\n", old->pathname, new->pathname);
    printf("The distance is %d.\n", dp->distance);
    puts(
        "\033[0m==============================================================="
        "====================");
    int lidx1 = 0, lidx2 = 0;  // line index
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
    puts(
        "\033[0m==============================================================="
        "====================");
}
