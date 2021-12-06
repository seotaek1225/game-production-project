#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <iostream>
#include <tchar.h>
#include <locale.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <fstream>
#include <string>


#define SERVERPORT 9000
#define BUFSIZE 512
#define WM_SOCKET (WM_USER+1)

struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvBytes;
	int sendBytes;
	BOOL recvdelayed;
	SOCKETINFO* next;
};

void err_quit(const char *msg);
void err_display(const char *msg);

SOCKETINFO* socketInfoList;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

BOOL AddSocketInfo(SOCKET sock);
SOCKETINFO* GetSocketInfo(SOCKET sock);
void RemoveSocketInfo(SOCKET sock);

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

typedef struct PersonalInfomation
{
	char id[20];
	char password[20];
}PersonalInfo;

int main()
{
	


	int retval;

	WNDCLASS wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = NULL;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = TEXT("wndclass");
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wndclass))
		return 1;

	HWND hWnd = CreateWindow(
		TEXT("wndclass"),
		TEXT("tcp server"),
		WS_OVERLAPPEDWINDOW,
		0, 0,
		0, 0,
		NULL,
		NULL,
		NULL,
		NULL);
	if (hWnd == NULL)
		return 0;

	WSADATA wsa;
	retval = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (retval != 0) {
		err_quit("WSAStartup()");
	}


	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		err_quit("listen_sock()");
	}


	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
	}

	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR) {
		err_quit("WSAAyncSelect()");
	}

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SOCKET:
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	SOCKETINFO* ptr;
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrlen, retval;
	TCHAR buf[16];
	//Position pos;
	Com com;
	Packet packet;
	std::ifstream idFile;
	std::ofstream connectFile;
	std::ofstream recvFile;
	int len;
	/*char *strArr[2] = { NULL, };
	int i = 0;*/
	/*char userId[20] = {};
	char userPassword[20] = {};*/
	PersonalInfo p[10];
	strcpy(p[0].id, "asdf");
	strcpy(p[0].password, "123123");
	strcpy(p[1].id, "asdfasdf");
	strcpy(p[1].password, "321321");

	if (WSAGETSELECTERROR(lParam))
	{
		_tprintf_s(TEXT("WSAGETSELECTERROR %d\n"), lParam);
		RemoveSocketInfo(wParam);
		return;
	}

	switch (lParam)
	{
	case FD_ACCEPT:
		addrlen = sizeof(clientAddr);
		client_sock = accept(wParam, (SOCKADDR*)&clientAddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display(_T("Accept()"));
			return;
		}

		inet_ntop(AF_INET, &clientAddr.sin_addr, buf, (sizeof(buf) / sizeof(buf[0])));

		_tprintf_s(TEXT("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n"),
			buf, ntohs(clientAddr.sin_port));
		AddSocketInfo(client_sock);


		connectFile.open("connect.txt", std::ios::app);
		if (connectFile.fail()) {
			printf("Error: file open fail\n");
		}

		//connectFile << inet_ntoa(clientAddr.sin_addr) << " " << ntohs(clientAddr.sin_port) << " connect " << std::endl;
		connectFile.close();

		retval = WSAAsyncSelect(client_sock, hWnd,
			WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
		if (retval == SOCKET_ERROR)
		{
			_tprintf_s(TEXT("error client_sock WSAAsyncSelect()\n"));
			RemoveSocketInfo(client_sock);
		}
		break;

	case FD_READ:
		ptr = GetSocketInfo(wParam);
		if (ptr->recvBytes > 0)
		{
			ptr->recvdelayed = TRUE;
			return;
		}

		retval = recv(wParam, (char*)ptr->buf, sizeof(packet), 0);
		if (retval == SOCKET_ERROR)
		{
			_tprintf_s(TEXT("error recv \n"));
			RemoveSocketInfo(wParam);
			return;
		}
		printf("recv = %d\n", retval);
		ptr->recvBytes = retval;

		// 받은 데이터 출력
		ptr->buf[retval] = '\0';
		/*Com *temp;
		temp = (Com*)ptr->buf;*/
		Packet *temp;
		temp = (Packet*)ptr->buf;


		addrlen = sizeof(clientAddr);
		getpeername(wParam, (SOCKADDR*)&clientAddr, &addrlen);

		inet_ntop(AF_INET, &clientAddr.sin_addr, buf, (sizeof(buf) / sizeof(buf[0])));
		_tprintf_s(TEXT("[TCP/%s:%d] "), buf, ntohs(clientAddr.sin_port));
		//printf_s("%s\n", ptr->buf);
		if (temp->login == false)
		{
			for (int i = 0; i < 10; i++)
			{
				if (strcmp(temp->id, p[i].id) != 0)
				{
					continue;
				}
				else
				{
					if (strcmp(temp->password, p[i].password) != 0)
					{
						continue;
					}
					else
					{	
						temp->login = true;
						break;	
					}
				}
			}
			//printf("login false\n");
			//idFile.open("password.csv");
			/*if (idFile.fail()) {
				printf("Error: file open fail\n");
			}*/
			//printf("open file\n");
			/*if (idFile.is_open())
			{
				printf("open file success\n");
				while (!idFile.eof())
				{
					printf("i = %d\n", i);
					printf("find id\n");
					printf("id : %s, pw : %s\n", temp->id, temp->password);
					std::string str;
					getline(idFile, str);
					char arr[100];
					strcpy(arr, str.c_str());
					printf("%s\n", arr);
					char *strTemp = strtok(arr, ",");


					while (strTemp != NULL)
					{

						printf("%s\n", strTemp);
						if (i == 0)
						{
							strcpy(userId, strTemp);
							printf("%s\n", userId);

						}
						else
						{
							strcpy(userPassword, strTemp);
							printf("%s\n", userPassword);
						}

						strTemp = strtok(NULL, ",");

					}
					printf("com size = %d, size = %d\n", sizeof(temp->id), sizeof(userId));
					printf("same id %d\n", strcmp(temp->id, userId));
					printf("same pw %d\n", strcmp(temp->password, userPassword));
					printf("%s %s %d %d\n", temp->id, userId, strlen(temp->id), strlen(userId));

					if (strcmp(temp->id, userId) == 0)
					{
						if (strcmp(temp->password, userPassword) == 0)
						{
							temp->login = true;
							break;
						}
					}
					i++;
					if (i == 2)
						i = 0;
				}
			}
			idFile.close();*/

		}
		else
		{
			if (temp->x < 10)
			{
				temp->x++;
			}
			else
			{
				temp->x = 10;
			}
			printf("x = %d y = %d\n", temp->x, temp->y);


			recvFile.open("recv.txt", std::ios::app);
			if (recvFile.fail()) {
				printf("Error: file open fail\n");
			}
			//recvFile << ntohs(clientAddr.sin_port) << " " << ptr->buf << std::endl;
			recvFile.close();
		}
		temp->portNum = ntohs(clientAddr.sin_port);
		strcpy(temp->address, buf);
		strcpy(ptr->buf, (char*)temp);


	case FD_WRITE:
		ptr = GetSocketInfo(wParam);
		if (ptr->recvBytes <= ptr->sendBytes)
			return;


		retval = send(wParam, (char*)ptr->buf + ptr->sendBytes,
			ptr->recvBytes - ptr->sendBytes, 0);
		printf("send = %d\n", retval);
		if (retval == SOCKET_ERROR)
		{
			_tprintf_s(TEXT("error send() SOCKET_ERROR\n"));
			RemoveSocketInfo(wParam);
			return;
		}
		ptr->sendBytes += retval;


		// 받은 데이터를 모두 보냈는지 체크
		if (ptr->recvBytes == ptr->sendBytes)
		{
			ptr->recvBytes = 0;
			ptr->sendBytes = 0;
			if (ptr->recvdelayed)
			{
				ptr->recvdelayed = FALSE;
				PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
			}
		}
		break;

	case FD_CLOSE:

		RemoveSocketInfo(wParam);
		break;
	}
}

