#include <termios.h>
#include "utils.h"

#ifndef SFRAME_H
#define SFRAME_H

/**
 * Inits the Supervision State Machine
 * @param port port file descriptor
 * @param u user Type
 * @param t sframe struct to be initialized
 * @return sframe pointer to struct
*/
sframe *sframe_init_stm(int port, user u, sframe * t);


/**
 * Start State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate 
*/
fstate sframe_startState(unsigned char input, sframe *t);


/**
 * Flag State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate 
*/
fstate sframe_flagState(unsigned char input, sframe *t);

/**
 * A State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate 
*/
fstate sframe_aState(unsigned char input, sframe *t);

/**
 * C State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate 
*/
fstate sframe_cState(unsigned char input, sframe *t);

/**
 * BCC State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate 
*/
fstate sframe_bccState(unsigned char input, sframe *t);

/**
 * Gets current STM state
 * @param input read from port
 * @param t supervision frame struct
 * @return current fstate
*/
fstate sframe_getState(unsigned char input, sframe *t);

#endif