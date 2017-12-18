#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>
#include <memory>
#include <stdlib.h>
#include <tlHelp32.h>
#include <tchar.h>
#include <string>
#include "wtypes.h"
#pragma comment (lib, "ws2_32.lib")


using namespace std;

void main()
{
	// Initialiize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't initialize winsock! Quitting.." << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting..." << endl;
		return;
	}

	// Bind the socket to an ip address and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));


	// Tell winsock the socket is fr listening
	listen(listening, SOMAXCONN);

	// wait for a connectin
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];		// client's remote name
	char service[NI_MAXHOST];	// Service (ie.. port) the client is connect on

	ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " <<
			ntohs(client.sin_port) << endl;
	}

	// close listening socket
	closesocket(listening);

	// while loop: accept and echo message back to client
	char buf[4096];


	HWND hwnd = FindWindowA(NULL, "ManiaPlanet");
	DWORD entityBase = 0xAAFD7C;
	DWORD offset = 0x17B7CBC;

	DWORD imagebase = 0x00400000;

	// screen resolution for drawing the dot
	int horizontal = 0;
	int vertical = 0;
	GetDesktopResolution(horizontal, vertical);



	while (true)
	{
		ZeroMemory(buf, 4096);
		// wait tfor client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error in rec(). Quitting" << endl;
			break;
		}
		if (bytesReceived == 0)
		{
			cerr << "Client disconnected" << endl;
			break;
		}
		string answer = string(buf, 0, bytesReceived);
		if (answer == "playsound")
		{
			cout << "Client> " << answer << endl;
			PlaySound(TEXT("DING.wav"), NULL, SND_ASYNC);
		}

		send(clientSocket, buf, bytesReceived + 1, 0);

		// READ ADDRESS FROM TRACKMANIA
		///int dynAddress;
		//DWORD clientModuleBaseAddress = dwGetModuleBaseAddress(procID, _T("ManiaPlanet.exe"));
		//ReadProcessMemory(handle, (PBYTE)clientModuleBaseAddress + offset, &dynAddress, sizeof(int), 0);
		//string res = to_string(dynAddress);

		//cout << string(buf, 0, bytesReceived) << endl;
		//Sleep(20000);
		//send(clientSocket, buf, bytesReceived + 1, 0);

		// Send playsound command to client when car is at start
		//if (res.substr(res.length() - 2) == "17")
		//{
		//	//int sendResult = send(clientSocket, "playsound", 9, 0);
		//	cout << "CAR IS AT START" << endl;
		//}
		//else
		//{
		//	cout << "CAR IS ON THE TRACK" << endl;
		//}
	}

	// close the sock
	closesocket(clientSocket);
	// Cleanup winsock
	WSACleanup();

}
