#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#pragma comment(lib, "ws2_32")
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include<ctime>

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		512

void err_quit(const char *msg);
void err_display(const char *msg);
int recvn(SOCKET s, char* buf, int len, int flags);

typedef struct Infomation
{
	int x, y;
	int portNum;
	char address[100];
}Info;

typedef union Communication
{
	Info info;
	char arr[BUFSIZE];
}Com;


int main()
{
	int retval;
	//Position pos{ 0,0 };
	Com com;
	com.info.x = 0;
	com.info.y = 0;
	srand((unsigned)time(NULL));
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);

	retval = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	char buf[BUFSIZE + 1];
	int len;

	while (1)
	{

		if (strcmp(buf, "end") == 0)
			break;

		retval = send(sock, (char*)&com, sizeof(com), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("send = %d\n", retval);
		printf("x = %d, y = %d\n", com.info.x, com.info.y);
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
		printf("info size = %d\n", sizeof(com));
	
		retval = recvn(sock, buf, retval, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		printf("recvn = %d\n", retval);
		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		Com *temp;
		temp = (Com*)buf;
		printf("[받은 데이터] x = %d, y = %d\n", temp->info.x, temp->info.y);
		printf("address = %s, port = %d\n", temp->info.address, temp->info.portNum);
		com.info.x = temp->info.x;
		com.info.y = temp->info.y;
		//i++;
		Sleep((rand() % 80) * 100);
	}

	printf("[TCP 클라이언트] : 접속 종료.\n");

	closesocket(sock);

	WSACleanup();
	return 0;
}

void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0)
	{
		received = recv(s, buf, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return len - left;
}