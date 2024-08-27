.PHONY:all
all : tcp_serverd tcp_client
tcp_serverd:Main.cc
	g++ -o $@  $^ -std=c++14
tcp_client:TcpClient.cc
	g++ -o $@  $^ -std=c++14
.PHONY:clean
clean:
	rm -f tcp_serverd tcp_client