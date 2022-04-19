#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#include "udp_client.h"

udp_communication_client_demo::udp_communication_client_demo()
{
	memset(&m_server_addr, 0, sizeof(m_server_addr));

	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(PORT_NUM);
	m_server_addr.sin_addr.s_addr = inet_addr(SERVER_IP4_ADDR);

	m_addr_len = sizeof(m_server_addr);
}

udp_communication_client_demo::~udp_communication_client_demo()
{
	close(m_ubp_client_fd);
}

int udp_communication_client_demo::send_message_server()
{
	long long int read_times_inter = 0;
	unsigned long int residue_portion = 0;
	long long int times = 0;
	int result;

	m_read_file_fd.seekg(0, ios::end);
	m_file_size = m_read_file_fd.tellg();
	m_read_file_fd.seekg(0, ios::beg);

	cout << "文件大小：" << m_file_size << endl;

	// 读的次数
	read_times_inter = m_file_size / MAX_BUFF_SIZE;
	// 剩余的部分
	residue_portion = m_file_size - (read_times_inter * MAX_BUFF_SIZE);

	cout << "次数："    << read_times_inter << " " << "剩余部分：" << residue_portion << endl;

	// 记录开始时间
	gettimeofday(&start, NULL);

	while (1)
	{
		if (times == read_times_inter)
		{
			cout << "整数部分读完了！" << endl;
			break;
		}

		if (m_read_file_fd.eof())
		{
			cout << "读到文件结尾！" << endl;
			goto END_FILE;
		}
		
		m_read_file_fd.read(m_read_buff, MAX_BUFF_SIZE);

		result = sendto(m_ubp_client_fd, m_read_buff, MAX_BUFF_SIZE, 0, (struct sockaddr *)&m_server_addr, m_addr_len);
		if (result != MAX_BUFF_SIZE)
		{
			cout << "发送错误！" << endl;
			return -1;
		}
		times++;
	}

	if (residue_portion > 0)
	{
		m_read_file_fd.read(m_read_buff, residue_portion);
		result = sendto(m_ubp_client_fd, m_read_buff, residue_portion, 0, (struct sockaddr *)&m_server_addr, m_addr_len);
		if (result != residue_portion)
		{
			cout << "剩余部分发送错误!" << endl;
			return -1;
		}
	}

END_FILE:

	// 结束时间计算
	gettimeofday(&end, NULL);
	// 单字节读写时间11分26秒 10分09秒 // 发送9264168960 需要664372940.000000 us
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lf us, %lf s, %lf min, %lf byte/s\n", usec, usec / 1000000, usec / 1000000 / 60, m_file_size / (usec / 1000000));

	return 0;
}

int udp_communication_client_demo::start_ubp_client()
{
	m_ubp_client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_ubp_client_fd < 0)
	{
		cout << "创建socke udp 错误！" << endl;
		return -1;
	}

	m_read_file_fd.open(READ_FILE_PATH, ios::in | ios::binary);
	if (!m_read_file_fd.is_open())
	{
		cout << "文件打开错误" << endl;
		return -1;
	}

	send_message_server();
	
	return 0;
}

int main(int argc, char *argv[])
{
	udp_communication_client_demo test;

	test.start_ubp_client();

	return 0;
}



