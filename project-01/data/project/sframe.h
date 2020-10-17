#include "utils.h"

#ifndef SFRAME_H
#define SFRAME_H

/**
 * Opens the transmition at port for the user type u
 * @param port to use for the transmission
 * @param u user type
 * @return -1 if an error occurred, or port file descriptor
*/
int llopen(int port, user u);

#endif