
#include "utils.h"

#ifndef APP_H
#define APP_H

/**
 * Opens the transmission at port for the user type u
 * @param port to use for the transmission
 * @param u user type
 * @return -1 if an error occurred, or port file descriptor
*/
int llopen(int port, user u);

int llwrite(int port, char * buffer, int length);

int llread(int port, char *buffer);

/**
 * Closes the transmission at port
 * @param port where was established the transmission
 * @return -1 if an error occurred, success otherwise
*/
int llclose(int port);

#endif