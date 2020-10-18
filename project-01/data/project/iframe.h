#include <termios.h>
#include "utils.h"

#ifndef IFRAME_H
#define IFRAME_H

typedef struct {
    user u;
    unsigned char flag1;
    unsigned char a;
    unsigned char c;
    unsigned char expected_c;
    unsigned char bcc;
    char dn[MAX_SIZE];
    unsigned char bcc2;
    unsigned char flag2;
    fstate state;
    int port;
    unsigned int seqnumber;
    volatile int num_retr;
} iframe;

iframe *iframe_init_stm(int port, user u);

fstate iframe_startState(unsigned char input, iframe *t);

fstate iframe_flagState(unsigned char input, iframe *t);

fstate iframe_aState(unsigned char input, iframe *t);

fstate iframe_cState(unsigned char input, iframe *t);

fstate iframe_bccState(unsigned char input, iframe *t);

fstate iframe_getState(unsigned char input, iframe *t);

int send_rr(int fd, unsigned char C);

#endif