#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>

using namespace std;

int main()
{
	//Объявление переменных
	WSADATA wsaData;
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocket = INVALID_SOCKET;

	const char* sendBuffer = "The message has been delivered";

	char recvBuffer[512];

	int result;

	// Инициализация сокетов
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		cout << "WSAStartup faild, result = " << result << endl;
		return 1;
	}

	// Настройка hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	
	// Связываем сокет с адресом
	result = getaddrinfo(NULL, "500", &hints, &addrResult);
	if (result != 0) {
		cout << "getaddrinfo faild, result = " << result << endl;
		WSACleanup();
		return 2;
	}
	// Cоздание сокета
	ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "Socket creation faild" << endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 3;
	}

	// Переводим сокет в режим прослушивания
	result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		cout << "Binding soket faild";
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 4;
	}

	// Ожидаем подключения клиента
	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		cout << "Listening soket faild";
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 5;
	}

	// Cоздаем сокет для Клиента
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "Accepting socket faild" << endl;
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);

	ZeroMemory(recvBuffer, 512);
	do
	{
		result = recv(ClientSocket, recvBuffer, 512, 0);
		if (result > 0) {
			cout << "Receveid " << result << "bytes" << endl;
			cout << "Receveid data: " << recvBuffer << endl;

			result = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
			if (result == SOCKET_ERROR){
				cout << "Failed to send data back";
				closesocket(ClientSocket);
				freeaddrinfo(addrResult);
				WSACleanup();
				return 1;
			}

			FILE* file;
			if (fopen_s(&file, "test.txt", "wb")!=0) {
				std::cerr << "Failed to create file\n";
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

			char buffer[1024];
			int bytesReceived;
			do {
				bytesReceived = recv(ClientSocket, buffer, sizeof(buffer), 0);
				if (bytesReceived > 0) {
					fwrite(buffer, 1, bytesReceived, file);
				}
			} while (bytesReceived > 0);

			// Close file
			fclose(file);
		}
		else if (result == 0) {
			cout << "Connection closing..." << endl;
		}
		else {
			cout << "recv faild with error" << endl;
			closesocket(ClientSocket);
			freeaddrinfo(addrResult);
			WSACleanup();
			return 1;
		}
	} while (result > 0);

	result = shutdown(ClientSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		cout << "shutdown client socket faild" << endl;
		closesocket(ClientSocket);
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	freeaddrinfo(addrResult);
	WSACleanup();
	return 0;
}