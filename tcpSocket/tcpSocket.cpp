#include "tcpSocket.h"

bool init_Socket()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		return false;
	}
	return true;
}

bool close_Socket()
{
	if (WSACleanup() != 0) {
		return false;
	}
	return true;
}

SOCKET createServerSocket()
{
	SOCKET fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (fd == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVERPORT);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;

	if (bind(fd, (struct sockaddr*)(&addr), sizeof(addr)) == SOCKET_ERROR) {
		return false;
	}
	listen(fd, 1);
	return fd;
}

SOCKET createClientSocket(const char * ip)
{
	SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVERPORT);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	while ((connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)) Sleep(1000);
	return fd;
}
