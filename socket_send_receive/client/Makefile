src = socket_send_receive_client.cpp

cxxflags = -g -Wall

target = client

$(target):$(src)
	g++ $(cxxflags) $(src) -o $(target) -lpthread 

clean:
	rm -rf *.o $(target)

