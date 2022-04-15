#!bin/bash

cpp = g++

bin_dir = ../

cxxflags = -g -Wall

target = write_read.a

src = file_read_write.cpp

all:$(target) publish

$(target):file_read_write.o
	ar -crv  $(target) file_read_write.o
#	$(cpp) $(cxxflags) file_read_write.o -o $(target)
	

file_read_write.o:$(src)
	$(cpp) -c $(src) -o file_read_write.o


publish:
	mkdir -p $(bin_dir)bin
	cp -rf $(target) $(bin_dir)bin

clean:
	rm -rf *.o $(target) $(bin_dir)bin


