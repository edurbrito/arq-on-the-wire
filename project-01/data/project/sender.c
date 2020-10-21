/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
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

  int fd = 0, file = 0;

  file = open(argv[2], O_RDONLY);

  struct stat st;
  stat(argv[2], &st);
  off_t size = st.st_size;

  if ((fd = llopen(atoi(argv[1]), SENDER)) <= 0)
    return -1;

  long int total = 0;

  while (total < size)
  {
    unsigned char buffer[MAX_SIZE];
    int l = MAX_SIZE > (size - total) ? size - total : MAX_SIZE;
    read(file, buffer, l);
    total += llwrite(fd, buffer, l);
  }
  
  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
