/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "datalink.h"
#include "receiver.h"

int parse_args(int argc, char **argv, int *port)
{
  char *p = NULL;
  int index, c;

  opterr = 0;

  while ((c = getopt(argc, argv, "p:")) != -1)
    switch (c)
    {
    case 'p':
      p = optarg;
      break;
    case '?':
      if (optopt == 'p')
        fprintf(stdout, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stdout, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stdout,
                "Unknown option character `\\x%x'.\n",
                optopt);
      fflush(stdout);
      return -1;
    default:
      return -1;
    }

  if (p != NULL)
    *port = atoi(p);
  else
    return -1;

  return atoi(p);
}

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

  int port;

  if (argc < 2 || parse_args(argc, argv, &port) < 0)
  {
    printf("Usage:\t./receiver.o -p serialport \n\tex: ./receiver.o -p 11\n");
    fflush(stdout);
    exit(1);
  }

  int fd = 0, file = 0, logs = 0, old_stdout = dup(STDOUT_FILENO);

  logs = open("logs/r.log", O_WRONLY | O_CREAT | O_TRUNC, 0777);
  dup2(logs, STDOUT_FILENO);

  if ((fd = llopen(port, RECEIVER)) <= 0)
  {
    printf("APP ##### Failed at llopen on port %d.\n", port);
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
      printf("APP ##### Failed at llread when receiving ctrl packet start.\n");
      free(buffer);
      return -1;
    }

    if (buffer[0] == STARTP)
    {

      ctrl_start = 1;

      if ((filesize = get_ctrl_packet_filesize(buffer)) <= 0)
      {
        printf("APP ##### Failed at get ctrl packet start filesize %d.\n", filesize);
        ctrl_start = 0;
      }

      if ((filename = get_ctrl_packet_filename(buffer)) == NULL)
      {
        printf("APP ##### Failed at get ctrl packet start filename %s.\n", filename);
        ctrl_start = 0;
      }
    }
  }

  if ((file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0)
  {
    printf("APP ##### Failed to open file to be written.\n");
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
      printf("APP ##### Failed at llread when receiving data packet.\n");
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
      printf("APP ##### Error when writing to new file.\n");
    }

    total += w;
  }

  while (!ctrl_end)
  {
    l = llread(fd, buffer);
    if (l <= 0)
    {
      printf("APP ##### Failed at llread when receiving ctrl packet end.\n");
      free(buffer);
      return -1;
    }

    if (buffer[0] == ENDP)
    {

      ctrl_end = 1;

      if (get_ctrl_packet_filesize(buffer) != filesize)
      {
        printf("APP ##### Failed at get ctrl packet end filesize %d.\n", filesize);
        ctrl_end = 0;
      }

      if (strcmp(get_ctrl_packet_filename(buffer), filename) != 0)
      {
        printf("APP ##### Failed at get ctrl packet end filename %s.\n", filename);
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
