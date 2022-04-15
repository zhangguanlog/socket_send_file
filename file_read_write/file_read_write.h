#pragma once
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include "file_define.h"

class file_read_write
{
public:
	file_read_write();
	~file_read_write();

public:
	int start_read_write();

private:
	int operate_read_file();
	int operate_write_file();
	int operate_read_file_one_byte();

private:
	ifstream 		m_read_file_fd;
	ofstream 		m_write_file_fd;
	char 			m_read_buff[2048];
	char			m_byte_buff;
	struct timeval 	start;
	struct timeval 	end;
	double usec;
	
};





