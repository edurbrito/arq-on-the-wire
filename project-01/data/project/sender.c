/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "app.h"
#include "sender.h"

int send_ctrl_packet(int ctrl_type, int fd, long int filesize, char *filename)
{

  int numbers = 0;
  int filename_size = strlen(filename);
  long int aux = filesize;

  while (aux != 0)
  {
    aux /= 10;
    ++numbers;
  }

  int total = 5 + numbers + filename_size;

  unsigned char *buffer = malloc(total * sizeof(unsigned char));

  buffer[0] = ctrl_type;
  buffer[1] = FILE_SIZEP;
  buffer[2] = numbers;

  int i = 0;
  while (filesize != 0)
  {
    aux = filesize % 10;
    filesize /= 10;
    buffer[i + 3] = aux + '0';
    i++;
  }

  i += 3;
  buffer[i] = FILE_NAMEP;
  i++;
  buffer[i] = filename_size;
  i++;

  int j;

  for (j = 0; j < filename_size; j++)
  {
    buffer[i + j] = filename[j];
  }

  if (llwrite(fd, buffer, total) <= 0)
  {
    printf("Failed at llwrite when sending ctrl packet %d.\n", ctrl_type);
    free(buffer);
    return -1;
  }

  free(buffer);

  return total;
}

int send_data_packet(int fd, int nr, unsigned char *data, int length)
{

  int l1 = length % 256;
  int l2 = length / 256;

  int total = 4 + length;

  unsigned char *buffer = malloc(total * sizeof(unsigned char));

  buffer[0] = DATAP;
  buffer[1] = nr;
  buffer[2] = l2;
  buffer[3] = l1;

  for (int i = 0; i < length; i++)
  {
    buffer[i + 4] = data[i];
  }

  if (llwrite(fd, buffer, total) <= 0)
  {
    printf("Failed at llwrite when sending data packet nr %d.\n", nr);
    free(buffer);
    return -1;
  }

  free(buffer);

  return total;
}

int main(int argc, char **argv)
{

  if (argc < 3)
  {
    printf("Usage:\t./a.o serialport filename \n\tex: ./a.o 10 /images/p.gif\n");
    exit(1);
  }

  int fd = 0, file = 0;

  if ((fd = llopen(atoi(argv[1]), SENDER)) < 0)
  {
    printf("Failed at llopen on port %d.\n", atoi(argv[1]));
    return -1;
  }

  if ((file = open(argv[2], O_RDONLY)) < 0)
  {
    printf("Failed to open file to be sent.\n");
    return -1;
  }

  struct stat st;
  stat(argv[2], &st);
  off_t size = st.st_size;

  if(send_ctrl_packet(STARTP, fd, size, argv[2]) <= 0)
    return -1;

  long int total = 0;
  int nr = 0;

  while (total < size)
  {
    unsigned char buffer[MAX_SIZEP];
    int l = MAX_SIZEP > (size - total) ? size - total : MAX_SIZEP;
    int r = 0;
    if((r = read(file, buffer, l)) < l){
      printf("Error when reading from file to be sent.\n");
    }
    total += r;
    if(send_data_packet(fd, nr % 255, buffer, r) <= 0)
      return -1;
    nr++;
  }

  if(send_ctrl_packet(ENDP, fd, size, argv[2]) <= 0)
    return -1;

  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
