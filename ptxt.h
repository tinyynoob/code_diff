#include <stdio.h>  // FILE

struct ptxt {  // plain text
    int line;
    char **text;
    int *hash;  // hash each line
};

void ptxt_init(FILE *f);
void ptxt_destroy(struct ptxt *p);
int str_hash(const char *s);
