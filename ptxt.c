#include "ptxt.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

static inline int find_endl(const char *s)
{
    int ans = 0;
    while (s[ans] != '\n')
        ans++;
    return ans;
}

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
#if DEBUG
        char errbuf[100];
        strerror_r(ENOMEM, errbuf, 100);
        if (ret == -1)
            perror(errbuf);
        strerror_r(EINVAL, errbuf, 100);
        if (ret == -1)
            perror(errbuf);
#endif
        p->hash[i] = str_hash(p->text[i]);
    }
    fclose(f);
    pthread_exit(p);
}

void ptxt_destroy(struct ptxt *p)
{
    for (int i = 0; i < p->line; i++)
        free(p->text[i]);
    free(p->text);
    free(p->hash);
    free(p);
}
