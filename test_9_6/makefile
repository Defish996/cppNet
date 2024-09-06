.PHONY:all
all:tcpserver tcpclient

LDFLAG=#-DSelfDefine=1

tcpserver:TcpServerMain.cc
	g++ -o $@ $^ $(LDFLAG) -ljsoncpp -std=c++14

tcpclient:TcpClientMain.cc
	g++ -o $@ $^ $(LDFLAG) -ljsoncpp -std=c++14

.PHONY:clean
clean:
	rm -f tcpserver tcpclient