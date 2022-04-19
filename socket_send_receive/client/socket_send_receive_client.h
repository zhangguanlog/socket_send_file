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
	int open_read_write_file();

private:
	int 				m_client_fd;						// 客户端fd
	struct sockaddr_in 	m_server_addr;						// 要连接的服务端信息
	socklen_t 			m_client_demo_sock_len;				// 客户端信息长度
	char 				m_recv_buff[MAX_BUFF_RECV_LEN]; 	// 接收数据存储
	char 				m_send_buff[MAX_BUFF_LEN];			// 发送数据存储
	pthread_t 			m_recv_pt;							// 接收数据线程
	pthread_t 			m_send_pt;							// 发送数据线程	
	ifstream 			m_read_file_fd;						// 读取文件的字符句柄、
	ofstream			m_write_file_fd;					// 写文件句柄
};



