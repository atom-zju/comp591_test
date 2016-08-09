all: sender receiver

sender: sender.cpp global.h
	g++ sender.cpp -Wall -g -o sender

receiver: receiver.cpp global.h
	g++ receiver.cpp -Wall -g -o receiver

clean: 
	rm sender receiver
