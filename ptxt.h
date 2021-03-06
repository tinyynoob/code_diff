#include <stdint.h>
#include <stdio.h>  // FILE

struct ptxt {  // plain text
    char *pathname;
    int line;
    char **text;     // does not contain '\n'
    uint32_t *hash;  // hash each line to a number
};

/* operations:
 * -1: end
 * 0: normal expand
 * 1: insert
 * 2: delete
 */
struct dp {
    int distance;
    int **table;
    int row;
    int column;
    short *operations;
};

uint32_t str_hash(const char *s);
void ptxt_init(const char *pathname);
struct dp *ptxt_distance(struct ptxt *old, struct ptxt *new);
void ptxt_destroy(struct ptxt *p);
void dp_generate_ops(struct dp *dp, struct ptxt *old, struct ptxt *new);
void dp_free_table(struct dp *dp);
void dp_destroy(struct dp *dp);
void print_result(struct ptxt *old, struct ptxt *new, struct dp *dp);
