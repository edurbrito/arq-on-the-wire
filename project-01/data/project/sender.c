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

  unsigned char control_package_start[20] = {STARTP, FILE_SIZE, sizeof(size), size, FILE_NAME, strlen(argv[2]), argv[2],'\0'};
  unsigned char control_package_end[20] = {END, FILE_SIZE, sizeof(size), size, FILE_NAME, strlen(argv[2]), argv[2],'\0'};
  
  llwrite(fd, control_package_start, strlen(control_package_start));
  
  int sequence_number = 0;
  
  int L1,L2;
  L2 = MAX_SIZE/256;
  L1 = MAX_SIZE%256;

  unsigned char data[MAX_SIZE - 4];
  unsigned char data_package[MAX_SIZE];
  
  while (total < size)
  {
    sequence_number = sequence_number%255;
    unsigned char data_info[4] = {DATA,sequence_number,L1,L2,'\0'};

    int l = MAX_SIZE > (size - total) ? size - total : MAX_SIZE;
    l -= 4;

    read(file, data, l);

    printf("%u\n", data);
    printf("%u\n", data_info);
    
    strcpy(data_package,data_info);
    memcpy(data_package+strlen(data_info),data,sizeof(data));

    printf("%u", data_package);
    
    total += llwrite(fd, data_package, l);
    sequence_number++;
    printf("NUMBER %d", sequence_number);
    
  }

  llwrite(fd, control_package_end, strlen(control_package_end));
  
  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
