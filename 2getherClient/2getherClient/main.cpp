#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>
#include <memory>
#include <stdlib.h>
#include <tlHelp32.h>
#include <tchar.h>
#include "wtypes.h"
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define DEFAULT_PORT "54000"
#define DEFAULT_BUFLEN 4096            
#define IP_ADDRESS "178.117.24.150" // Warning: This is NOT my ip ..

DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, TCHAR *szModuleName);
string str_last_word(const string &str);



struct client_type
{
	SOCKET socket;
	int id;
	char received_message[DEFAULT_BUFLEN];
};

int process_client(client_type &new_client);
int main();

int process_client(client_type &new_client)
{
	while (1)
	{
		memset(new_client.received_message, 0, DEFAULT_BUFLEN);

		if (new_client.socket != 0)
		{
			int iResult = recv(new_client.socket, new_client.received_message, DEFAULT_BUFLEN, 0);
			

			if (iResult != SOCKET_ERROR)
			{
				string answer = new_client.received_message;
				//cout << answer << endl;
				std::string str;
				const char * c = str.c_str();
				cout << answer << endl; 

				answer = str_last_word(answer);

				if (strcmp(answer.c_str(), "cmd:playsound\r\n") == 0)
				{
					PlaySound(TEXT("DING.wav"), NULL, SND_ASYNC);
				}
			}
			else
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}
	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected" << endl;

	return 0;
}


int main()
{
	WSAData wsa_data;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	string sent_message = "";
	client_type client = { INVALID_SOCKET, -1, "" };
	int iResult = 0;
	string message;

	HWND hwnd = FindWindowA(NULL, "ManiaPlanet");
	DWORD entityBase = 0xAAFD7C;
	DWORD offset = 0x17B7CBC;

	DWORD imagebase = 0x00400000;

	// hook to ManiaPlanet process
	DWORD procID;
	GetWindowThreadProcessId(hwnd, &procID);
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

	while (hwnd == NULL)
	{
		cout << "Please start trackmania first" << endl;
		Sleep(3000);
		hwnd = FindWindowA(NULL, "ManiaPlanet");
	}

	while (procID == NULL)
	{
		cout << "Can't find process" << endl;
		Sleep(3000);
		GetWindowThreadProcessId(hwnd, &procID);
		handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
	}

	

	cout << "Starting Client...\n";

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (iResult != 0) {
		cout << "WSAStartup() failed with error: " << iResult << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	cout << "Connecting...\n";

	// Resolve the server address and port
	iResult = getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo() failed with error: " << iResult << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		client.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client.socket == INVALID_SOCKET) {
			cout << "socket() failed with error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// Connect to server.
		iResult = connect(client.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(client.socket);
			client.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (client.socket == INVALID_SOCKET) {
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Successfully Connected" << endl;

	//Obtain id from server for this client;
	recv(client.socket, client.received_message, DEFAULT_BUFLEN, 0);
	message = client.received_message;

	DWORD clientModuleBaseAddress = dwGetModuleBaseAddress(procID, _T("ManiaPlanet.exe"));

	if (str_last_word(message) != "Server is full")
	{
		client.id = atoi(client.received_message);

		thread my_thread(process_client, client);

		while (1)
		{
			
			// READ ADDRESS FROM TRACKMANIA
			int dynAddress;
			ReadProcessMemory(handle, (PBYTE)clientModuleBaseAddress + offset, &dynAddress, sizeof(int), 0);

			string res = to_string(dynAddress);
			string sent_message = "cmd:playsound";

			if (res.substr(res.length() - 2) == "17") {
				///iResult = send(client.socket, command.c_str(), strlen(command.c_str()), 0);
				cout << "Sending my position to other clients..." << endl;
				iResult = send(client.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);
			}
			else {
				iResult = 1;
			}

			if (iResult <= 0)
			{
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
			Sleep(500);

		}
		//Shutdown the connection since no more data will be sent
		my_thread.detach();
	}
	else
		cout << client.received_message << endl;

	cout << "Shutting down socket..." << endl;
	iResult = shutdown(client.socket, SD_SEND);
	if (iResult == SOCKET_ERROR) 
	{
		cout << "shutdown() failed with error: " << WSAGetLastError() << endl;
		closesocket(client.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(client.socket);
	WSACleanup();
	system("pause");
	return 0;
}

string str_last_word(const string &str) 
{
	size_t index = str.find_last_of(" ");
	if (index != string::npos) {
		return str.substr(index + 1, str.length());
	}
	else
		return "";
}

DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, TCHAR *szModuleName)
{
	DWORD_PTR dwModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_tcsicmp(ModuleEntry32.szModule, szModuleName) == 0)
				{
					dwModuleBaseAddress = (DWORD_PTR)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		CloseHandle(hSnapshot);
	}
	return dwModuleBaseAddress;
}