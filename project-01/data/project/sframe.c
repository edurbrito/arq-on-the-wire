#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "app.h"

sframe *sframe_init_stm(int port, user u)
{
    sframe *t = malloc(sizeof(sframe));
    t->state = START;
    t->u = u;
    t->port = port;
    t->num_retr = MAX_RETR;
    if (t->seqnumber == 0)
        t->seqnumber = !t->seqnumber;
    else
        t->seqnumber = 0;
    return t;
}

fstate sframe_startState(unsigned char input, sframe *t)
{
    if (input == FLAG)
    {
        t->flag1 = input;
        return FLAG_RCV;
    }
    return START;
}

fstate sframe_flagState(unsigned char input, sframe *t)
{
    if (input == A1)
    {
        t->a = input;
        return A_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate sframe_aState(unsigned char input, sframe *t)
{
    if (input == t->expected_c) // SET || UA
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate sframe_cState(unsigned char input, sframe *t)
{
    if (input == (t->a ^ t->c))
    {
        t->bcc = input;
        return BCC_OK;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate sframe_bccState(unsigned char input, sframe *t)
{
    if (input == FLAG)
    {
        t->flag2 = input;
        return STOP;
    }
    return START;
}

fstate sframe_getState(unsigned char input, sframe *t)
{
    switch (t->state)
    {
    case START:
        return sframe_startState(input, t);
    case FLAG_RCV:
        return sframe_flagState(input, t);
    case A_RCV:
        return sframe_aState(input, t);
    case C_RCV:
        return sframe_cState(input, t);
    case BCC_OK:
        return sframe_bccState(input, t);
    case STOP:
        return STOP;
    default:
        return sframe_startState(input, t);
    }
}

int send_sframe(int fd, user u)
{
    unsigned char C = u == SENDER ? SET : UA;
    unsigned char a[5] = {FLAG, A1, C, A1 ^ C, FLAG};
    int res = write(fd, a, 5);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }
    printf("Sended message to port.\n");

    return 0;
}

int send_iframe(int fd, int ns, char *buffer, int length)
{
    unsigned char C = CI(ns);
    unsigned char BCC2 = 0;
    int total = 6 + length;
    unsigned char a[total];
    a[0] = FLAG;
    a[1] = A1;
    a[2] = C;
    a[3] = A1 ^ C;

    size_t i;
    for (i = 0; i < length; i++)
    {
        a[4 + i] = buffer[i];
        BCC2 ^= buffer[i];
    }

    a[4 + i] = BCC2;
    a[4 + i + 1] = FLAG;

    int res = write(fd, a, total);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }

    printf("Sended iframe to port.\n");

    t->buffer = realloc(t->buffer, total * sizeof(char));
    strncpy(t->buffer, a, total);
    t->length = total;

    return total;
}

void alarmHandler(int signum)
{
    if (signum == SIGALRM)
    {
        if (t->num_retr > 0 && t->state != STOP)
        {
            if (send_sframe(t->port, t->u) == -1)
                exit(-1);
            alarm(TIMEOUT);
            t->num_retr--;
        }
        else if (t->num_retr <= 0 && t->state != STOP)
        {
            printf("No answer received. Ending port connection.\n");
            exit(-1);
        }
    }
}

void alarmHandler2(int signum)
{
    if (signum == SIGALRM)
    {
        if (t->num_retr > 0 && t->state != STOP)
        {
            if (send_iframe(t->port, t->seqnumber, t->buffer, t->length) == -1)
                exit(-1);
            alarm(TIMEOUT);
            t->num_retr--;
        }
        else if (t->num_retr <= 0 && t->state != STOP)
        {
            printf("No answer received. Ending port connection.\n");
            exit(-1);
        }
    }
}
