
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

typedef enum // User Type
{
    SENDER,
    RECEIVER,
} user; 


#endif