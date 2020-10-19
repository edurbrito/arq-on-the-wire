#include <termios.h>
#include "utils.h"

#ifndef IFRAME_H
#define IFRAME_H

pframe *iframe_init_stm(int port, user u, pframe *t);

fstate iframe_startState(unsigned char input, pframe *t);

fstate iframe_flagState(unsigned char input, pframe *t);

fstate iframe_aState(unsigned char input, pframe *t);

fstate iframe_cState(unsigned char input, pframe *t);

fstate iframe_bccState(unsigned char input, pframe *t);

fstate iframe_getState(unsigned char input, pframe *t);

#endif