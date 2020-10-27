#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "iframe.h"
#include "app.h"

pframe *t;

int send_sframe(int fd, unsigned char A, unsigned char C)
{
    unsigned char a[5] = {FLAG, A, C, A ^ C, FLAG};
    int res = write(fd, a, 5);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }
    printf("Sended sframe (C=%x) to port.\n", C);

    return res;
}

int send_iframe(int fd, int ns, unsigned char *buffer, int length)
{
    unsigned char C = CI(ns);
    unsigned char BCC2 = 0;

    unsigned char a[4] = {FLAG, A1, C, A1 ^ C};
    int res = write(fd, a, 4);
    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }

    size_t i;
    unsigned char aux[length];
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
    }

    if (BCC2 == FLAG)
    {
        a[0] = ESC;
        a[1] = EFLAG;
        a[2] = FLAG;
        res = write(fd, a, 3);
    }
    else if (BCC2 == ESC)
    {
        a[0] = ESC;
        a[1] = EESC;
        a[2] = FLAG;
        res = write(fd, a, 3);
    }
    else
    {
        a[0] = BCC2;
        a[1] = FLAG;
        res = write(fd, a, 2);
    }

    if (res <= 0)
    {
        printf("Could not write to serial port.\n");
        perror("Error: ");
        return -1;
    }

    printf("Sended iframe (Ns=%d, C=%x) to port.\n", t->seqnumber, C);

    if (t->buffer != NULL)
        free(t->buffer);
    t->buffer = malloc(length * sizeof(unsigned char));
    strncpy(t->buffer, aux, length);
    t->length = length;

    return length;
}

int llopen(int port, user u)
{
    char portname[12];
    sprintf(portname, "/dev/ttyS%d", port);

    int fd = open(portname, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(portname);
        return -1;
    }

    t = sframe_init_stm(fd, u, t);
    struct termios newtio;

    if (tcgetattr(fd, t->oldtio) == -1)
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
    newtio.c_cc[VMIN] = SVMIN;   /* blocking read until SVMIN chars received */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr: ");
        return -1;
    }

    printf("New termios structure set\n");

    printf("Starting port connection...\n");

    t->expected_a = A1;
    if (t->u == SENDER)
    {
        t->expected_c = UA;
        if (send_sframe(t->port, A1, SET) == -1)
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
                if (send_sframe(t->port, A1, SET) == -1)
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
        if (send_sframe(t->port, A1, UA) == -1)
            return -1;
    }
    else
    {
        printf("Answer received. Connection established.\n");
    }

    return fd;
}

int llwrite(int port, unsigned char *buffer, int length)
{

    t = sframe_init_stm(port, SENDER, t);

    t->expected_a = A1;
    t->expected_c = RR(!t->seqnumber);
    int total = send_iframe(port, t->seqnumber, buffer, length);

    if (total < 0)
    {
        printf("Aborting llwrite after send_iframe.\n");
        return -1;
    }

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
            printf("BCC2 REJECTED (Nr=%d, C=%x) received.\n", t->seqnumber, REJ(t->seqnumber));
            total = send_iframe(port, t->seqnumber, t->buffer, t->length);

            if (total < 0)
            {
                printf("Aborting llwrite after send_iframe.\n");
                return -1;
            }
        }
        else if (t->state == RR_DUP)
        {
            printf("RR DUP (Nr=%d, C=%x) received.\n", t->seqnumber, RR(t->seqnumber));
            return total;
        }
    }

    printf("RR (Nr=%d, C=%x) received.\n", !t->seqnumber, t->c);
    t->seqnumber = !t->seqnumber;

    return total;
}

int llread(int port, unsigned char *buffer)
{
    t = iframe_init_stm(port, RECEIVER, t);

    t->expected_a = A1;
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
            if (send_sframe(t->port, A1, REJ(t->seqnumber)) == -1)
                return -1;
        }
        else if (t->state == RR_DUP)
        {
            printf("RR DUPLICATED\n");
            if (send_sframe(t->port, A1, RR(t->seqnumber)) == -1)
                return -1;
        }
    }

    printf("BCC2 RECEIVED SUCCESSFULLY %x\n", t->bcc2);
    t->seqnumber = !t->seqnumber;
    if (send_sframe(t->port, A1, RR(t->seqnumber)) == -1)
        return -1;

    memcpy(buffer, t->buffer, t->i);

    return t->i;
}

int llclose(int port)
{
    t = sframe_init_stm(port, t->u, t);
    t->expected_c = DISC;

    printf("Closing port connection...\n");

    if (t->u == SENDER)
    {
        if (send_sframe(t->port, A1, DISC) == -1)
            return -1;

        t->expected_a = A2;
    }

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
        else if (res == 0)
        {
            if (t->num_retr > 0 && t->u == SENDER)
            {
                if (send_sframe(t->port, A1, DISC) == -1)
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

    if (t->u == SENDER)
    {
        if (send_sframe(t->port, A2, UA) == -1)
            return -1;

        printf("Frame DISC received from RECEIVER. Sent last Acknowledgment.\n");
    }
    else
    {
        t = sframe_init_stm(port, t->u, t);
        t->expected_c = UA;
        t->expected_a = A2;

        if (send_sframe(t->port, A2, DISC) == -1)
            return -1;

        printf("Frame DISC received from SENDER. Sent frame DISC.\n");

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
            else if (res == 0)
            {
                if (t->num_retr > 0)
                {
                    if (send_sframe(t->port, A2, DISC) == -1)
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

        printf("Frame UA received from SENDER. Closing port connection.\n");
    }

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