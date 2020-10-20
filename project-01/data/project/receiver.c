/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "app.h"
#include "receiver.h"

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

  if ((fd = llopen(atoi(argv[1]), RECEIVER)) <= 0)
    return -1;

  unsigned char *buffer;

  for (int i = 0; i < 5; i++)
  {
    int l = llread(fd, buffer);

    // printf("BUFFER FD %p\n", buffer);

    // for (int i = 0; i < l; i++)
    // {
    //   printf("BYTE %d IS %x\n", i, buffer[i]);
    //   fflush(stdout);
    // }
    
  }
  
  if(llclose(fd) < 0)
    return -1;

  return 0;
}
