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

  if ((argc < 3) ||
      ((strcmp("10", argv[1]) != 0) &&
       (strcmp("11", argv[1]) != 0)))
  {
    printf("Usage:\t./a.o serialport filename \n\tex: ./a.o 10 /images/p.gif\n");
    exit(1);
  }

  int fd = 0;

  if ((fd = llopen(atoi(argv[1]), SENDER)) <= 0)
    return -1;

  for (size_t i = 0; i < 5; i++)
  {
    unsigned char a[] = {'a', FLAG, ESC, ESC, ESC, ESC, ESC, ESC, FLAG};
    llwrite(fd, a, strlen(a));
  }

  if(llclose(fd) < 0)
    return -1;

  return 0;
}
