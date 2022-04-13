#pragma once
#include <fstream>
#include <sys/time.h>
#include <unistd.h>
#include "read_write_define.h"

class socket_file_read_send_write
{
public:
	socket_file_read_send_write();
	~socket_file_read_send_write();

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





