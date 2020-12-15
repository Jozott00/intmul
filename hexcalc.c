#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "hexcalc.h"

#define EXIT_ERR(msg, iserrno)                                                                           \
    {                                                                                                    \
        if (iserrno == 1)                                                                                \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg " : %s\n", __FILE__, __LINE__, strerror(errno)); \
        else                                                                                             \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg "\n", __FILE__, __LINE__);                       \
        exit(EXIT_FAILURE);                                                                              \
    }

char addHexChar(char a, char b, char *overflow)
{
    char sa[] = {a, '\0'};
    char sb[] = {b, '\0'};

    long ia = strtol(sa, NULL, 16);
    long ib = strtol(sb, NULL, 16);
    long io = strtol(overflow, NULL, 16);
    long sum = ia + ib + io;

    //new overflow
    int no = (int)sum / 16;
    sprintf(overflow, "%x", no);

    int result = sum % 16;
    char sres[] = {' ', '\0'};
    sprintf(sres, "%x", result);

    return sres[0];
}

// result stored in a
void addHexString(char *result, char *b)
{
    size_t count = strlen(result);
    int diff = count - strlen(b);

    char overflow[] = {'0', '\0'};

    for (int i = count - 1; i >= 0; i--)
    {
        char cb = i - diff >= 0 ? b[i - diff] : '0';
        char r = addHexChar(result[i], cb, overflow);
        result[i] = r;
    }

    if (overflow[0] != '0')
    {
        if (realloc(result, count + 1) == NULL)
            EXIT_ERR("Could not reallocate memory", 1);

        // memcpy(result + 1, result, count);
        strcpy(result + 1, result);
        result[0] = overflow[0];
        result[count + 1] = '\0';
    }
}

void addZerosToHex(char **a, size_t count)
{
    size_t len = strlen(*a);
    size_t total = len + count;
    char *result = malloc(total + 1 * sizeof(char));
    memset(result, '0', total);
    memcpy(result, *a, len);
    result[total] = '\0';

    *a = result;
}

int calcQuadResult(char **hh, char *hl, char *lh, char *ll, size_t len)
{

    addZerosToHex(hh, len);
    addZerosToHex(&hl, len / 2);
    addZerosToHex(&lh, len / 2);

    addHexString(*hh, hl);
    addHexString(*hh, lh);
    addHexString(*hh, ll);

    free(hl);
    free(lh);

    size_t hhlen = strlen(*hh);
    return hhlen;
}