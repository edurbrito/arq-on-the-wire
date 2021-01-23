#include <termios.h>
#include "utils.h"

#ifndef IFRAME_H
#define IFRAME_H

/**
 * Inits the Information State Machine
 * @param port port file descriptor
 * @param u user Type
 * @param t pframe struct to be initialized
 * @return pframe pointer to struct
*/
pframe *iframe_init_stm(int port, user u, pframe *t);

/**
 * Start State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_startState(unsigned char input, pframe *t);

/**
 * Flag State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_flagState(unsigned char input, pframe *t);

/**
 * A State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_aState(unsigned char input, pframe *t);

/**
 * C State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_cState(unsigned char input, pframe *t);

/**
 * DATA State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_dataState(unsigned char input, pframe *t);

/**
 * ESC State for STM
 * @param input read from port
 * @param t frame struct
 * @return current fstate 
*/
fstate iframe_escState(unsigned char input, pframe *t);

/**
 * Gets current STM state
 * @param input read from port
 * @param t frame struct
 * @return current fstate
*/
fstate iframe_getState(unsigned char input, pframe *t);

#endif