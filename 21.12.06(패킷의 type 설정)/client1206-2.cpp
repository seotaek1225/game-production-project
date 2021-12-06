#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
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

enum class CommandType {
	LOGIN,
	PLAYING,
	LOGOUT
};

struct Message {
	CommandType type;
	union {
		struct login_t {
			char userid[20];
			char passwd[20];
		}login;
		struct logout_t {

		}logout;
		struct playing_t {
			int x, y;
		}playing;
	}u;
};

int main()
{
	int retval;
	Message msg;
	msg.type = CommandType::LOGIN;

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
		if (msg.type == CommandType::LOGIN)
		{
			printf("id : ");
			scanf("%s", msg.u.login.userid);
			printf("password : ");
			scanf("%s", msg.u.login.passwd);
		}
		if (strcmp(buf, "end") == 0)
			break;

		retval = send(sock, (char*)&msg, sizeof(msg), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("send = %d\n", retval);
		//printf("x = %d, y = %d\n", packet.x, packet.y);
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
		printf("info size = %d\n", sizeof(msg));

		retval = recvn(sock, buf, retval, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		//printf("recvn = %d\n", retval);
		buf[retval] = '\0';
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);

		Message *temp;
		temp = (Message*)buf;
		if (temp->type == CommandType::LOGIN)
		{
			printf("로그인 실패\n");
			continue;
		}

		printf("[받은 데이터] x = %d, y = %d\n", temp->u.playing.x, temp->u.playing.y);
		//printf("address = %s, port = %d\n", temp->address, temp->portNum);
		msg.u.playing.x = temp->u.playing.x;
		msg.u.playing.y = temp->u.playing.y;
		msg.type = temp->type;

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