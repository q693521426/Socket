
#pragma comment(lib,"Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>

using namespace std;

#define  PORT 2117
#define  IP_ADDRESS "127.0.0.1"
#define  MAX_CLIENT_SIZE 1000

struct ClientInfo
{
	size_t  index;
	SOCKET Socket;
	struct sockaddr_in Addr;
}ClientArray[MAX_CLIENT_SIZE], ServerInfo;

static size_t ClientIndex = 0;
CRITICAL_SECTION CriticalSection;

DWORD WINAPI ClientThread(LPVOID lpParameter);
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

int main(int argc, char* argv[])
{
	WSADATA  Ws;
	int Ret = 0;
	int AddrLen = 0;
	HANDLE hThread[MAX_CLIENT_SIZE];

	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		cout << "Init Windows Socket Failed::" << GetLastError() << endl;
		return -1;
	}

	//Create Socket
	ServerInfo.Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerInfo.Socket == INVALID_SOCKET)
	{
		cout << "Create Socket Failed::" << GetLastError() << endl;
		return -1;
	}

	ServerInfo.Addr.sin_family = AF_INET;
	ServerInfo.Addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	ServerInfo.Addr.sin_port = htons(PORT);
	memset(ServerInfo.Addr.sin_zero, 0x00, 8);

	//Bind Socket
	Ret = bind(ServerInfo.Socket, (struct sockaddr*)&ServerInfo.Addr, sizeof(ServerInfo.Addr));
	if (Ret != 0)
	{
		cout << "Bind Socket Failed::" << GetLastError() << endl;
		return -1;
	}

	Ret = listen(ServerInfo.Socket, 10);
	if (Ret != 0)
	{
		cout << "listen Socket Failed::" << GetLastError() << endl;
		return -1;
	}

	cout << "Server start......127.0.0.1:2117" << endl;

	while (true)
	{
		ClientArray[ClientIndex].index = ClientIndex;
		AddrLen = sizeof(ClientArray[ClientIndex].Addr);
		ClientArray[ClientIndex].Socket = accept(ServerInfo.Socket, (struct sockaddr*)&ClientArray[ClientIndex].Addr, &AddrLen);
		if (ClientArray[ClientIndex].Socket == INVALID_SOCKET)
		{
			cout << "Accept Failed::" << GetLastError() << endl;
			break;
		}

		cout << "client connect::" << inet_ntoa(ClientArray[ClientIndex].Addr.sin_addr) << ":" << ClientArray[ClientIndex].Addr.sin_port << endl;
		
		ClientIndex++;
		hThread[ClientIndex-1] = CreateThread(NULL, 0, ClientThread, (LPVOID)&(ClientArray[ClientIndex-1]), 0, NULL);
		if (hThread[ClientIndex-1] == NULL)
		{
			cout << "Create Thread Failed!" << endl;
			break;
		}

		CloseHandle(hThread[ClientIndex-1]);
	}

	closesocket(ServerInfo.Socket);
	for(size_t i=0;i<=ClientIndex;++i)
		closesocket(ClientArray[i].Socket);
	WSACleanup();

	return 0;
}

DWORD WINAPI ClientThread(LPVOID lpParameter)
{
	ClientInfo* ClientInfo = (struct ClientInfo*)lpParameter;
	int Ret = 0;
	char RecvBuffer[MAX_PATH];
	string Recv,SendBuffer;
	vector<string> RecvVec;

	while (true)
	{
		memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
		Ret = recv(ClientInfo->Socket, RecvBuffer, MAX_PATH, 0);
		Recv = RecvBuffer;
		if (Ret == 0 || Ret == SOCKET_ERROR)
		{
			cout << "client exit ::" <<
				inet_ntoa(ClientInfo->Addr.sin_addr) << ":" <<
				ClientInfo->Addr.sin_port << endl;
			break;
		}
		RecvVec.clear();
		SplitString(Recv, RecvVec, " ");
		if (RecvVec[0] == "t")
		{
			time_t t = time(0);
			char tmp[64];
			strftime(tmp, sizeof(tmp), "%Y/%m/%d %X %A", localtime(&t));
			cout << "request for time ::" << tmp << endl;
			Ret = send(ClientInfo->Socket, tmp, sizeof(tmp), 0);
			if (Ret == SOCKET_ERROR)
			{
				cout << "Send Info Error::" << GetLastError() << endl;
				continue;
			}
		}
		else if (RecvVec[0] == "n")
		{
			stringstream ss;
			string port;
			ss << ServerInfo.Addr.sin_port;
			ss >> port;
			SendBuffer = "server ::" + string(inet_ntoa(ServerInfo.Addr.sin_addr))
				+ ":" + port;
			cout << "request for name ::" << SendBuffer <<endl;
			Ret = send(ClientInfo->Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
			if (Ret == SOCKET_ERROR)
			{
				cout << "Send Info Error::" << GetLastError() << endl;
				continue;
			}
		}
		else if (RecvVec[0] == "l")
		{
			SendBuffer = "client_list ::\n";
			cout << "request for client_list ::"<<endl ;
			for (int i=0; i < ClientIndex; ++i)
			{
				stringstream ss;
				string port;
				ss << ClientArray[i].Addr.sin_port;
				ss >> port;
				SendBuffer = SendBuffer + "\tclient ::" + string(inet_ntoa(ClientArray[i].Addr.sin_addr))
					+ ":" + port;
			}
			cout << SendBuffer << endl;
			Ret = send(ClientInfo->Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
			if (Ret == SOCKET_ERROR)
			{
				cout << "Send Info Error::" << GetLastError() << endl;
				continue;
			}
		}
		else if (RecvVec[0] == "s")
		{
			int send_index = atoi(RecvVec[1].c_str());
			if (send_index >= ClientIndex)
			{
				stringstream ss;
				string index;
				ss << send_index;
				ss >> index;
				SendBuffer = "there is no client id ::" + index;
				Ret = send(ClientInfo->Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
				if (Ret == SOCKET_ERROR)
				{
					cout << "Send Info Error::" << GetLastError() << endl;
					continue;
				}
			}
			else
			{
				stringstream ss;
				string port;
				ss << ClientInfo->Addr.sin_port;
				ss >> port;
				SendBuffer = "msg ::" + RecvVec[2] +
					"\n\tfrom client ::" + string(inet_ntoa(ClientInfo->Addr.sin_addr))
					+ ":" + port;
				cout << "send msg ::" << RecvVec[2] << endl <<
					"\tfrom client ::" << SendBuffer << endl
					<< "\tto client ::" << inet_ntoa(ClientArray[send_index].Addr.sin_addr)
					<< ":" << ClientArray[send_index].Addr.sin_port << endl;
				Ret = send(ClientArray[send_index].Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
				if (Ret == SOCKET_ERROR)
				{
					cout << "Send Info Error::" << GetLastError() << endl;
					SendBuffer = "failed";
					Ret = send(ClientInfo->Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
					continue;
				}
				SendBuffer = "successfully";
				Ret = send(ClientArray[send_index].Socket, SendBuffer.c_str(), SendBuffer.size(), 0);
			}
		}
	}

	return 0;
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}