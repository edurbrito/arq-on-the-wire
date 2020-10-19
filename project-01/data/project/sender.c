/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "app.h"
#include "sender.h"

int main(int argc, char **argv)
{
  int fd = 0;
  
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

  if(llopen(fd, SENDER) != fd){
    printf("Could not open the port.\n");
  }

  for (size_t i = 0; i < 5; i++)
  {
    unsigned char a[] = {'a', FLAG, ESC, ESC, ESC, ESC, ESC, ESC, FLAG};

    llwrite(fd, a, strlen(a));
  }
  
  llclose(fd);

  return 0;
}
