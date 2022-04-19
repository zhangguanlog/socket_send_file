#!bin/bash

target = udp_client

cpp = g++

cxxflags = -g -Wall 

$(target):udp_client.o
	$(cpp) $(cxxflags) udp_client.o -o $(target)

udp_client.o:udp_client.cpp
	$(cpp)  -c udp_client.cpp -o udp_client.o


clean:
	rm -rf *.o $(target)

