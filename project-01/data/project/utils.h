

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
#define A1 0x03
#define A2 0x01
#define SET 0x03
#define UA 0x07
#define DISC 0x0B
#define CI(n) (n << 6)
#define RR(n) (n << 7 | 0b101)
#define REJ(n) (n << 7 | 0b1)
#define ESC 0x7d
#define EFLAG 0x5e
#define EESC 0x5d

#define DATAP 1
#define STARTP 2
#define ENDP 3

#define FILE_SIZEP 0
#define FILE_NAMEP 1

#define MAX_SIZE 256
#define MAX_SIZEP MAX_SIZE - 4

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
    RR_DUP,
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
    unsigned char expected_a;
    unsigned char c;
    unsigned char expected_c;
    unsigned char bcc;
    unsigned char bcc2;
    unsigned char flag2;
    fstate state;
    int port;
    unsigned int num_retr;
    unsigned int seqnumber;
    unsigned char *buffer;
    unsigned int length;
    struct termios *oldtio;
} pframe;

/**
 * Parses the user console args
 * @param argc
 * @param argv
 * @param port to be initialized with the port number given
 * @param filename to be initialized with the filename given
 * @return port number, or -1 if an error occurred
*/
int parse_args(int argc, char **argv, int *port, char *filename);

/**
 * Shows a user message in the screen
 * @param stdout file descriptor
 * @param filename the name of the file being uploaded/downloaded
 * @param total current number of bytes uploaded/downloaded
 * @param size file size
 * @param action "Uploading" or "Downloading" expected string
 * @return 0 if success, an error otherwise
*/
int send_user_message(int stdout, char *filename, int total, int size, char *action);

/**
 * Flushes stdout pending output
 * @param printf wrapper for printf calls
 * @return 0 if success, an error otherwise
*/
int logpf(int printf);

#endif