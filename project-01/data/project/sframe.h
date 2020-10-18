#include "utils.h"

#ifndef SFRAME_H
#define SFRAME_H

typedef enum // Supervision Frame State
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} sf_state;

typedef struct // Supervision Frame Struct
{
    user u;
    unsigned char flag1;
    unsigned char a;
    unsigned char c;
    unsigned char bcc;
    unsigned char flag2;
    sf_state state;
    int port;
    volatile int num_retr;
} sframe;


/**
 * Inits the Supervision State Machine
 * @param port port file descriptor
 * @param u user Type
 * @return sframe pointer to struct
*/
sframe *sf_init_stm(int port, user u);


/**
 * Start State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_startState(unsigned char input, sframe *t);


/**
 * Flag State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_flagState(unsigned char input, sframe *t);

/**
 * A State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_aState(unsigned char input, sframe *t);

/**
 * C State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_cState(unsigned char input, sframe *t);

/**
 * BCC State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_bccState(unsigned char input, sframe *t);

/**
 * Gets current STM state
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state
*/
sf_state sf_getState(unsigned char input, sframe *t);

/**
 * Sends supervision message to serial port
 * @param fd serial port file descriptor
 * @param u user type
 * @return -1 if an error occurred
*/
int send_sup(int fd, user u);

/**
 * Alarm Signal Handler
 * @param signum signal number
*/
void alarmHandler(int signum);

#endif