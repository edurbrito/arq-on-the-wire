/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "app.h"
#include "receiver.h"

int get_ctrl_packet_filesize(unsigned char *buffer)
{
  if (buffer[1] == FILE_SIZEP)
  {
    int l1 = buffer[2];
    unsigned char filesize_s[l1];
    for (int i = 0; i < l1; i++)
    {
      filesize_s[l1 - i - 1] = buffer[i + 3];
    }

    return atoi(filesize_s);
  }

  return 0;
}

unsigned char *get_ctrl_packet_filename(unsigned char *buffer)
{
  int l1 = buffer[2];
  if (buffer[3 + l1] == FILE_NAMEP)
  {
    int l2 = buffer[4 + l1];
    unsigned char *filename = malloc((l2 + 1) * sizeof(unsigned char));
    int i;
    for (i = 0; i < l2; i++)
    {
      filename[i] = buffer[5 + l1 + i];
    }

    filename[i] = '_';

    return filename;
  }

  return NULL;
}

int main(int argc, char **argv)
{

  if (argc < 2)
  {
    printf("Usage:\t./b.o serialport \n\tex: ./a.o 11\n");
    exit(1);
  }

  int fd = 0, file = 0;

  if ((fd = llopen(atoi(argv[1]), RECEIVER)) <= 0)
  {
    printf("Failed at llopen on port %d.\n", atoi(argv[1]));
    return -1;
  }

  unsigned char *buffer = malloc(MAX_SIZE * sizeof(char));
  int l = 0;

  while (buffer[0] != STARTP)
  {
    l = llread(fd, buffer);
    if (l <= 0)
    {
      printf("Failed at llread when receiving ctrl packet start.\n");
      free(buffer);
      return -1;
    }
  }

  int filesize;

  if ((filesize = get_ctrl_packet_filesize(buffer)) <= 0)
  {
    printf("Failed at get ctrl packet filesize %d.\n", filesize);
    free(buffer);
    return -1;
  }

  unsigned char *filename;

  if ((filename = get_ctrl_packet_filename(buffer)) == NULL)
  {
    printf("Failed at get ctrl packet filename %s.\n", filename);
    return -1;
  }

  if ((file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
  {
    printf("Failed to open file to be written.\n");
    free(buffer);
    free(filename);
    return -1;
  }

  long int total = 0;
  int w = 0;

  while (total < filesize)
  {
    int l = llread(fd, buffer);
    if (l < 0)
    {
      printf("Failed at llread when receiving data packet.\n");
      free(buffer);
      free(filename);
      return -1;
    }

    unsigned char aux[l - 4];

    for (size_t i = 4; i < l; i++)
    {
      aux[i - 4] = buffer[i];
    }

    int w = 0;

    if ((w = write(file, aux, l - 4)) < l - 4)
    {
      printf("Error when writing to new file.\n");
    }

    total += w;
  }

  while (buffer[0] != ENDP)
  {
    l = llread(fd, buffer);
    if (l <= 0)
    {
      printf("Failed at llread when receiving ctrl packet end.\n");
      free(buffer);
      free(filename);
      return -1;
    }
  }

  free(buffer);
  free(filename);

  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
