#pragma once
#include <netinet/in.h>
#include <pthread.h>
#include "socket_define_server.h"

class socket_receive_send_server
{
public:
	socket_receive_send_server();
	~socket_receive_send_server();

public:
	int start_socket_server();

private:
	int process_buff_message();
	static void *create_receive_message(void *pragma);
	static void *create_send_message(void *param);
	

private:
	char 				m_receive_buffer[MAX_BUFFER_LEN];
	char 				m_send_buffer[MAX_BUFFER_SEND_LEN];
	pthread_t 			m_send_pt;
	pthread_t 			m_receive_pt;
	int 				m_socket_server_fd;
	struct sockaddr_in 	m_server_accept_addr;
	int 				m_client_conn_fd;
	ofstream			m_write_file_fd;	
	ifstream 			m_read_file_fd;						// 读取文件的字符句柄、
};


