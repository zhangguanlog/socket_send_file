#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <ctime>

using namespace std;

#include "socket_send_receive_client.h"

client_socket_send_receive::client_socket_send_receive()
{
	memset(m_recv_buff, 0, sizeof(m_recv_buff));
	memset(m_send_buff, 0, sizeof(m_send_buff));
	
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_addr.s_addr = inet_addr("192.168.88.129");
	m_server_addr.sin_port = htons(8888);
	m_client_demo_sock_len = sizeof(m_server_addr);
}

client_socket_send_receive::~client_socket_send_receive()
{
	close(m_client_fd);
}

void *client_socket_send_receive::client_recv_message(void *param)
{
	client_socket_send_receive *client_parm = (client_socket_send_receive *)param;
	int recv_len;

	while (1)
	{
		recv_len = recv(client_parm->m_client_fd, client_parm->m_recv_buff, MAX_BUFF_LEN, 0);
		if (recv_len >= 0)
		{
			cout << client_parm->m_recv_buff;
			memset(client_parm->m_recv_buff, 0, MAX_BUFF_LEN);
		}
	}

	return NULL;
}

void *client_socket_send_receive::client_send_message(void *param)
{
	client_socket_send_receive 	*client_send = (client_socket_send_receive *)param;
	ifstream 					read_file_fd;
	streampos 					position;
	long long int 				file_size;
	long int 					read_times_integer; // 读取文件大小 除 每次能读的大小的整数部分
	int 						remainder_size;		// 剩余的部分
	long int 					times = 0;				// 写次数	
	struct timeval 				start;
	struct timeval 				end;
	double 						usec;
	int 						result;
	int							need_read = 1;

	read_file_fd.open(READ_FILE_NAME, ios::in | ios::binary);
	if (!read_file_fd.is_open())
	{
		cout << "open read file error!" << endl;
		return NULL;
	}

	// 文件指针设置到文件末尾
	read_file_fd.seekg(0, ios::end);
	// 获取文件的大小
	position = read_file_fd.tellg();
	file_size = position;
	// 文件指针移动到文件开始
	read_file_fd.seekg(0, ios::beg);
	cout << file_size << endl;

	// 计算读完文件需要读的次数
	read_times_integer = file_size / MAX_BUFF_LEN;
	// 计算剩余还没有读取的部分
	remainder_size = file_size - (read_times_integer * MAX_BUFF_LEN);

	cout << read_times_integer << " " << remainder_size << endl;

	gettimeofday(&start, NULL);

	// 读取的整数次数，每次读取sizeof(m_read_buff)
	// for (times = 0; times < read_times_integer; times++)
	while (1)
	{
		if (times == read_times_integer)
		{
			cout << "read over!" << endl;
			break;
		}
		if (need_read)
		{
			read_file_fd.read(client_send->m_send_buff, sizeof(client_send->m_send_buff));
		}

		if (read_file_fd.eof())
		{
			while (1)
			{
				cout << "end file!" << endl;
			}
			break;
		}		
		result = send(client_send->m_client_fd, client_send->m_send_buff, sizeof(client_send->m_send_buff), 0);
		if (result != MAX_BUFF_LEN)
		{
			cout << "send error! \n" << endl;
			need_read = 0;
			continue;
		}
		need_read = 1;
		times++;
		// cout << times << " "<< read_times_integer << endl;
		memset(client_send->m_send_buff, 0, MAX_BUFF_LEN);
	}

	// 读写剩余的部分
	if (remainder_size > 0)
	{
		read_file_fd.read(client_send->m_send_buff, remainder_size);	
		send(client_send->m_client_fd, client_send->m_send_buff, remainder_size, 0);
		memset(client_send->m_send_buff, 0, MAX_BUFF_LEN);
	}
	
	gettimeofday(&end, NULL);
	// 单字节读写时间11分26秒 10分09秒 // 发送9264168960 需要664372940.000000 us
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lfus\n", usec);

#if 0
	while(fgets(client_send->m_send_buff, MAX_BUFF_LEN, stdin))
	{
		send(client_send->m_client_fd, client_send->m_send_buff, strlen(client_send->m_send_buff), 0);
		memset(client_send->m_send_buff, 0, MAX_BUFF_LEN);
	}
#endif

	read_file_fd.close();

	return NULL;
}

int client_socket_send_receive::client_process_recv_send_buff()
{
	pthread_create(&m_recv_pt, NULL, client_recv_message, this);

	pthread_create(&m_send_pt, NULL, client_send_message, this);

	return 0;
}

int client_socket_send_receive::begin_socket_client()
{
	int result;
	char exit_buff[] = "quit";

	m_client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_client_fd < 0)
	{
		cout << "create socket error!" << endl;
		return -1;
	}

	result = connect(m_client_fd, (struct sockaddr*)&m_server_addr, m_client_demo_sock_len);
	if (result < 0)
	{
		cout << "connect error!" << endl;
		return -2;
	}

	client_process_recv_send_buff();

	while (1)
	{
		if ((!strncmp(m_recv_buff, exit_buff, 4)) || (!strncmp(m_send_buff, exit_buff, 4)))
		{
			// break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	client_socket_send_receive *test = new client_socket_send_receive;

	test->begin_socket_client();

	delete test;

	return 0;
}


