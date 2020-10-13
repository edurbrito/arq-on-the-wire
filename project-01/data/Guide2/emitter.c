/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "utils.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX_RETR 3
#define TIMEOUT 3

volatile int num_retr = MAX_RETR;
int fd = 0, res = 0;
tram *t;

void send_set()
{
  char a[5] = {0x7E, 0x03, 0x03, 0x00, 0x7E};
  res = write(fd, a, 5); // Writes 5 bytes
  printf("%d bytes written \n", res);
}

void alarmHandler(int signum)
{
  if (signum == SIGALRM)
  {
    if (num_retr > 0 && t->state != STOP)
    {
      printf("ALARM SET SEND %d ##########\n", num_retr);
      send_set();
      alarm(3);
      num_retr--;
    }
    else if(num_retr <= 0 && t->state != STOP)
    {
      printf("NO ANSWER RECEIVED\n");
      exit(1);
    }
  }
}

int main(int argc, char **argv) {

  struct termios oldtio, newtio;
  char buf[255];

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
       (strcmp("/dev/ttyS11", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  signal(SIGALRM, alarmHandler);

  t = init_stm(EMITTER);

  send_set();

  alarm(3);

  while (t->state != STOP)
  {
    char a;
    res = read(fd, &a, 1);

    if (res <= 0)
    {
      perror("Error");
      break;
    }

    t->state = getState(a, t);
    printf("RES %d STATE %d  |  %x%x%x%x%x\n", res, t->state, t->flag, t->a, t->c, t->bcc, t->flag);
  }

  printf("ENDED LOOP %x%x%x%x%x\n", t->flag, t->a, t->c, t->bcc, t->flag);

  free(t);

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
