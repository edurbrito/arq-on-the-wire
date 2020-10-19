#include <termios.h>
#include "utils.h"

#ifndef IFRAME_H
#define IFRAME_H

iframe *iframe_init_stm(int port, user u, iframe * t);

fstate iframe_startState(unsigned char input, iframe *t);

fstate iframe_flagState(unsigned char input, iframe *t);

fstate iframe_aState(unsigned char input, iframe *t);

fstate iframe_cState(unsigned char input, iframe *t);

fstate iframe_bccState(unsigned char input, iframe *t);

fstate iframe_getState(unsigned char input, iframe *t);

#endif