C=gcc
CDEPS=sframe.c iframe.c datalink.c utils.c

.PHONY: all

all: clean sender receiver

clean:
	rm -f sender.o receiver.o

sender: 
	$(C) sender.c $(CDEPS) -o sender.o

receiver: 
	$(C) receiver.c $(CDEPS) -o receiver.o

