/*Non-Canonical Input Processing*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "app.h"
#include "sender.h"

int parse_args(int argc, char **argv, int *port, char *filename)
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
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr,
                "Unknown option character `\\x%x'.\n",
                optopt);
      fflush(stdout);
      return -1;
    default:
      return -1;
    }

  if(p != NULL)
    *port = atoi(p);
  else
    return -1;

  strcpy(filename, argv[optind]);

  return atoi(p);
}

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
  int port;
  char * filename = malloc(MAX_SIZE*sizeof(char));

  if (argc < 3 || parse_args(argc, argv, &port, filename) < 0)
  {
    printf("Usage:\t./a.o -p serialport filename \n\tex: ./a.o -p 10 /tests/p.gif\n");
    exit(1);
  }

  int fd = 0, file = 0, logs = 0, old_stdout = dup(STDOUT_FILENO);

  logs = open("logs/s.log", O_WRONLY | O_CREAT | O_TRUNC, 0777);
  dup2(logs, STDOUT_FILENO);

  if ((fd = llopen(port, SENDER)) < 0)
  {
    printf("Failed at llopen on port %d.\n", port);
    return -1;
  }

  if ((file = open(filename, O_RDONLY)) < 0)
  {
    printf("Failed to open file to be sent.\n");
    return -1;
  }

  struct stat st;
  stat(filename, &st);
  off_t size = st.st_size;

  if (send_ctrl_packet(STARTP, fd, size, filename) <= 0)
    return -1;

  long int total = 0;
  int nr = 0;

  while (total < size)
  {
    unsigned char buffer[MAX_SIZEP];
    int l = MAX_SIZEP > (size - total) ? size - total : MAX_SIZEP;
    int r = 0;
    if ((r = read(file, buffer, l)) < l)
    {
      printf("Error when reading from file to be sent.\n");
    }
    total += r;
    if (send_data_packet(fd, nr % 255, buffer, r) <= 0)
      return -1;
    nr++;
  }

  if (send_ctrl_packet(ENDP, fd, size, filename) <= 0)
    return -1;

  close(file);
  free(filename);
  if (llclose(fd) < 0)
    return -1;

  fflush(stdout);
  dup2(old_stdout, STDOUT_FILENO);

  return 0;
}
