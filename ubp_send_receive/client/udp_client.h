#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sys/time.h>
#include <time.h>
#include "udp_client_define.h"

class udp_communication_client_demo
{
public:
	udp_communication_client_demo();
	~udp_communication_client_demo();

public:
	int start_ubp_client();
	
private:
	int send_message_server();
	
private:
	int 				m_ubp_client_fd;
	struct sockaddr_in 	m_server_addr;
	ifstream 			m_read_file_fd;	// 要打开的文件的句柄
	long long int		m_file_size;
	socklen_t			m_addr_len;
	char				m_read_buff[MAX_BUFF_SIZE];	
	struct timeval 		start;					// 开始时间
	struct timeval 		end;					// 结束时间
	double 				usec;					// 用时us 微妙

};

