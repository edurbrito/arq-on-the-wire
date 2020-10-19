

#include <termios.h>

#ifndef UTILS_H
#define UTILS_H

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SVTIME 30
#define SVMIN 0

#define MAX_RETR 3
#define TIMEOUT 3

#define FLAG 0x7E
#define A1   0x03
#define A2   0x01
#define SET  0x03
#define UA   0x07
#define CI(n) (n << 6)
#define RR(n) (n << 7 | 0b101)
#define REJ(n) (n << 7 | 0b1)
#define ESC   0x7d
#define EFLAG 0x5e
#define EESC  0x5d

#define MAX_SIZE 50

typedef enum // User Type
{
    SENDER,
    RECEIVER,
} user; 

typedef enum // Frame States
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC1_OK,
    DATA_RCV,
    ESC_RCV,
    BCC2_REJ,
    STOP
} fstate;

typedef struct // Protocol Frame Struct
{
    user u;
    unsigned char flag1;
    unsigned char a;
    unsigned char c;
    unsigned char expected_c;
    unsigned char bcc;
    unsigned char bcc2;
    unsigned char flag2;
    fstate state;
    int port;
    volatile int num_retr;
    unsigned int seqnumber;
    char * buffer;
    unsigned int i;
    unsigned int length;
    struct termios * oldtio;
} pframe;

#endif