
CC = g++
CFLAGS = -Wall -c -g
LFLAGS =

RECV = receiver
SEND = sender
RECV_SRC = receiver.cpp sockwall.cpp sockwindow.cpp
SEND_SRC = sender.cpp sockwall.cpp sockwindow.cpp
RECV_OBJ = receiver.o sockwall.o sockwindow.o
SEND_OBJ = sender.o sockwall.o sockwindow.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all :  $(RECV) $(SEND)

$(RECV) : $(RECV_OBJ)
	$(CC) $(LFLAGS) -o $(RECV) $(RECV_OBJ)

$(SEND) : $(SEND_OBJ)
	$(CC) $(LFLAGS) -o $(SEND) $(SEND_OBJ)

$(SEND_OBJS): global.h

$(RECV_OBJS): global.h

receiver.o :  sockwindow.h sockwall.h
sender.o :  sockwindow.h sockwall.h
sockwall.o : sockwindow.h sockwall.h
sockwindow.o : sockwindow.h

clean:
	rm -f *.o receiver sender
