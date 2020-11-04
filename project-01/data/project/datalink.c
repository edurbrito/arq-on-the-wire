#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "iframe.h"
#include "datalink.h"

pframe *t;

int send_sframe(int fd, unsigned char A, unsigned char C)
{
    unsigned char a[5] = {FLAG, A, C, A ^ C, FLAG};
    int res = write(fd, a, 5);
    if (res <= 0)
    {
        logpf(printf("DTL ##### Could not write to serial port.\n"));
        perror("Error: ");
        return -1;
    }
    logpf(printf("DTL ##### SFRAME (C=%x, BCC1=%x) \tSENT\n", C, A ^ C));

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
        logpf(printf("DTL ##### Could not write to serial port.\n"));
        perror("Error: ");
        return -1;
    }

    size_t i;
    for (i = 0; i < length; i++)
    {
        BCC2 ^= buffer[i];

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
            logpf(printf("DTL ##### Could not write to serial port.\n"));
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
        logpf(printf("DTL ##### Could not write to serial port.\n"));
        perror("Error: ");
        return -1;
    }

    logpf(printf("DTL ##### IFRAME (Ns=%d, C=%x, BCC2 %x) \tSENT\n", t->seqnumber, C, BCC2));

    if (t->buffer != NULL)
        free(t->buffer);
    t->buffer = malloc(length * sizeof(unsigned char));

    for (int i = 0; i < length; i++)
    {
        t->buffer[i] = buffer[i];
    }

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

    logpf(printf("DTL ##### New termios structure set\n"));
    logpf(printf("DTL ##### Starting port connection...\n"));

    t->expected_a = A1;
    if (t->u == SENDER)
    {
        if (send_sframe(t->port, A1, SET) == -1) // SENDER sends SET message to RECEIVER
            return -1;

        t->expected_c = UA; // SENDER expects UA message from RECEIVER
    }
    else if (t->u == RECEIVER)
        t->expected_c = SET; // RECEIVER expects SET message from SENDER

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            logpf(printf("DTL ##### Could not read from serial port.\n"));
            perror("Error: ");
            return -1;
        }
        else if (res == 0 && t->u == SENDER)
        {
            if (t->num_retr > 0)
            {
                if (send_sframe(t->port, A1, SET) == -1) // SENDER sends SET message to RECEIVER again
                    return -1;
                t->num_retr--;
            }
            else if (t->num_retr <= 0)
            {
                logpf(printf("DTL ##### No answer received. Ending port connection.\n"));
                return -1;
            }
        }
        else
        {
            t->num_retr = MAX_RETR;
        }

        t->state = sframe_getState(input, t);
    }

    logpf(printf("DTL ##### SFRAME (C=%x, BCC1=%x) \tRECEIVED\n", t->c, t->bcc));
    if (t->u == RECEIVER)
    {
        if (send_sframe(t->port, A1, UA) == -1) // RECEIVER sends UA message to SENDER
            return -1;
    }

    return fd;
}

int llwrite(int port, unsigned char *buffer, int length)
{

    t = sframe_init_stm(port, SENDER, t);

    int total = send_iframe(port, t->seqnumber, buffer, length); // SENDER sends IFRAME to RECEIVER
    t->expected_a = A1;
    t->expected_c = RR(!t->seqnumber); // SENDER expects RR message from RECEIVER

    // DUP TEST
    // ############################
    // if (rand() % 15 == 0)
    // {
    //     printf("TST ##### SENDING TWO FRAMES REPEATED\n");
    //     total = send_iframe(port, t->seqnumber, buffer, length);
    // }

    if (total < 0)
    {
        logpf(printf("DTL ##### Aborting llwrite after send_iframe.\n"));
        return -1;
    }

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            logpf(printf("DTL ##### Could not read from serial port.\n"));
            perror("Error: ");
            return -1;
        }
        else if (res == 0)
        {
            if (t->num_retr > 0)
            {
                total = send_iframe(port, t->seqnumber, buffer, length); // SENDER sends IFRAME to RECEIVER again

                if (total < 0)
                {
                    logpf(printf("DTL ##### Aborting llwrite after send_iframe.\n"));
                    return -1;
                }

                t->num_retr--;
            }
            else if (t->num_retr <= 0)
            {
                logpf(printf("DTL ##### No answer received. Ending port connection.\n"));
                return -1;
            }
        }
        else
        {
            t->num_retr = MAX_RETR;
        }

        t->state = sframe_getState(input, t);

        if (t->state == BCC2_REJ)
        {
            logpf(printf("DTL ##### BCC2 REJECTED (Nr=%d, C=%x) \tRECEIVED\n", t->seqnumber, REJ(t->seqnumber)));
            total = send_iframe(port, t->seqnumber, buffer, length); // SENDER sends IFRAME to RECEIVER again

            if (total < 0)
            {
                logpf(printf("DTL ##### Aborting llwrite after send_iframe.\n"));
                return -1;
            }

            t->num_retr--;
            if (t->num_retr <= 0)
            {
                logpf(printf("DTL ##### No correct answer received after many BCC2_REJ. Ending port connection.\n"));
                return -1;
            }
        }
        else if (t->state == RR_DUP)
        {
            logpf(printf("DTL ##### DUP RR (Nr=%d, C=%x) \tRECEIVED\n", t->seqnumber, RR(t->seqnumber)));
            t->state = START;
        }
    }

    logpf(printf("DTL ##### RR (Nr=%d, C=%x, BCC1=%x) \tRECEIVED\n", !t->seqnumber, t->c, t->bcc));
    t->seqnumber = !t->seqnumber;

    return total;
}

// TEST
#include <time.h>

