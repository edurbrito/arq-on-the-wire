# ARQ On The Wire

This project went around the development of a data link protocol and the provision of a reliable data communication service between two systems connected by a transmission medium - in this case, a serial cable. 

The development environment was established with the **LINUX** operating system, the **C Programming Language** and the **RS-232 Serial Ports** (asynchronous communication), as well as the respective Drivers and API functions provided by the operating system. 

## SRC

Our project follows a module-based organization structure. The interfaces and their implementations are clearly distinguished and can be found inside the `src` folder. The former correspond to the header files, allowing to create a higher layer of abstraction and to provide its users only the necessary blocks of code, which are in the source files, in an inner model that works like a black box.

The data link layer encompasses most of the available modules: 
* The `datalink.h` module, that contains the main functions listed - `llopen , llwrite , llread and llclose`. 
* `sframe.h` and `iframe.h` modules, which represent the state machines associated with the evaluation of the frames with supervision and information format, respectively. 
* `utils.h` module, that includes some auxiliary functions, configuration constants and the main data structures used by the other modules. 

The application layer includes the other modules, which use the functions provided by the previous modules:
* The `sender.h` module, that, besides providing the user interface for sending the data, also contains the functions for creating and sending the control packets and data packages. 
* The `receiver.h` module, which in addition to providing the user interface for receiving data, also contains the functions for receiving and confirming control packets and data packets.

## RUN

To experiment with the virtual serial port provided by `socat` and the Stop & Wait ARQ mechanism developed, run the following commands:

1. `docker build -t ubuntu:socat .`
2. `docker run -d --name stopwait ubuntu:socat`
3. `docker exec -it stopwait /bin/bash`

Inside the docker container
1. `./tests.sh` for exchanging the files inside the `test` folder between the sender and the receiver
2. `./diff.sh` for checking if every file was sent without errors

## TESTS

Testing means exchanging files on the virtual wire simulated by `socat`. These files are sent by the `sender` process and received and acknowledged by the `receiver` process.

Real simulations were made on a real serial port, between 2 computers, with some more noise and reliability tests, to ensure the ARQ mechanism reacted to error frames as expected.

The `tests` folder contains some files that can be used to test this protocol.

## REPORT

The full report with the logs and data collected can be found [here](report/T1G11.pdf).

## AUTHORS

* Eduardo Brito, [@edurbrito](https://github.com/edurbrito)
* Pedro Ferreira, [@pdff2000](https://github.com/pdff2000)