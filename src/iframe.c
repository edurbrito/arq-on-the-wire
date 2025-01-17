#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "iframe.h"

pframe *iframe_init_stm(int port, user u, pframe *t)
{
    if (t == NULL)
    {
        t = malloc(sizeof(pframe));
        t->seqnumber = 0;
    }
    if (t->buffer != NULL)
        free(t->buffer);
    t->buffer = malloc((MAX_SIZE + 1) * 2 * sizeof(unsigned char)); // Double for all-ESC case and + 1 for BCC2
    t->length = 0;
    t->bcc2 = 0;

    t->state = START;
    t->u = u;
    t->port = port;
    t->num_retr = MAX_RETR;
    return t;
}

fstate iframe_startState(unsigned char input, pframe *t)
{
    t->length = 0;
    if (input == FLAG)
    {
        t->flag1 = input;
        return FLAG_RCV;
    }
    return START;
}

fstate iframe_flagState(unsigned char input, pframe *t)
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

fstate iframe_aState(unsigned char input, pframe *t)
{
    if (input == t->expected_c)
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == CI(!t->seqnumber))
        return RR_DUP;
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

fstate iframe_cState(unsigned char input, pframe *t)
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

fstate iframe_dataState(unsigned char input, pframe *t)
{
    if (input == ESC)
    {
        return ESC_RCV;
    }
    else if (input == FLAG)
    {
        unsigned char bcc2 = t->buffer[t->length - 1];
        t->bcc2 ^= bcc2; // Reverting the last XOR that was bcc2 itself
        t->flag2 = input;
        t->length--;

        if (bcc2 != t->bcc2)
        {
            t->bcc2 = 0;
            return BCC2_REJ;
        }

        return STOP;
    }

    t->buffer[t->length] = input;
    t->bcc2 ^= input;
    t->length++;

    return DATA_RCV;
}

fstate iframe_escState(unsigned char input, pframe *t)
{
    if (input == EFLAG)
    {
        t->buffer[t->length] = FLAG;
        t->bcc2 ^= FLAG;
    }
    else if (input == EESC)
    {
        t->buffer[t->length] = ESC;
        t->bcc2 ^= ESC;
    }
    t->length++;
    return DATA_RCV;
}

fstate iframe_getState(unsigned char input, pframe *t)
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
    case BCC1_OK:
        return iframe_dataState(input, t);
    case DATA_RCV:
        return iframe_dataState(input, t);
    case ESC_RCV:
        return iframe_escState(input, t);
    case STOP:
        return STOP;
    default:
        return iframe_startState(input, t);
    }
}