BOOL AddSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		_tprintf_s(TEXT("[오류] 메모리가 부족합니다!\n"));
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvBytes = 0;
	ptr->sendBytes = 0;
	ptr->recvdelayed = FALSE;
	ptr->next = socketInfoList;
	socketInfoList = ptr;

	return TRUE;
}

SOCKETINFO* GetSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = socketInfoList;
	while (ptr)
	{
		if (ptr->sock == sock)
			return ptr;
		ptr = ptr->next;
	}

	return NULL;
}

void RemoveSocketInfo(SOCKET sock)
{
	SOCKADDR_IN clientAddr;
	std::ofstream connectFile;
	int addrlen = sizeof(clientAddr);
	getpeername(sock, (SOCKADDR*)&clientAddr, &addrlen);
	TCHAR buf[16];
	inet_ntop(AF_INET, &clientAddr.sin_addr, buf, (sizeof(buf) / sizeof(buf[0])));
	_tprintf_s(TEXT("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n"),
		buf, ntohs(clientAddr.sin_port));
	connectFile.open("connect.txt", std::ios::app);

	connectFile << buf << " " << ntohs(clientAddr.sin_port) << " disconnect " << std::endl;
	connectFile.close();
	SOCKETINFO* cur = socketInfoList;
	SOCKETINFO* prev = NULL;
	while (cur)
	{
		if (cur->sock == sock)
		{
			if (prev)
				prev->next = cur->next;
			else
				socketInfoList = cur->next;

			closesocket(cur->sock);
			delete cur;
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}