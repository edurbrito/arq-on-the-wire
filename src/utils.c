
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

int parse_args(int argc, char **argv, int *port, char *filename)
{
    char *p = NULL;
    int index, c;

    opterr = 0;

    while ((c = getopt(argc, argv, "p:")) != -1)
        switch (c)
        {
        case 'p':
            p = optarg;
            break;
        case '?':
            if (optopt == 'p')
                fprintf(stderr, "APP ##### Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "APP ##### Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr,
                        "APP ##### Unknown option character `\\x%x'.\n",
                        optopt);
            fflush(stdout);
            return -1;
        default:
            return -1;
        }

    if (p != NULL)
        *port = atoi(p);
    else
        return -1;

    if (filename != NULL)
        strcpy(filename, argv[optind]);

    return atoi(p);
}

int send_user_message(int stdout, char *filename, int total, int size, char *action)
{
    char str[256];
    float percentage = (float)(1.0 * total / size) * 100.0;
    int l = sprintf(str, "\r%s %s    |    Please Wait... %.0f%%", action, filename, percentage);

    if (total == size)
    {
        l = sprintf(str, "\r%s %s    |    Complete %.0f%%      \n", action, filename, percentage);
    }
    write(stdout, str, l);

    return 0;
}

int logpf(int printf)
{
    fflush(stdout);
    return 0;
}

int prob(double p){
    double prob = (double) rand() / RAND_MAX;
    if(prob < p) return 1;
    return 0;
}