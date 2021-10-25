#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512


typedef struct inpormation
{
	int m_share;
	int m_port;
}Inpo;


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


int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;


	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");


	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	char buf[BUFSIZE + 1];
	int len;
	int menu;
	int inpo_len;
	char input[BUFSIZE + 1];
	int client_share;

	while (1) {
		printf("메뉴를 선택해주세요 \n");

		printf("1. 읽기 2. 쓰기 3. 종료\n");
		scanf("%d", &menu);
		if (menu != 1 && menu != 2 && menu != 3)
		{
			printf("메뉴에 있는 번호를 선택해주세요 \n");
			continue;
		}

		else if (menu == 1)
		{
			itoa(menu, buf, 10);
			retval = send(sock, buf, strlen(buf), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}
			retval = recvn(sock, (char*)&inpo_len, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			retval = recvn(sock, buf, inpo_len, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;
			buf[retval] = '\0';

			Inpo *temp;
			temp = (Inpo*)buf;


			printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);

			printf("[받은 데이터]  %d share = %d\n", temp->m_port, temp->m_share);

		}
		else if (menu == 2)
		{

			itoa(menu, buf, 10);
			retval = send(sock, buf, strlen(buf), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}
			printf("[보낼 데이터] : ");

			while (getchar() != '\n');

			scanf("%d", &client_share);
			len = strlen(input);
			if (input[len - 1] == '\n')
				input[len - 1] = '\0';
			if (strlen(input) == 0)
				break;

			itoa(client_share, input, 10);

			retval = send(sock, input, strlen(input), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
			printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

			retval = recvn(sock, (char*)&inpo_len, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			retval = recvn(sock, buf, inpo_len, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;
			buf[retval] = '\0';

			Inpo *temp;
			temp = (Inpo*)buf;

			printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);

			printf("[받은 데이터]  %d share = %d\n", temp->m_port, temp->m_share);
		}
		else if (menu == 3)
		{
			itoa(menu, buf, 10);
			retval = send(sock, buf, strlen(buf), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("send()");
				break;
			}

			retval = recvn(sock, (char*)&inpo_len, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			retval = recvn(sock, buf, inpo_len, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			break;
		}
	}
	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}