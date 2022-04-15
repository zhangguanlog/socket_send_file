#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include "socket_define_client.h"

// :public file_read_write
class client_socket_send_receive 
{
public:
	client_socket_send_receive();
	~client_socket_send_receive();

public:
	int begin_socket_client();

private:
	int client_process_recv_send_buff();
	static void *client_recv_message(void *param);
	static void *client_send_message(void *param);

private:
	int 				m_client_fd;
	struct sockaddr_in 	m_server_addr;
	socklen_t 			m_client_demo_sock_len;
	char 				m_recv_buff[MAX_BUFF_LEN];
	char 				m_send_buff[MAX_BUFF_LEN];
	pthread_t 			m_recv_pt;
	pthread_t 			m_send_pt;
};



