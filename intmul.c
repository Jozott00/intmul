#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "hexcalc.h"

#define EXIT_ERR(msg, iserrno)                                                                           \
    {                                                                                                    \
        if (iserrno == 1)                                                                                \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg " : %s\n", __FILE__, __LINE__, strerror(errno)); \
        else                                                                                             \
            (void)fprintf(stderr, "[%s:%d] ERROR: " msg "\n", __FILE__, __LINE__);                       \
        exit(EXIT_FAILURE);                                                                              \
    }

int pipeout[4][2];

size_t get_values(char **a, char **b)
{
    size_t hexlen = 0;

    char *line;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, stdin)) != -1)
    {

        if (linelen == 1)
            EXIT_ERR("Length of number was 0", 0)
        if (hexlen != 0 && linelen - 1 != hexlen)
            EXIT_ERR("Numbers of different lengths", 0)

        if (hexlen == 0)
        {
            *a = malloc(linelen);
            strcpy(*a, line);
            hexlen = linelen - 1;
            if (hexlen % 2 != 0 && hexlen != 1)
                EXIT_ERR("number is not even", 0);
        }
        else
        {
            *b = malloc(linelen);
            strcpy(*b, line);
            break;
        }
    }

    return hexlen;
}

void gen_half_strlines(size_t len, char *A, char **Ah, char **Al)
{
    int lh = len / 2;
    int ll = len - lh;

    *Ah = malloc(lh + 2);
    *Al = malloc(ll + 2);

    memcpy(*Ah, A, lh);
    memcpy(*Al, A + lh, ll);
    Ah[0][lh] = '\n';
    Ah[0][lh + 1] = '\0';
    Al[0][ll] = '\n';
    Al[0][ll + 1] = '\0';
}

void write_to_pipe(int fd, char *data)
{
    FILE *f = fdopen(fd, "w");
    if (fputs(data, f) == -1)
        EXIT_ERR("cannot write to pipe", 1);

    fflush(f);
}

void fork_and_pipe(int hexlen, char *A, char *B)
{
    char *Ah;
    char *Al;
    char *Bh;
    char *Bl;

    gen_half_strlines(hexlen, A, &Ah, &Al);
    gen_half_strlines(hexlen, B, &Bh, &Bl);

    free(A);
    free(B);

    // Ah * Bh = cid1
    // Ah * Bl = cid2
    // Al * Bh = cid3
    // Al * Bl = cid4
    pid_t cdi[4];
    int pipein[4][2];

    for (int i = 0; i < 4; i++)
    {
        pipe(pipein[i]);
        pipe(pipeout[i]);

        cdi[i] = fork();

        switch (cdi[i])
        {
        case -1:
            EXIT_ERR("Cannot fork!", 1);
            break;
        case 0:
            close(pipein[i][1]);
            close(pipeout[i][0]);

            dup2(pipein[i][0], STDIN_FILENO);
            dup2(pipeout[i][1], STDOUT_FILENO);

            close(pipein[i][0]);
            close(pipeout[i][1]);

            execlp("./intmul", "intmul", NULL);

            EXIT_ERR("Could not execute", 1);
            break;
        default:
            close(pipein[i][0]);
            // close(pipeout[i][1]);

            // if (dup2(pipeout[i][0], STDOUT_FILENO) == -1)
            //     EXIT_ERR("Could not dub out", 1);

            // close(pipeout[i][0]);

            int pfd = pipein[i][1];

            switch (i)
            {
            case 0:
                write_to_pipe(pfd, Ah);
                write_to_pipe(pfd, Bh);
                break;
            case 1:
                write_to_pipe(pfd, Ah);
                write_to_pipe(pfd, Bl);
                break;
            case 2:
                write_to_pipe(pfd, Al);
                write_to_pipe(pfd, Bh);
                break;
            default:
                write_to_pipe(pfd, Al);
                write_to_pipe(pfd, Bl);
                break;
            };
            break;
        }
    }

    free(Ah);
    free(Al);
    free(Bh);
    free(Bl);
}

void read_from_pipes(char *results[4])
{

    for (int i = 0; i < 4; i++)
    {
        FILE *out = fdopen(pipeout[i][0], "r");
        size_t lencap = 0;
        ssize_t len = 0;
        if ((len = getline(&results[i], &lencap, out)) == -1)
            EXIT_ERR("failed to read result", 1)

        results[i][len - 1] = '\0';
    }
}
int main(void)
{
    char *A;
    char *B;

    size_t hexlen = get_values(&A, &B);

    // srand(time(NULL));
    // int random = rand() % (13 + 1 - 2) + 2;
    // fprintf(stdout, "waiting %d seconds\n", random);
    // fflush(stdout);
    // sleep(random);

    if (hexlen == 1)
    {
        unsigned int a = strtol(A, NULL, 16);
        unsigned int b = strtol(B, NULL, 16);
        fprintf(stdout, "%X\n", (a * b));
        // fprintf(stderr, "%X\n", (a * b));
        fflush(stdout);
        // fflush(stderr);
        exit(EXIT_SUCCESS);
    }

    fork_and_pipe(hexlen, A, B);

    char *results[4];
    read_from_pipes(results);

    calcQuadResult(results, results[1], results[2], results[3], hexlen);

    fprintf(stdout, "%s\n", results[0]);
    fflush(stdout);

    int status,
        pid;
    while ((pid = wait(&status)) != -1)
    {
        // fprintf(stderr, "Prozess %d exited with status %d\n", pid, status);
        // fflush(stderr);
    }

    if (errno != ECHILD)
        EXIT_ERR("cannot wait!", 1)

    return 0;
}
