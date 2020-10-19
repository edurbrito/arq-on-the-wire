#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "iframe.h"
#include "app.h"

pframe *t;

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
    printf("Sended sframe (C=%x) to port.\n", C);

    return 0;
}

int send_iframe(int fd, int ns, char *buffer, int length)
{
    unsigned char C = CI(ns);
    unsigned char BCC2 = 0;
    unsigned int total = 0;

    unsigned char a[4] = {FLAG, A1, C, A1 ^ C};
    int res = write(fd, a, 4);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }
    total += res;

    size_t i;
    char aux[length];
    for (i = 0; i < length; i++)
    {
        BCC2 ^= buffer[i];
        aux[i] = buffer[i];

        if (buffer[i] == FLAG)
        {
            a[0] = ESC;
            a[1] = EFLAG;
            res = write(fd, a, 2);
        }
        else if (buffer[i] == ESC)
        {
            a[0] = ESC;
            a[1] = EESC;
            res = write(fd, a, 2);
        }
        else
        {
            res = write(fd, &buffer[i], 1);
        }

        if (res <= 0)
        {
            printf("Could not write to serial port.\n");
            perror("Error: ");
            return -1;
        }
        total += res;
    }

    a[0] = BCC2;
    a[1] = FLAG;
    res = write(fd, a, 2);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }
    total += res;

    printf("Sended iframe (Ns=%d, C=%x) to port.\n", t->seqnumber, C);

    if (t->buffer != NULL)
        free(t->buffer);
    t->buffer = malloc(length * sizeof(char));
    strncpy(t->buffer, aux, length);
    t->length = length;

    return total;
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

    newtio.c_cc[VTIME] = SVTIME; /* inter-character timer unused */
    newtio.c_cc[VMIN] = SVMIN; /* blocking read until 5 chars received */

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
        unsigned char C = t->u == SENDER ? SET : UA;
        if (send_sframe(t->port, C) == -1)
            return -1;
    }
    else if (t->u == RECEIVER)
        t->expected_c = SET;

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            printf("Could not read from serial port.\n");
            perror("Error: ");
            return -1;
        }
        else if (res == 0 && t->u == SENDER)
        {
            if (t->num_retr > 0)
            {
                if (send_sframe(t->port, SET) == -1)
                    return -1;
                t->num_retr--;
            }
            else if (t->num_retr <= 0)
            {
                printf("No answer received. Ending port connection.\n");
                return -1;
            }
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

    t->expected_c = RR(!t->seqnumber);
    int total = send_iframe(port, t->seqnumber, buffer, length);

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            printf("Could not read from serial port.\n");
            perror("Error: ");
            return -1;
        }
        else if (res == 0 && t->u == SENDER)
        {
            if (t->num_retr > 0)
            {
                if (send_iframe(t->port, t->seqnumber, t->buffer, t->length) == -1)
                    return -1;
                t->num_retr--;
            }
            else if (t->num_retr <= 0)
            {
                printf("No answer received. Ending port connection.\n");
                return -1;
            }
        }

        t->state = sframe_getState(input, t);
        if (t->state == BCC2_REJ)
        {
            // printf("BCC2 REJECTED\n");
            total = send_iframe(port, t->seqnumber, t->buffer, t->length);
        }
    }

    printf("RR (Nr=%d, C=%x) received.\n", !t->seqnumber, t->c);
    t->seqnumber = !t->seqnumber;

    return total;
}

int llread(int port, char *buffer)
{
    t = iframe_init_stm(port, RECEIVER, t);

    t->expected_c = CI(t->seqnumber);

    printf("Reading from port.\n");

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            printf("Could not read from serial port.\n");
            perror("Error: ");
            return -1;
        }

        printf("STATE %d with INPUT %x\n", t->state, input);
        t->state = iframe_getState(input, t);

        if (t->state == BCC2_REJ)
        {
            printf("BCC2 REJECTED\n");
            if (send_sframe(t->port, REJ(t->seqnumber)) == -1)
                return -1;
        }
    }

    printf("BCC2 RECEIVED SUCCESSFULLY\n");
    t->seqnumber = !t->seqnumber;
    if (send_sframe(t->port, RR(t->seqnumber)) == -1)
        return -1;

    // TODO: SAVE DATA TO BUFFER

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

    if (close(port) != 0)
    {
        perror("close: ");
        return -1;
    }

    return 0;
}