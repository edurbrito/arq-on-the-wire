#include <termios.h>
#include "utils.h"

#ifndef SFRAME_H
#define SFRAME_H

typedef struct // Supervision Frame Struct
{
    user u;
    unsigned char flag1;
    unsigned char a;
    unsigned char c;
    unsigned char expected_c;
    unsigned char bcc;
    unsigned char flag2;
    fstate state;
    int port;
    volatile int num_retr;
    unsigned int seqnumber;
    char * buffer;
    unsigned int length;
} sframe;


/**
 * Inits the Supervision State Machine
 * @param port port file descriptor
 * @param u user Type
 * @return sframe pointer to struct
*/
sframe *sframe_init_stm(int port, user u);


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

/**
 * Sends supervision message to serial port
 * @param fd serial port file descriptor
 * @param u user type
 * @return -1 if an error occurred
*/
int send_sframe(int fd, user u);

int send_iframe(int fd, int ns, char *buffer, int length);

/**
 * Alarm Signal Handler
 * @param signum signal number
*/
void alarmHandler(int signum);

void alarmHandler2(int signum);

sframe *t;
struct termios oldtio;

#endif