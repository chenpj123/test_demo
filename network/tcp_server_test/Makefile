CC=arm-none-linux-gnueabi-gcc

cantest: netlib.c tcpserver.c
	$(CC) -o tcp_server_test netlib.c tcpserver.c -lpthread

clean:
	rm *.o tcp_server_test


