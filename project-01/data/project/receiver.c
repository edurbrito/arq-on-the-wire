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
    printf("Usage:\t./a.o serialport filename \n\tex: ./a.o 10 /tests/pr.gif\n");
    exit(1);
  }

  int fd = 0, file = 0;

  file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);

  struct stat st;
  stat("./tests/p.gif", &st);
  off_t size = st.st_size;


  if ((fd = llopen(atoi(argv[1]), RECEIVER)) <= 0)
    return -1;


  long int total = 0;
  unsigned char *buffer = malloc(MAX_SIZE* sizeof(char));
  int w = 0;

  while (total < size)
  {
    int l = llread(fd, buffer);
    w = write(file, buffer, l);

    total += w;
  }

  free(buffer);
  close(file);
  
  if(llclose(fd) < 0)
    return -1;

  return 0;
}
