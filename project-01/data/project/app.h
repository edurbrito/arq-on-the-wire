
#include "utils.h"

#ifndef APP_H
#define APP_H

/**
 * Opens the transmition at port for the user type u
 * @param port to use for the transmission
 * @param u user type
 * @return -1 if an error occurred, or port file descriptor
*/
int llopen(int port, user u);

#endif