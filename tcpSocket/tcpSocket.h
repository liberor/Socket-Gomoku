#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <stdbool.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <windows.h>
#define SERVERPORT 52014
bool init_Socket();
bool close_Socket();
SOCKET createServerSocket();
SOCKET createClientSocket(const char* ip);