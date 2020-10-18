#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "sframe.h"
#include "iframe.h"
#include "app.h"

int llopen(int port, user u)
{

    struct termios newtio;

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

    t = sframe_init_stm(port, u);
    printf("Starting port connection...\n");

    if (t->u == SENDER)
    {
        t->expected_c = UA;
        signal(SIGALRM, alarmHandler);
        if (send_sframe(t->port, t->u) == -1)
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
        if (send_sframe(t->port, t->u) == -1)
            return -1;
    }
    else
    {
        printf("Answer received. Connection established.\n");
        alarm(0);
    }

    return port;
}

int llwrite(int port, char * buffer, int length)
{

    t = sframe_init_stm(port, SENDER);
    signal(SIGALRM, alarmHandler2);

    int chunk_size = MAX_SIZE - 1 >= length ? length : MAX_SIZE - 1 ;
    int bonus = length - chunk_size; // i.e. remainder

    for (int start = 0, end = chunk_size; start < length; start = end, end = start + chunk_size)
    {
        if (bonus)
        {
            end++;
            bonus--;
        }

        char chunk[end - start];

        for (int i = start; i < end; i++)
        {
            chunk[i - start] = buffer[i];
        }
        
        t->expected_c = RR(!t->seqnumber);
        send_iframe(port, t->seqnumber, chunk, length);
        alarm(TIMEOUT);

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

        printf("RR with sequence number %d received.\n", t->seqnumber);
        t->seqnumber = !t->seqnumber;
        alarm(0);
    }
}

int llread(int port, char *buffer)
{
    iframe *t;

    t = iframe_init_stm(port, RECEIVER);
    t->expected_c = CI(t->seqnumber);
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

        printf("CHAR AT %d is %c\n", i, input);
        i++;

        if(i == 9) break;

        // t->state = iframe_getState(input, t);
    }

    t->seqnumber = !t->seqnumber;
    if (send_rr(t->port, RR(t->seqnumber)) == -1)
        return -1;

    return 9;
}

int llclose(int port)
{

    if (tcsetattr(port, TCSANOW, &oldtio) != 0)
    {
        perror("tcsetattr: ");
        return -1;
    }

    free(t);

    if (close(port) != 0)
    {
        perror("close: ");
        return -1;
    }

    return 0;
}