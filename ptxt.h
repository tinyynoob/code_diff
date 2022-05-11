#include <stdio.h>  // FILE

struct ptxt {  // plain text
    int line;
    char **text;  // contains '\n'
    int *hash;    // hash each line
};

void ptxt_init(const char *path);
void ptxt_destroy(struct ptxt *p);
int str_hash(const char *s);
