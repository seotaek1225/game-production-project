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

typedef struct Packet
{
	int x, y;
	int portNum;
	char address[100];
	char id[20];
	char password[20];
	bool login;
}Packet;

int main()
{
	int retval;
	//Position pos{ 0,0 };
	/*Com com;
	com.info.x = 0;
	com.info.y = 0;*/
	Packet packet;
	packet.x = 0;
	packet.y = 0;
	packet.login = false;
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
		if (packet.login == false)
		{
			printf("id : ");
			scanf("%s", packet.id);
			printf("password : ");
			scanf("%s", packet.password);
		}
		
		if (strcmp(buf, "end") == 0)
			break;

		retval = send(sock, (char*)&packet, sizeof(packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("send = %d\n", retval);
		printf("x = %d, y = %d\n", packet.x, packet.y);
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
		printf("info size = %d\n", sizeof(packet));

		retval = recvn(sock, buf, retval, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		printf("recvn = %d\n", retval);
		buf[retval] = '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		/*Com *temp;
		temp = (Com*)buf;*/
		Packet *temp;
		temp = (Packet*)buf;
		printf("[���� ������] x = %d, y = %d\n", temp->x, temp->y);
		printf("address = %s, port = %d\n", temp->address, temp->portNum);
		packet.x = temp->x;
		packet.y = temp->y;
		packet.login = temp->login;
		//i++;
		Sleep((rand() % 80) * 100);
		if (packet.login == false)
		{
			printf("�α��� ����\n");
		}
	}

	printf("[TCP Ŭ���̾�Ʈ] : ���� ����.\n");

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