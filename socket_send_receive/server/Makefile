#!bin/bash

src = socket_receive_send_server.cpp

cxxflags = -g -Wall

target = server

$(target):$(src)
	g++ $(cxxflags) $(src) -o $(target) -lpthread 

clean:
	rm -rf *.o $(target)

