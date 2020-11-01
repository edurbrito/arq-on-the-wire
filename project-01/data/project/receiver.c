/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "datalink.h"
#include "receiver.h"

int get_ctrl_packet_filesize(unsigned char *buffer)
{
  if (buffer[1] == FILE_SIZEP)
  {
    int l1 = buffer[2]; // FILE SIZE string size in chars
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
  int l1 = buffer[2]; // FILE SIZE string size in chars
  if (buffer[3 + l1] == FILE_NAMEP)
  {
    int l2 = buffer[4 + l1]; // FILE NAME string size in chars
    unsigned char *filename = malloc((l2 + 1) * sizeof(unsigned char)); // TEST : + 1
    int i;
    for (i = 0; i < l2; i++)
    {
      filename[i] = buffer[5 + l1 + i];
    }

    filename[i] = '_'; // TEST

    return filename;
  }

  return NULL;
}

int get_data_packet_size(unsigned char * buffer, int nr, int lread){
  unsigned char C = buffer[0];
  unsigned char N = buffer[1];
  unsigned char L2 = buffer[2];
  unsigned char L1 = buffer[3];

  int l2 = (int) L2;
  int l1 = (int) L1;
  int l = 256 * l2 + l1;

  if(C != DATAP || (int) N != nr || l != lread)
    return -1;
  
  return l;  
}

int main(int argc, char **argv)
{

  int port;

  if (argc < 2 || parse_args(argc, argv, &port, NULL) < 0)
  {
    logpf(printf("Usage:\t./receiver.o -p serialport \n\tex: ./receiver.o -p 11\n"));
    fflush(stdout);
    exit(1);
  }

  int fd = 0, file = 0, logs = 0, old_stdout = dup(STDOUT_FILENO);

  logs = open("logs/r.log", O_WRONLY | O_CREAT | O_TRUNC, 0777);
  dup2(logs, STDOUT_FILENO);

  if ((fd = llopen(port, RECEIVER)) <= 0)
  {
    logpf(printf("APP ##### Failed at llopen on port %d.\n", port));
    return -1;
  }

  unsigned char *buffer = malloc(MAX_SIZE * sizeof(char));
  int l = 0;
  int filesize;
  unsigned char *filename;

  int ctrl_start = 0, ctrl_end = 0;

  while (!ctrl_start)
  {
    l = llread(fd, buffer);
    if (l <= 0)
    {
      logpf(printf("APP ##### Failed at llread when receiving ctrl packet start.\n"));
      free(buffer);
      return -1;
    }

    if (buffer[0] == STARTP)
    {

      ctrl_start = 1;

      if ((filesize = get_ctrl_packet_filesize(buffer)) <= 0)
      {
        logpf(printf("APP ##### Failed at get ctrl packet start filesize %d.\n", filesize));
        ctrl_start = 0;
      }

      if ((filename = get_ctrl_packet_filename(buffer)) == NULL)
      {
        logpf(printf("APP ##### Failed at get ctrl packet start filename %s.\n", filename));
        ctrl_start = 0;
      }
    }
  }

  if ((file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
  {
    logpf(printf("APP ##### Failed to open file to be written.\n"));
    free(buffer);
    free(filename);
    return -1;
  }

  long int total = 0;
  int w = 0, nr = 0;

  while (total < filesize)
  {
    int l = llread(fd, buffer);
    int datasize = l - 4;
    
    if (l < 0 || get_data_packet_size(buffer, nr % 255, datasize) < 0) // -4 represents the app packet header
    {
      logpf(printf("APP ##### Failed at llread when receiving data packet.\n"));
      free(buffer);
      free(filename);
      return -1;
    }

    unsigned char aux[datasize];

    for (size_t i = 4; i < l; i++)
    {
      aux[i - 4] = buffer[i];
    }

    int w = 0;

    if ((w = write(file, aux, datasize)) < datasize)
    {
      logpf(printf("APP ##### Error when writing to new file.\n"));
    }

    total += w;
    nr += 1;

    if(logs > 0)
      send_user_message(old_stdout, filename, total, filesize, "Downloading");
  }

  while (!ctrl_end)
  {
    l = llread(fd, buffer);
    if (l <= 0)
    {
      logpf(printf("APP ##### Failed at llread when receiving ctrl packet end.\n"));
      free(buffer);
      return -1;
    }

    if (buffer[0] == ENDP)
    {

      ctrl_end = 1;

      if (get_ctrl_packet_filesize(buffer) != filesize)
      {
        logpf(printf("APP ##### Failed at get ctrl packet end filesize %d.\n", filesize));
        ctrl_end = 0;
      }

      if (strcmp(get_ctrl_packet_filename(buffer), filename) != 0)
      {
        logpf(printf("APP ##### Failed at get ctrl packet end filename %s.\n", filename));
        ctrl_end = 0;
      }
    }
  }

  free(buffer);
  free(filename);

  close(file);
  if (llclose(fd) < 0)
    return -1;

  fflush(stdout);
  dup2(old_stdout, STDOUT_FILENO);

  return 0;
}
