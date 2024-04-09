#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <cstdio>

using namespace std;

int main()
{
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ConnectSocket = INVALID_SOCKET;


	const char* sendBuffer = "Hello from Client!";
	string message;
	size_t BytesRead;

	char recvBuffer[512];
	char Buffer[1024];

	int result;
	// инициализация сокетов
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		cout << "WSAStartup faild, result = " << result << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	result = getaddrinfo("localhost", "500", &hints, &addrResult);
	if (result != 0) {
		cout << "getaddrinfo faild, result = " << result << endl;
		WSACleanup();
		return 2;
	}

	ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Socket creation faild" << endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 3;
	}

	result = connect(ConnectSocket, addrResult->ai_addr, (int) addrResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		cout << "Unable connect to server";
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 4;
	}

	cout << "Message: " << endl;
	getline(cin, message);
	
	result = send(ConnectSocket,message.c_str(), (int)strlen(sendBuffer), 0);
	if (result == SOCKET_ERROR) {
		cout << "send faild, error" << result << endl;
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 5;
	}

	cout << "Bytes send: " << result << "bytes" << endl;

	FILE* file;
	if (fopen_s(&file, "test.txt", "rb") != 0){
		cout << "Failed to open file\n";
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	while ((BytesRead = fread(Buffer, 1, sizeof(Buffer), file)) > 0) {
		send(ConnectSocket, Buffer, static_cast<int>(BytesRead), 0);
	}

	// Close file
	fclose(file);

	result = shutdown(ConnectSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		cout << "shutdown, error" << result << endl;
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 6;
	}

	ZeroMemory(recvBuffer, 512);
	do 
	{

		result = recv(ConnectSocket, recvBuffer, 512, 0);
		if (result > 0) {
			cout << "Receveid " << result << "bytes" << endl;
			cout << "Receveid data: " << recvBuffer << endl;
		}
		else if (result == 0) {
			cout << "Connection closed" << endl;
		}
		else {
			cout << "recv faild with error" << endl;
		}
	} while (result > 0);

	closesocket(ConnectSocket);
	freeaddrinfo(addrResult);
	WSACleanup();
	return 0;
}

