#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <ctime>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/sockios.h>

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
	// 将线程状态修改为unjoinable状态
	pthread_detach(m_send_pt);
	pthread_detach(m_receive_pt);
	
	// 等待创建的线程结束
	pthread_join(m_receive_pt, NULL);
	pthread_join(m_send_pt, NULL);

	// 关闭socket
	close(m_socket_server_fd);

	// 关闭已打开的写入的文件
	m_write_file_fd.close();	
	m_read_file_fd.close();
}

// 发送数据线程实现
void *socket_receive_send_server::create_send_message(void *param)
{
	socket_receive_send_server* send_t = (socket_receive_send_server *) param;
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
	// unsigned long int			value;

	// 打开需要发送的文件
	send_t->m_read_file_fd.open(READ_FILE_NAME, ios::in | ios::binary);
	if (!send_t->m_read_file_fd.is_open())
	{
		cout << "open read file error!" << endl;
		return NULL;
	}

	// 文件指针设置到文件末尾
	send_t->m_read_file_fd.seekg(0, ios::end);
	// 获取文件的大小
	position = send_t->m_read_file_fd.tellg();
	file_size = position;
	// 文件指针移动到文件开始
	send_t->m_read_file_fd.seekg(0, ios::beg);
	cout << file_size << endl;

	// 计算读完文件需要读的次数
	read_times_integer = file_size / MAX_BUFFER_SEND_LEN;
	// 计算剩余还没有读取的部分
	remainder_size = file_size - (read_times_integer * MAX_BUFFER_SEND_LEN);

	cout << "需要读的次数：" << read_times_integer << "剩余部分：" << remainder_size << endl;

	// 设置发送缓冲区的大小的一半为40k
	tcp_max_buff_size = 40 * 1024; 

	optlen = sizeof(tcp_max_buff_size);

	// 设置发送缓冲区的大小为80k
	result = setsockopt(send_t->m_client_conn_fd, SOL_SOCKET, SO_SNDBUF, &tcp_max_buff_size, optlen);
	if (result < 0)
	{
		cout << "set send buff error!" << endl;
	}

	optlen = sizeof(tcp_max_buff_size);

	// 获取发送缓冲区的大小
	result = getsockopt(send_t->m_client_conn_fd, SOL_SOCKET, SO_SNDBUF,&tcp_max_buff_size, &optlen);
	if (result < 0)
	{
		cout << "get the buff size error!" << endl;
	}
	cout << "默认的发送缓冲区的大小：" << tcp_max_buff_size << endl;

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
		// result = ioctl(send_t->m_client_conn_fd, SIOCOUTQ, &value);
		// cout << "value:" << value << endl;
		// if (result < 0)
		{
			// cout << "get send buff error!" << endl;
			// goto EXECUTE_SEND;
		}

		send_t->m_read_file_fd.read(send_t->m_send_buffer, sizeof(send_t->m_send_buffer));

		// 读到文件结尾，退出循环
		if (send_t->m_read_file_fd.eof())
		{
			cout << "end file!" << endl;
			break;
		}

		// 发送数据
		result = send(send_t->m_client_conn_fd, send_t->m_send_buffer, sizeof(send_t->m_send_buffer), 0);
		if (result != MAX_BUFFER_SEND_LEN)
		{
			cout << "send error! \n" << endl;
		}
		times++;
		// cout << times << " "<< read_times_integer << endl;
		memset(send_t->m_send_buffer, 0, MAX_BUFFER_SEND_LEN);
	}

	// 读写剩余的部分
	if (remainder_size > 0)
	{
		send_t->m_read_file_fd.read(send_t->m_send_buffer, remainder_size);	
		result = send(send_t->m_client_conn_fd, send_t->m_send_buffer, remainder_size, 0);
		if (result != remainder_size)
		{
			cout << "send sheng yu bu feng error!" << endl;
		}
		memset(send_t->m_send_buffer, 0, MAX_BUFFER_SEND_LEN);
	}

	// 结束时间计算
	gettimeofday(&end, NULL);
	// 单字节读写时间11分26秒 10分09秒 // 发送9264168960 需要664372940.000000 us
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lf us, %lf s, %lf min, %lf byte/s\n", usec, usec / 1000000, usec / 1000000 / 60, file_size / (usec / 1000000));

	// 发送完成退出程序
	// memcpy(client_send->m_recv_buff, "quit", 4);

// EXECUTE_SEND:

#if 0
	// 等待从键盘获取字符串
	while(fgets(send_t->m_send_buffer, MAX_BUFFER_LEN, stdin))
	{
		// 发送输入的字符
		send(send_t->m_client_conn_fd, send_t->m_send_buffer, strlen(send_t->m_send_buffer), 0);
		memset(send_t->m_send_buffer, 0, MAX_BUFFER_LEN);
	}

#endif
	return NULL;
}

// 接收线程实现
void *socket_receive_send_server::create_receive_message(void *pragma)
{
	socket_receive_send_server* recv_t = (socket_receive_send_server *) pragma;
	int 						len = 0;	

	// 打开指定的文件，没有的话创建
	recv_t->m_write_file_fd.open(WRITE_FILE_NAME, ios::out  | ios::ate | ios::binary);
	if (!recv_t->m_write_file_fd.is_open())
	{
		cout << "write file open error!" << endl;
		return NULL;
	}
	
	cout << "write file open ok!" << endl;

	while (1)
	{
		// 读取缓冲区中的数据
		len = recv(recv_t->m_client_conn_fd, recv_t->m_receive_buffer, MAX_BUFFER_LEN, 0);
		if (len > 0)
		{		
			// cout << "len = " << len << "\n" << recv_t->m_receive_buffer;
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

	// 创建socket，默认阻塞模式
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
#if 0
		if ((!strncmp(m_send_buffer, quit, 4)))
		{
			cout << "bye bye!" << endl;
			break;
		}
#endif
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

