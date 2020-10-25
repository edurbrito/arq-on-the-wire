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
  

  int size_file = (int)size;
  
  int count = 0;
  printf("HERE + %d\n", size_file);
  while (size_file != 0) {
      size_file /= 10;     
      ++count;
  }

  char size_hex[10];
  sprintf(size_hex,"%ld" ,size);
  
  int control_package_size = 5 + count + strlen(argv[2]);

  unsigned char control_package_start[control_package_size];
  unsigned char control_package_end[control_package_size];

  control_package_start[0] = STARTP;
  control_package_start[1] = FILE_SIZE;
  control_package_start[2] = count;
  for (int i=1; i<count+1;i++)
  {
    control_package_start[i+2] = (size_hex[i-1]);
  }
  control_package_start[3+count] = FILE_NAME;
  control_package_start[4+count] = strlen(argv[2]);
  for (int i=1; i<=strlen(argv[2]);i++)
  {
    control_package_start[4+count+i] = argv[2][i-1];
  }
  
  llwrite(fd, control_package_start, control_package_size);
  
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
    
    strcpy(data_package,data_info);
    memcpy(data_package+strlen(data_info),data,sizeof(data));
    
    total += llwrite(fd, data_package, l);
    sequence_number++;
    
  }

  control_package_start[0] = END;
  llwrite(fd, control_package_start, control_package_size);

  close(file);
  if (llclose(fd) < 0)
    return -1;

  return 0;
}
