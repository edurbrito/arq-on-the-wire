#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "iframe.h"
#include "app.h"

sframe *t;

int send_sframe(int fd, unsigned char C)
{
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

int send_iframe(int fd, int ns, char *buffer, int length)
{
    unsigned char C = CI(ns);
    unsigned char BCC2 = 0;
    int total = 6 + length;
    unsigned char a[total];
    a[0] = FLAG;
    a[1] = A1;
    a[2] = C;
    a[3] = A1 ^ C;

    size_t i;
    char aux[length];
    for (i = 0; i < length; i++)
    {
        a[4 + i] = buffer[i];
        BCC2 ^= buffer[i];
        aux[i] = buffer[i];
    }

    a[4 + i] = BCC2;
    a[4 + i + 1] = FLAG;

    int res = write(fd, a, total);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }

    printf("Sended iframe to port.\n");

    if (t->buffer != NULL)
        free(t->buffer);
    t->buffer = malloc(length * sizeof(char));
    strncpy(t->buffer, aux, length);
    t->length = length;

    return total;
}

void alarmHandler(int signum)
{
    if (signum == SIGALRM)
    {
        if (t->num_retr > 0 && t->state != STOP)
        {
            unsigned char C = t->u == SENDER ? SET : UA;
            if (send_sframe(t->port, C) == -1)
                exit(-1);
            alarm(TIMEOUT);
            t->num_retr--;
        }
        else if (t->num_retr <= 0 && t->state != STOP)
        {
            printf("No answer received. Ending port connection.\n");
            exit(-1);
        }
    }
}

void alarmHandler2(int signum)
{
    if (signum == SIGALRM)
    {
        if (t->num_retr > 0 && t->state != STOP)
        {
            if (send_iframe(t->port, t->seqnumber, t->buffer, t->length) == -1)
                exit(-1);
            alarm(TIMEOUT);
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

    t = sframe_init_stm(port, u, t);
    struct termios newtio;

    if (tcgetattr(port, t->oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr: ");
        return -1;
    }

    bzero(&newtio, sizeof(struct termios));
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

    printf("Starting port connection...\n");

    if (t->u == SENDER)
    {
        t->expected_c = UA;
        signal(SIGALRM, alarmHandler);
        unsigned char C = t->u == SENDER ? SET : UA;
        if (send_sframe(t->port, C) == -1)
            return -1;
        alarm(TIMEOUT);
    }
    else if (t->u == RECEIVER)
        t->expected_c = SET;

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

        t->state = sframe_getState(input, t);
    }

    if (t->u == RECEIVER)
    {
        printf("Frame received. Connection accepted.\n");
        unsigned char C = t->u == SENDER ? SET : UA;
        if (send_sframe(t->port, C) == -1)
            return -1;
    }
    else
    {
        printf("Answer received. Connection established.\n");
        alarm(0);
    }

    return port;
}

int llwrite(int port, char *buffer, int length)
{

    t = sframe_init_stm(port, SENDER, t);

    signal(SIGALRM, alarmHandler2);

    t->state = START;
    t->expected_c = RR(!t->seqnumber);
    send_iframe(port, t->seqnumber, buffer, length);
    alarm(TIMEOUT);
    int i = 0;

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

        printf("CHAR AT %d is %x\n", i, input);
        i++;
        t->state = sframe_getState(input, t);
    }

    printf("RR with sequence number %d received.\n", t->seqnumber);
    t->seqnumber = !t->seqnumber;
    alarm(0);
}

int llread(int port, char *buffer)
{
    iframe *t;

    t = iframe_init_stm(port, RECEIVER, t);
    t->expected_c = CI(t->seqnumber);
    int i = 0;

    printf("Reading from port.\n");

    while (t->state != STOP)
    {
        unsigned char input;
        // printf("PORT AT %d I %d\n", t->port,i);
        // fflush(stdout);
        int res = read(t->port, &input, 1);

        if (res <= 0)
        {
            printf("Could not read from serial port.\n");
            perror("Error: ");
            return -1;
        }

        printf("CHAR AT %d is %x\n", i, input);

        if (input == FLAG)
        {
            if (i == 1)
            {
                i = 0;
                t->seqnumber = !t->seqnumber;
                if (send_sframe(t->port, RR(t->seqnumber)) == -1)
                    return -1;
            }
            i++;
        }

        // t->state = iframe_getState(input, t);
    }

    return 9;
}

int llclose(int port)
{
    if (tcsetattr(port, TCSANOW, t->oldtio) != 0)
    {
        perror("tcsetattr: ");
        return -1;
    }

    free(t->oldtio);
    free(t);

    printf("Ending transmittion.\n");

    // if (close(port) != 0)
    // {
    //     perror("close: ");
    //     return -1;
    // }

    return 0;
}