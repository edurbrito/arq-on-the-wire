/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "app.h"
#include "receiver.h"

int get_ctrl_packet_filesize(unsigned char * buffer){
  if(buffer[1] == FILE_SIZEP){
    int l1 = buffer[2];
    unsigned char filesize_s[l1];
    for (int i = 0; i < l1; i++)
    {
      filesize_s[l1-i-1] = buffer[i+3];
    }

    return atoi(filesize_s);
  }

  return 0;
}

unsigned char * get_ctrl_packet_filename(unsigned char * buffer){
  int l1 = buffer[2];
  if(buffer[3+l1] == FILE_NAMEP){
    int l2 = buffer[4+l1];
    unsigned char * filename = malloc((l2+1) * sizeof(unsigned char));
    int i;
    for (i = 0; i < l2; i++)
    {
      filename[i] = buffer[5+l1+i];
    }

    filename[i] = '_';

    return filename;
  }

  return NULL;
}

int main(int argc, char **argv)
{

  if (argc < 3)
  {
    printf("Usage:\t./a.o serialport filename \n\tex: ./a.o 10 /tests/pr.gif\n");
    exit(1);
  }

  int fd = 0, file = 0;

  if ((fd = llopen(atoi(argv[1]), RECEIVER)) <= 0)
    return -1;

  unsigned char *buffer = malloc(MAX_SIZE * sizeof(char));
  int l = 0;

  while(buffer[0] != STARTP){
    l = llread(fd, buffer);
  }

  int filesize;

  if((filesize = get_ctrl_packet_filesize(buffer)) <= 0)
    return -1;

  unsigned char * filename;

  if((filename = get_ctrl_packet_filename(buffer)) == NULL)
    return -1;
  
  file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);

  long int total = 0;
  int w = 0;

  while (total < filesize)
  {
    int l = llread(fd, buffer);

    unsigned char aux[l-4];

    for (size_t i = 4; i < l; i++)
    {
      aux[i-4] = buffer[i];
    }

    total += write(file, aux, l-4);
  }

  l = llread(fd, buffer);

  free(buffer);
  free(filename);

  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