// INTERRUPTION TEST
// ############################
// void sighandler(int signal)
// {
//     logpf(printf("TST ##### SIGNAL RECEIVED\n"));
//     sleep(3 + rand() % 3);
// }
// int cccc = 0;

int llread(int port, unsigned char *buffer)
{
    // TEST
    srand(12);
    int ch = 0;
    t = iframe_init_stm(port, RECEIVER, t);

    t->expected_a = A1;
    t->expected_c = CI(t->seqnumber); // RECEIVER expects IFRAME from SENDER

    logpf(printf("DTL ##### Reading from port.\n"));

    // DUP TEST
    // ############################
    // if ((rand() % 2) == 0)
    // {
    //     printf("TST ##### WAITING A LITTLE LONGER\n");
    //     sleep(5);
    // }

    // INTERRUPTION TEST
    // ############################
    // signal(SIGUSR1, sighandler);

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            logpf(printf("DTL ##### Could not read from serial port.\n"));
            perror("Error: ");
            return -1;
        }

        // REJ TEST
        // ############################
        // if (input == 0xeb && ((rand() % 5) == 0) && !ch)
        // {
        //     input = 0xea;
        //     printf("TST ##### CHANGING CHAR\n");
        //     ch = 1;
        // }

        t->state = iframe_getState(input, t);

        if (t->state == BCC2_REJ)
        {
            logpf(printf("DTL ##### BCC2 REJECTED (C=%x) \tSENT\n", REJ(t->seqnumber)));
            if (send_sframe(t->port, A1, REJ(t->seqnumber)) == -1) // RECEIVER sends REJ message to SENDER
                return -1;
            t->state = START;
        }
        else if (t->state == RR_DUP)
        {
            logpf(printf("DTL ##### DUP RR (C=%x) \tSENT\n", RR(t->seqnumber)));
            if (send_sframe(t->port, A1, RR(t->seqnumber)) == -1) // RECEIVER sends DUP message to SENDER
                return -1;
            t->state = START;
        }
    }

    logpf(printf("DTL ##### BCC2 SUCCESSFULLY (C=%2x, BCC2=%x) RECEIVED\n", t->c, t->bcc2));
    t->seqnumber = !t->seqnumber;
    
    // if(t->bcc2 == 0xf2 && cccc < 5){

    //     cccc++;
        
    //     memcpy(buffer, t->buffer, t->length);

    //     return t->length;
    // }

    if (send_sframe(t->port, A1, RR(t->seqnumber)) == -1) // RECEIVER sends RR message to SENDER
        return -1;

    memcpy(buffer, t->buffer, t->length);

    return t->length;
}

int llclose(int port)
{
    t = sframe_init_stm(port, t->u, t);
    t->expected_c = DISC; // SENDER/RECEIVER expects DISC message from RECEIVER/SENDER

    logpf(printf("DTL ##### Closing port connection...\n"));

    if (t->u == SENDER)
    {
        if (send_sframe(t->port, A1, DISC) == -1) // SENDER sends DISC message to RECEIVER
            return -1;

        t->expected_a = A2;
    }

    while (t->state != STOP)
    {
        unsigned char input;
        int res = read(t->port, &input, 1);

        if (res < 0)
        {
            logpf(printf("DTL ##### Could not read from serial port.\n"));
            perror("Error: ");
            return -1;
        }
        else if (res == 0)
        {
            if (t->num_retr > 0 && t->u == SENDER)
            {
                if (send_sframe(t->port, A1, DISC) == -1) // SENDER sends DISC message to RECEIVER again
                    return -1;
                t->num_retr--;
            }
            else if (t->num_retr <= 0)
            {
                logpf(printf("DTL ##### No answer received. Ending port connection.\n"));
                return -1;
            }
        }
        else
        {
            t->num_retr = MAX_RETR;
        }

        t->state = sframe_getState(input, t);
    }

    if (t->u == RECEIVER)
    {
        logpf(printf("DTL ##### DISC (C=%x, BCC1=%x) \tRECEIVED\n", t->c, t->bcc));

        t = sframe_init_stm(port, t->u, t);

        if (send_sframe(t->port, A2, DISC) == -1) // RECEIVER sends DISC message to SENDER
            return -1;

        t->expected_c = UA; // RECEIVER expects UA message from SENDER
        t->expected_a = A2;

        while (t->state != STOP)
        {
            unsigned char input;
            int res = read(t->port, &input, 1);

            if (res < 0)
            {
                logpf(printf("DTL ##### Could not read from serial port.\n"));
                perror("Error: ");
                return -1;
            }
            else if (res == 0)
            {
                if (t->num_retr > 0)
                {
                    if (send_sframe(t->port, A2, DISC) == -1) // RECEIVER sends DISC message to SENDER again
                        return -1;
                    t->num_retr--;
                }
                else if (t->num_retr <= 0)
                {
                    logpf(printf("DTL ##### No answer received. Ending port connection.\n"));
                    return -1;
                }
            }
            else
            {
                t->num_retr = MAX_RETR;
            }

            t->state = sframe_getState(input, t);
        }

        logpf(printf("DTL ##### UA (C=%x, BCC1=%x) \tRECEIVED\n", t->c, t->bcc));
    }
    else
    {
        logpf(printf("DTL ##### DISC (C=%x, BCC1=%x) \tRECEIVED\n", t->c, t->bcc));

        if (send_sframe(t->port, A2, UA) == -1) // SENDER sends UA message to RECEIVER
            return -1;
    }

    if (tcsetattr(port, TCSANOW, t->oldtio) != 0)
    {
        perror("tcsetattr: ");
        return -1;
    }

    free(t->oldtio);
    free(t);

    logpf(printf("DTL ##### Ending transmittion.\n"));

    if (close(port) != 0)
    {
        perror("close: ");
        return -1;
    }

    return 0;
}