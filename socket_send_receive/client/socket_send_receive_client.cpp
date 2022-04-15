#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <ctime>
#include <sys/ioctl.h>
#include <linux/sockios.h>

using namespace std;

#include "socket_send_receive_client.h"

client_socket_send_receive::client_socket_send_receive()
{
	// 存储字符串设置为0
	memset(m_recv_buff, 0, sizeof(m_recv_buff));
	memset(m_send_buff, 0, sizeof(m_send_buff));

	// 连接服务端信息初始化，协议族
	m_server_addr.sin_family = AF_INET;
	// 服务器地址ip
	m_server_addr.sin_addr.s_addr = inet_addr("192.168.88.133");
	// 连接端口号
	m_server_addr.sin_port = htons(8888);
	// 结构体sockaddr_in长度
	m_client_demo_sock_len = sizeof(m_server_addr);
}

client_socket_send_receive::~client_socket_send_receive()
{
	// 将线程状态修改为unjoinable状态
	pthread_detach(m_send_pt);
	pthread_detach(m_recv_pt);

	// 等待创建的线程结束
	pthread_join(m_recv_pt, NULL);
	pthread_join(m_send_pt, NULL);

	// 关闭socket
	close(m_client_fd);
}

// 接收线程
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

// 发送线程
void *client_socket_send_receive::client_send_message(void *param)
{
	client_socket_send_receive 	*client_send = (client_socket_send_receive *)param;
	ifstream 					read_file_fd;			// 读取文件的字符句柄
	streampos 					position;				// 文件末尾位置的大小
	long long int 				file_size;				// 文件的大小
	long int 					read_times_integer; 	// 读取文件大小 除 每次能读的大小的整数部分
	int 						remainder_size;			// 剩余的部分大小
	long int 					times = 0;				// 写次数	
	struct timeval 				start;					// 开始时间
	struct timeval 				end;					// 结束时间
	double 						usec;					// 用时us 微妙
	int 						result;					// 每次执行的结果记录
	int 						tcp_max_buff_size = 0;	// 发送缓冲区的大小
	socklen_t 					optlen;    				// 选项值长度
	unsigned long int			value;

	// 打开需要发送的文件
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

	// 设置发送缓冲区的大小的一半为40k
	tcp_max_buff_size = 40 * 1024; 

	optlen = sizeof(tcp_max_buff_size);

	// 设置发送缓冲区的大小为80k
	result = setsockopt(client_send->m_client_fd, SOL_SOCKET, SO_SNDBUF, &tcp_max_buff_size, optlen);
	if (result < 0)
	{
		cout << "set send buff error!" << endl;
	}

	optlen = sizeof(tcp_max_buff_size);

	// 获取发送缓冲区的大小
	result = getsockopt(client_send->m_client_fd, SOL_SOCKET, SO_SNDBUF,&tcp_max_buff_size, &optlen);
	if (result < 0)
	{
		cout << "get the buff size error!" << endl;
	}
	cout << tcp_max_buff_size << endl;

	// 记录开始时间
	gettimeofday(&start, NULL);

	// 开始读文件数据，然后发送
	while (1)
	{
		// 读取到最大次数退出循环
		if (times == read_times_integer)
		{
			cout << "read over!" << endl;
			break;
		}

		// 获取已使用的发送缓冲区的大小
		result = ioctl(client_send->m_client_fd, SIOCOUTQ, &value);
		if (result < 0)
		{
			cout << "get send buff error!" << endl;
			goto EXECUTE_SEND;
		}

		// 缓冲区存不下了，不要读取
		if ((tcp_max_buff_size - value) < 2048 * 2)
		{
			// cout << tcp_max_buff_size - value << endl;
			continue;
		}		
		
		// 读取数据，每次读取buff能够存的最大值
		read_file_fd.read(client_send->m_send_buff, sizeof(client_send->m_send_buff));

		// 读到文件结尾，退出循环
		if (read_file_fd.eof())
		{
			cout << "end file!" << endl;
			break;
		}

		// 发送数据
		result = send(client_send->m_client_fd, client_send->m_send_buff, sizeof(client_send->m_send_buff), 0);
		if (result != MAX_BUFF_LEN)
		{
			cout << "send error! \n" << endl;
		}
		times++;
		// cout << times << " "<< read_times_integer << endl;
		memset(client_send->m_send_buff, 0, MAX_BUFF_LEN);
	}

	// 读写剩余的部分
	if (remainder_size > 0)
	{
		read_file_fd.read(client_send->m_send_buff, remainder_size);	
		result = send(client_send->m_client_fd, client_send->m_send_buff, remainder_size, 0);
		if (result != remainder_size)
		{
			cout << "send sheng yu bu feng error!" << endl;
		}
		memset(client_send->m_send_buff, 0, MAX_BUFF_LEN);
	}

	// 结束时间计算
	gettimeofday(&end, NULL);
	// 单字节读写时间11分26秒 10分09秒 // 发送9264168960 需要664372940.000000 us
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lfus, %lfs, %lfmin\n", usec, usec / 1000000, usec / 1000000 / 60);

	// 发送完成退出程序
	memcpy(client_send->m_recv_buff, "quit", 4);

EXECUTE_SEND:

	read_file_fd.close();
	return NULL;
}

int client_socket_send_receive::client_process_recv_send_buff()
{
	int result;
	
	result = pthread_create(&m_recv_pt, NULL, client_recv_message, this);
	if (result)
	{
		cout << "create receive thread error!" << endl;
		return -1;
	}

	result = pthread_create(&m_send_pt, NULL, client_send_message, this);
	if (result)
	{
		cout << "create send thread error!" << endl;
		return -2;
	}

	return 0;
}

int client_socket_send_receive::begin_socket_client()
{
	int result;
	char exit_buff[] = "quit";

	// 创建一个socket
	m_client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_client_fd < 0)
	{
		cout << "create socket error!" << endl;
		return -1;
	}

	// 连接服务端
	result = connect(m_client_fd, (struct sockaddr*)&m_server_addr, m_client_demo_sock_len);
	if (result < 0)
	{
		cout << "connect error!" << endl;
		return -2;
	}

	// 创建接收发送线程
	client_process_recv_send_buff();

	// 退出主程序
	while (1)
	{
		if ((!strncmp(m_recv_buff, exit_buff, 4)))
		{
			cout << "bye bye!" << endl;
			break;
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


