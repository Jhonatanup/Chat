#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <time.h>
#include <map>

#pragma comment (lib, "Ws2_32.lib")

#define PORT "40000"
#define VER "1"
#define VERSION 1
#define TYPE 3
#define FILL_CHAR ' '
#define FROM 20
#define TO  20
#define TAMANHO 5
#define ALL_LEN 20049
#define MESSAGE 20000

//void criaConexao(void *arg);
void gerenciaConexao(void *arg);

using namespace std;

map<string, SOCKET> listaClientes;//map para relacionar nickname com SOCKET

const string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d-%m-%Y %X", &tstruct);

	return buf;
}

typedef struct parametros {
	SOCKET ListenSocket;
	SOCKET ClienteSocket;
}PARAM;

int main() {
	WSADATA wsaData;
	int iResult;
	PARAM par;
	HANDLE Thread;
	DWORD TName;

	//inicializando o Winsock indicando a versão do Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//MAKEWORD(2,2) indica qual versão do Winsock está sendo utilizada
	if (iResult != 0) {
		printf("WSAStartup falhou. Erro %d\n", iResult);
		return 1;
	}

	//Criando o Winsock no servidor

	struct addrinfo hints;
	struct addrinfo *result = NULL;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //especifica que o endereçamento IP é IPV4
	hints.ai_socktype = SOCK_STREAM; //especifica o socket stream
	hints.ai_protocol = IPPROTO_TCP; //especifica o protocolo da camada de transporte
	hints.ai_flags = AI_PASSIVE;

	//Define o endereço e a porta do servidor
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("Gettaddrinfo falhou. Erro %d\n", iResult);
		WSACleanup();
		return 1;
	}


	//Cria o socket para ser usado no servidor
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClienteSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Falha na criacao do socket. Erro: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Bind falhou. Erro: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	par.ListenSocket = ListenSocket;

	freeaddrinfo(result);

	//LISTEN

	int lResult;
	//HANDLE Thread;
	//DWORD TName;

	printf("Aguardando conexoes...\n\n");
	while (true) {
		lResult = listen(ListenSocket, SOMAXCONN);
		if (lResult == SOCKET_ERROR) {
			printf("Listen falhou. Erro: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			system("PAUSE");
			break;
		}

		ClienteSocket = accept(ListenSocket, NULL, NULL);
		printf("\nConexao iniciada.\n");

		if (ClienteSocket == INVALID_SOCKET) {
			printf("Accept falhou. Erro %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			system("PAUSE");
			break;
		}
		//CHAMA THREAD
		Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)gerenciaConexao, &ClienteSocket, 0, &TName);
	}

	


	//Chama a funcao para ouvir passando como parametro o comprimento máximo da fila.
	//Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)criaConexao, &par, 0, &TName);

	//WaitForSingleObject(Thread, INFINITE);
	return 0;
}

void gerenciaConexao(void *arg) {
	SOCKET *sock = (SOCKET*)arg;
	SOCKET ClientSocket = *sock;

	string sendBuffer(ALL_LEN, FILL_CHAR);
	string version(VERSION, FILL_CHAR);
	string type(TYPE, FILL_CHAR);
	string to(TO, FILL_CHAR);
	string from(FROM, FILL_CHAR);
	string tamanho(TAMANHO, FILL_CHAR);
	string message(MESSAGE, FILL_CHAR);
	string aux;
	string recvBuffer;
	string recvVersion;
	string recvType;
	string recvTo;
	string recvFrom;
	string recvTamanho;//Msglen
	string recvMessage;
	char recvBuff[ALL_LEN] = {};
	char sendBuff[ALL_LEN] = {};

	int iResult = recv(ClientSocket, recvBuff, ALL_LEN, 0);
	if (iResult <= 0) {
		printf("Receive falhou. Erro: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		system("PAUSE");
		_endthread();
	}

	recvBuffer = recvBuff;

	recvVersion = recvBuffer.substr(0, VERSION);
	recvType = recvBuffer.substr(VERSION, TYPE);
	recvFrom = recvBuffer.substr(VERSION + TYPE, FROM);
	//recvTo = recvBuffer.substr(VERSION + TYPE + FROM, TO);
	//recvTamanho = recvBuffer.substr(VERSION + TYPE + TO + FROM, TAMANHO);
	//recvMessage = recvBuffer.substr(VERSION + TYPE + TO + FROM + TAMANHO, recvBuffer.length()-VERSION - TYPE - TO - FROM - TAMANHO-1);
	
	printf("\nServidor - Type recebida: %s", recvType.c_str());
	printf("\nServidor - Nickname recebido: %s", recvFrom.c_str());


	//REG
	if (recvType == "REG") {
		version = "1";
		type = "ACK";
		message = "True";
		
		if (message.size() < MESSAGE) {
			for (int i = message.size(); i <= MESSAGE; i++)
				message += FILL_CHAR;
		}
		sendBuffer = version + type + from + to + tamanho + message;
		printf("\n %s", sendBuffer);
		iResult = send(ClientSocket, sendBuffer.c_str(), ALL_LEN, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Send falhou: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			_endthread();
		}
			
	}
}
		
	

	//printf("\nServidor - Comando recebido: %s", recvCommand.c_str());
	/*
	version.replace(0, VERSION, VER);
	command.replace(0, COMMAND, recvCommand);

	if (recvCommand.find("GET_H ") != string::npos) {
		message = "\nHora: ";
		message += currentDateTime().substr(10, 19);
	}
	else if (recvCommand.find("GET_D ") != string::npos) {
		message = "\nData: ";
		message += currentDateTime().substr(0, 10);
	}
	else if (recvCommand.find("GET_DH") != string::npos || recvCommand.find("GET_HD") != string::npos) {
		message = "\nData e hora: ";
		message += currentDateTime();
	}
	else {
		message = "\nComando invalido ";
	}

	if (message.size() < MESSAGE) {
		for (int i = message.size(); i <= MESSAGE; i++)
			message += FILL_CHAR;
	}

	sendBuffer.replace(0, VERSION, version);
	sendBuffer.replace(VERSION, VERSION + COMMAND, command);
	sendBuffer.replace(VERSION + COMMAND, VERSION + COMMAND + MESSAGE, message);


	iResult = send(ClientSocket, sendBuffer.c_str(), ALL_LEN, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Send falhou: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		_endthread();
	}

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("Shutdown falhou: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		_endthread();
	}

	closesocket(ClientSocket);
	printf("\n\nConexao finalizada.");
	*/
