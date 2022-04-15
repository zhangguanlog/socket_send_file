#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fstream>

using namespace std;

#include "socket_receive_send_server.h"

socket_receive_send_server::socket_receive_send_server()
{
	// 设置本地的一些地址信息
	m_server_accept_addr.sin_family = AF_INET;
	m_server_accept_addr.sin_port = htons(LOCAL_PRART);
	m_server_accept_addr.sin_addr.s_addr = htons(INADDR_ANY);

	// 字符串初始化
	memset(m_receive_buffer, 0, sizeof(m_receive_buffer));
	memset(m_send_buffer, 0, sizeof(m_send_buffer));
}

socket_receive_send_server::~socket_receive_send_server()
{
	// 关闭已打开的写入的文件
	m_write_file_fd.close();

	// 将线程状态修改为unjoinable状态
	pthread_detach(m_send_pt);
	pthread_detach(m_receive_pt);
	
	// 等待创建的线程结束
	pthread_join(m_receive_pt, NULL);
	pthread_join(m_send_pt, NULL);

	// 关闭socket
	close(m_socket_server_fd);
}

// 发送数据线程实现
void *socket_receive_send_server::create_send_message(void *param)
{
	socket_receive_send_server* send_t = (socket_receive_send_server *) param;

	// 等待从键盘获取字符串
	while(fgets(send_t->m_send_buffer, MAX_BUFFER_LEN, stdin))
	{
		// 发送输入的字符
		send(send_t->m_client_conn_fd, send_t->m_send_buffer, strlen(send_t->m_send_buffer), 0);
		memset(send_t->m_send_buffer, 0, MAX_BUFFER_LEN);
	}
	
	return NULL;
}

// 接收线程实现
void *socket_receive_send_server::create_receive_message(void *pragma)
{
	socket_receive_send_server* recv_t = (socket_receive_send_server *) pragma;
	int 						len;	

	// 打开指定的文件，没有的话创建
	recv_t->m_write_file_fd.open(WRITE_FILE_NAME, ios::out  | ios::ate | ios::binary);
	if (!recv_t->m_write_file_fd.is_open())
	{
		cout << "write file open error!" << endl;
		return NULL;
	}

	while (1)
	{
		// 读取缓冲区中的数据
		len = recv(recv_t->m_client_conn_fd, recv_t->m_receive_buffer, MAX_BUFFER_LEN, 0);
		if (len > 0)
		{		
			// cout << "len = " << len << " " << recv_t->m_receive_buffer;
			// 像文件中写入数据
			recv_t->m_write_file_fd.write(recv_t->m_receive_buffer, len);
			// 写入完成后字符串清零
			memset(recv_t->m_receive_buffer, 0, MAX_BUFFER_LEN);
		}
	}

	return NULL;
}

// 创建发送和接收线程
int socket_receive_send_server::process_buff_message()
{
	int result;

	result = pthread_create(&m_receive_pt, NULL, create_receive_message, this);
	if (result != 0)
	{
		cout << "create receive message thread error!" << endl;
		return -1;
	}

	result = pthread_create(&m_send_pt, NULL, create_send_message, this);
	if (result != 0)
	{
		cout << "create send message thrad error!" << endl;
		return -1;
	}

	return 0;
}

// 开始创建server
int socket_receive_send_server::start_socket_server()
{
	int 					result;
	socklen_t				len;
	char 					quit[] = "quit";

	// 创建socket
	m_socket_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket_server_fd < 0)
	{
		cout << "create server socket error!" << endl;
		return -1;
	}

	// 绑定网卡
	result = bind(m_socket_server_fd, (struct sockaddr *)&m_server_accept_addr, sizeof(struct sockaddr));
	if (result < 0)
	{
		cout << "server bind error!" << endl;
		return -2;
	}

	// 监听网卡
	result = listen(m_socket_server_fd, SERVER_MAX_NUM);
	if (result < 0)
	{
		cout << "listen error!" << endl;
		return -3;
	}

	// 等待客户端连接
	len = sizeof(struct sockaddr_in);
	m_client_conn_fd = accept(m_socket_server_fd, (struct sockaddr*)&m_server_accept_addr, &len);
	if (m_client_conn_fd < 0)
	{
		cout << "accept error!" << endl;
		return -1;
	}

	// 网络字节转换为本地字节
	char *ip = (char *)inet_ntoa(m_server_accept_addr.sin_addr);
	cout << "conn:" << m_client_conn_fd << "accept ip:" << ip << endl;

	// 创建线程
	process_buff_message();	

	// 退出主程序
	while (1)
	{
		if ((!strncmp(m_send_buffer, quit, 4)))
		{
			cout << "bye bye!" << endl;
			break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// 创建一个类的实例测试类
	socket_receive_send_server *server = new socket_receive_send_server;

	server->start_socket_server();

	delete server;

	return 0;
}

