#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <ctime>

using namespace std;

#include "file_read_write.h"

file_read_write::file_read_write()
{
	memset(m_read_buff, 0, sizeof(m_read_buff));
}

file_read_write::~file_read_write()
{
	m_read_file_fd.close();
	m_write_file_fd.close();
}

int file_read_write::operate_read_file_one_byte()
{
	m_read_file_fd.open(READ_FILE_NAME, ios::in | ios::binary);
	if (!m_read_file_fd.is_open())
	{
		cout << "open read file error!" << endl;
		return 1;
	}

	gettimeofday(&start, NULL);

	while (1)
	{
		m_read_file_fd.read(&m_byte_buff, 1);
		if (m_read_file_fd.eof())
		{
			break;
		}
		m_write_file_fd.write(&m_byte_buff, 1);
	}

	gettimeofday(&end, NULL);
	// 单字节读写时间11分05秒 10分09秒
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lfus\n", usec);

	return 0;
}

int file_read_write::operate_read_file()
{
	streampos 		position;
	long long int 	file_size;
	long int 		read_times_integer; // 读取文件大小 除 每次能读的大小的整数部分
	int 			remainder_size;		// 剩余的部分
	long int 		times;				// 写次数

	m_read_file_fd.open(READ_FILE_NAME, ios::in | ios::binary);
	if (!m_read_file_fd.is_open())
	{
		cout << "open read file error!" << endl;
		return 1;
	}

	// 文件指针设置到文件末尾
	m_read_file_fd.seekg(0, ios::end);
	// 获取文件的大小
	position = m_read_file_fd.tellg();
	file_size = position;
	// 文件指针移动到文件开始
	m_read_file_fd.seekg(0, ios::beg);
	cout << position << endl;
	cout << file_size << endl;

	// 计算读完文件需要读的次数
	read_times_integer = file_size / sizeof(m_read_buff);
	// 计算剩余还没有读取的部分
	remainder_size = file_size - (read_times_integer * sizeof(m_read_buff));

	cout << read_times_integer << " " << remainder_size << endl;

	gettimeofday(&start, NULL);

	// 读取的整数次数，每次读取sizeof(m_read_buff)
	for (times = 0; times < read_times_integer; times++)
	{
		m_read_file_fd.read(m_read_buff, sizeof(m_read_buff));
		if (m_read_file_fd.eof())
		{
			break;
		}
		// m_write_file_fd.write(m_read_buff, sizeof(m_read_buff));

		memset(m_read_buff, 0, sizeof(m_read_buff));
	}

	// 读写剩余的部分
	m_read_file_fd.read(m_read_buff, remainder_size);
	// m_write_file_fd.write(m_read_buff, remainder_size);

	gettimeofday(&end, NULL);
	// 单字节读写时间11分26秒 10分09秒
	usec = (end.tv_sec - start.tv_sec) * 1000000;
	usec += end.tv_usec - start.tv_usec;

	printf("%lfus\n", usec);

	return 0;
}

int file_read_write::operate_write_file()
{
	m_write_file_fd.open(WRITE_FILE_NAME, ios::out | ios::binary);
	if (!m_write_file_fd.is_open())
	{
		cout << "write file open error!" << endl;
		return 1;
	}

	return 0;
}

int file_read_write::start_read_write()
{
	int result;
	
	result = operate_write_file();
	if (result)
	{
		return 1;
	}

	// operate_read_file();
	operate_read_file_one_byte();

	return 0;
}

int main(int argc, char *argv[])
{
	file_read_write *file_operation = new file_read_write;

	file_operation->start_read_write();

	delete file_operation;

	return 0;
}


