#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "app.h"

pframe *sframe_init_stm(int port, user u, pframe *t)
{
    if (t == NULL)
    {
        t = malloc(sizeof(pframe));
        t->oldtio = malloc(sizeof(struct termios));
        t->seqnumber = 0;
    }
    t->state = START;
    t->u = u;
    t->port = port;
    t->num_retr = MAX_RETR;
    return t;
}

fstate sframe_startState(unsigned char input, pframe *t)
{
    if (input == FLAG)
    {
        t->flag1 = input;
        return FLAG_RCV;
    }
    return START;
}

fstate sframe_flagState(unsigned char input, pframe *t)
{
    if (input == t->expected_a)
    {
        t->a = input;
        return A_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate sframe_aState(unsigned char input, pframe *t)
{
    if (input == t->expected_c)
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    else if (input == REJ(t->seqnumber))
        return BCC2_REJ;
    return START;
}

fstate sframe_cState(unsigned char input, pframe *t)
{
    if (input == (t->a ^ t->c))
    {
        t->bcc = input;
        return BCC1_OK;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate sframe_bccState(unsigned char input, pframe *t)
{
    if (input == FLAG)
    {
        t->flag2 = input;
        return STOP;
    }
    return START;
}

fstate sframe_getState(unsigned char input, pframe *t)
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
    case BCC1_OK:
        return sframe_bccState(input, t);
    case STOP:
        return STOP;
    default:
        return sframe_startState(input, t);
    }
}
