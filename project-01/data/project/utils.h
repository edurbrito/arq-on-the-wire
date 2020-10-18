
#ifndef UTILS_H
#define UTILS_H

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SVTIME 5
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

#define MAX_SIZE 3

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
    BCC_OK,
    STOP
} fstate;

#endif