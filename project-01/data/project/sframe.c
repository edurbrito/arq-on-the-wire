#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"

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
    unsigned char flag;
    unsigned char a;
    unsigned char c;
    unsigned char bcc;
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
sframe *sf_init_stm(int port, user u)
{
    sframe *t = malloc(sizeof(sframe));
    t->state = START;
    t->u = u;
    t->port = port;
    t->num_retr = MAX_RETR;
    return t;
}

/**
 * Start State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_startState(unsigned char input, sframe *t)
{
    if (input == FLAG)
    {
        t->flag = input;
        return FLAG_RCV;
    }
    return START;
}

/**
 * Flag State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_flagState(unsigned char input, sframe *t)
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

/**
 * A State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_aState(unsigned char input, sframe *t)
{
    if ((input == SET && t->u == RECEIVER) || (input == UA && t->u == SENDER)) // SET || UA
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == FLAG)
        return FLAG_RCV;
    return START;
}

/**
 * C State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_cState(unsigned char input, sframe *t)
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

/**
 * BCC State for STM
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state 
*/
sf_state sf_bccState(unsigned char input, sframe *t)
{
    if (input == FLAG)
        return STOP;
    return START;
}

/**
 * Gets current STM state
 * @param input read from port
 * @param t supervision frame struct
 * @return current sf_state
*/
sf_state sf_getState(unsigned char input, sframe *t)
{
    switch (t->state)
    {
    case START:
        return sf_startState(input, t);
    case FLAG_RCV:
        return sf_flagState(input, t);
    case A_RCV:
        return sf_aState(input, t);
    case C_RCV:
        return sf_cState(input, t);
    case BCC_OK:
        return sf_bccState(input, t);
    case STOP:
        return STOP;
    default:
        return sf_startState(input, t);
    }
}

sframe *t;

/**
 * Sends supervision message to serial port
 * @param fd serial port file descriptor
 * @param u user type
 * @return -1 if an error occurred
*/
int send_sup(int fd, user u)
{
    unsigned char C = u == SENDER ? SET : UA;
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

/**
 * Alarm Signal Handler
 * @param signum signal number
*/
void alarmHandler(int signum)
{
    if (signum == SIGALRM)
    {
        if (t->num_retr > 0 && t->state != STOP)
        {
            if(send_sup(t->port, t->u) == -1) exit(-1);
            alarm(3);
            t->num_retr--;
        }
        else if (t->num_retr <= 0 && t->state != STOP)
        {
            printf("No answer received. Ending port connection.\n");
            exit(-1);
        }
    }
}

int llopen(int port, user u)
{

    struct termios oldtio, newtio;

    if (tcgetattr(port, &oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr: ");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = SVMIN; /* inter-character timer unused */
    newtio.c_cc[VMIN] = SVTIME; /* blocking read until 5 chars received */

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
    */

    tcflush(port, TCIOFLUSH);

    if (tcsetattr(port, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr: ");
        return -1;
    }

    printf("New termios structure set\n");

    signal(SIGALRM, alarmHandler);

    t = sf_init_stm(port, u);
    printf("Starting port connection...\n");

    if (t->u == SENDER)
    {
        if(send_sup(t->port, t->u) == -1) return -1;
        alarm(3);
    }

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res <= 0)
        {
            printf("Could not read from serial port.\n");
            perror("Error: ");
            return -1;
        }

        t->state = sf_getState(input, t);
        printf("RES %d STATE %d  |  %x%x%x%x%x\n", res, t->state, t->flag, t->a, t->c, t->bcc, t->flag);
    }

    printf("ENDED LOOP %x%x%x%x%x\n", t->flag, t->a, t->c, t->bcc, t->flag);

    if (t->u == RECEIVER)
    {
        if(send_sup(t->port, t->u) == -1) return -1;
    }

    free(t);

    if (tcsetattr(port, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr: ");
        return -1;
    }

    return port;
}