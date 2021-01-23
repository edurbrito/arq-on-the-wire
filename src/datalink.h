
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

/**
 * Writes length chars from buffer to port
 * @param port to use for the transmission
 * @param buffer to be sent to port
 * @param length size of buffer
 * @return -1 if an error occurred, or total bytes written
*/
int llwrite(int port, unsigned char * buffer, int length);

/**
 * Reads from port to buffer
 * @param port to use for the transmission
 * @param buffer to be filled with bytes read
 * @return -1 if an error occurred, or total bytes read
*/
int llread(int port, unsigned char *buffer);

/**
 * Closes the transmission at port
 * @param port where was established the transmission
 * @return -1 if an error occurred, success otherwise
*/
int llclose(int port);

#endif