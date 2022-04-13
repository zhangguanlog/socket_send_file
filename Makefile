#!bin/bash

cpp = g++

cxxflags = -g -Wall

target = write_read

src = socket_file_read_send_write.cpp

$(target):socket_file_read_send_write.o
	$(cpp) $(cxxflags) socket_file_read_send_write.o -o $(target)

socket_file_read_send_write.o:$(src)
	$(cpp) -c $(src) -o socket_file_read_send_write.o


clean:
	rm -rf *.o $(target)


