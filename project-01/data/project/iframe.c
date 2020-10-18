#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "iframe.h"
#include "app.h"

iframe *iframe_init_stm(int port, user u)
{
    iframe *t = malloc(sizeof(iframe));
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

fstate iframe_startState(unsigned char input, iframe *t)
{
    if (input == FLAG)
    {
        t->flag1 = input;
        return FLAG_RCV;
    }
    return START;
}

fstate iframe_flagState(unsigned char input, iframe *t)
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

fstate iframe_aState(unsigned char input, iframe *t)
{
    if (input == t->expected_c)
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate iframe_cState(unsigned char input, iframe *t)
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

fstate iframe_bccState(unsigned char input, iframe *t)
{
    if (input == FLAG)
    {
        t->flag2 = input;
        return STOP;
    }
    return START;
}

fstate iframe_getState(unsigned char input, iframe *t)
{
    switch (t->state)
    {
    case START:
        return iframe_startState(input, t);
    case FLAG_RCV:
        return iframe_flagState(input, t);
    case A_RCV:
        return iframe_aState(input, t);
    case C_RCV:
        return iframe_cState(input, t);
    case BCC_OK:
        return iframe_bccState(input, t);
    case STOP:
        return STOP;
    default:
        return iframe_startState(input, t);
    }
}

int send_rr(int fd, unsigned char C)
{
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
