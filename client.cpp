#pragma comment(lib,"Ws2_32.lib")
#include <Windows.h>
#include <iostream>   
#include <string>
#include <vector>

using namespace std;

void cmd_menu_list(int flag);
DWORD WINAPI RecvThread(LPVOID lpParam);
void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

int main()
{
	string Cmd, SendBuffer;
	vector<string> CmdList;
	HANDLE  hThread;
	DWORD   dwThreadId;

	WSADATA  Ws;
	SOCKET ClientSocket;
	int Ret = -1;
	int exist=0;

	struct sockaddr_in ServerAddr;
	
	
	while (true)
	{
		cmd_menu_list(exist);
		getline(cin, Cmd);
		CmdList.clear();
		SplitString(Cmd, CmdList, " ");
		if (CmdList[0] == "c")
		{
			if (exist == 1)
			{
				cout << "Connected Server" << endl;
				continue;
			}
			if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
			{
				cout << "Init Windows Socket Failed::" << GetLastError() << endl;
				continue;
			}
			if (CmdList.size() != 3)
			{
				cout << "Connect Error"<< endl;
				cout << "Usage:<ip-addr> <port>" << endl;
				continue;
			}
			ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			ServerAddr.sin_family = AF_INET;
			ServerAddr.sin_addr.s_addr = inet_addr(CmdList[1].c_str());
			ServerAddr.sin_port = htons(atoi(CmdList[2].c_str()));
			memset(ServerAddr.sin_zero, 0x00, 8);

			Ret = connect(ClientSocket, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
			if (Ret == SOCKET_ERROR)
			{
				cout << "Connect Error::" << GetLastError() << endl;
				continue;
			}
			else
			{
				cout << "Connect Successfully" << endl;
				exist = 1;
			}
			hThread = CreateThread(
				NULL,                   // default security attributes
				0,                      // use default stack size  
				RecvThread,			// thread function name
				(LPVOID)ClientSocket,          // argument to thread function 
				0,                      // use default creation flags 
				&dwThreadId);			// returns the thread identifier 
			CloseHandle(hThread);
		}
		else if (CmdList[0] == "d")
		{
			if (exist == 1)
			{
				closesocket(ClientSocket);
				WSACleanup();
				exist = 0;
				cout << "Disconnect from server successfully" << endl;
			}
			else
			{
				cout << "No connect to Server" << endl;
			}
		}
		else if (CmdList[0] == "t")
		{
			if (!exist)
			{
				cout << "No connect to Server" << endl;
				continue;
			}
			if (CmdList.size() != 1)
			{
				cout << "Connect Error" << endl;
				cout << "Usage:<t>" << endl;
				continue;
			}
			else
			{
				Ret = send(ClientSocket, Cmd.c_str(), Cmd.size(), 0);
				if (Ret == SOCKET_ERROR)
				{
					cout << "Send Info Error::" << GetLastError() << endl;
					continue;
				}
			}
		}
		else if (CmdList[0] == "n")
		{
			if (!exist)
			{
				cout << "No connect to Server" << endl;
				continue;
			}
			if (CmdList.size() != 1)
			{
				cout << "Connect Error" << endl;
				cout << "Usage:<n>" << endl;
				continue;
			}
			else
			{
				Ret = send(ClientSocket, Cmd.c_str(), Cmd.size(), 0);
				if (Ret == SOCKET_ERROR)
				{
					cout << "Send Info Error::" << GetLastError() << endl;
					continue;
				}
			}
		}
		else if (CmdList[0] == "l")
		{
			if (!exist)
			{
				cout << "No connect to Server" << endl;
				continue;
			}
			if (CmdList.size() != 1)
			{
				cout << "Connect Error" << endl;
				cout << "Usage:<l>" << endl;
				continue;
			}
			else
			{
				Ret = send(ClientSocket, Cmd.c_str(), Cmd.size(), 0);
				if (Ret == SOCKET_ERROR)
				{
					cout << "Send Info Error::" << GetLastError() << endl;
					continue;
				}
			}
		}
		else if (CmdList[0] == "s")
		{
			if (!exist)
			{
				cout << "No connect to Server" << endl;
				continue;
			}
			if (CmdList.size() != 3)
			{
				cout << "Connect Error" << endl;
				cout << "Usage:<s><ip><msg>" << endl;
				continue;
			}
			Ret = send(ClientSocket, Cmd.c_str(), Cmd.size(), 0);
			if (Ret == SOCKET_ERROR)
			{
				cout << "Send Info Error::" << GetLastError() << endl;
				continue;
			}
		}
		else if (CmdList[0] == "e")
		{
			if (exist == 1)
			{ 
				closesocket(ClientSocket);
				WSACleanup();
			}
			exit(0);
		}
		else
		{
			cout << "Error cmd" << endl;
		}
		cout << endl << endl ;
		Sleep(200);
	}
}

void cmd_menu_list(int flag)
{
	cout << "menu help:" << endl;
	cout << "choose following character as cmd" << endl;
	if(!flag)
		cout << "<c> connect to a server" << endl;
	if (flag)
	{
		cout << "<d> disconnect from the server" << endl;
		cout << "<t> receive server current time" << endl;
		cout << "<n> receive server name" << endl;
		cout << "<l> receive active connection list of server" << endl;
		cout << "<s> send message to specific client" << endl;
	}
	cout << "<e> exit" << endl;
}

void close_thread(HANDLE&  hThreadArray)
{
	CloseHandle(hThreadArray);
}

DWORD WINAPI RecvThread(LPVOID lpParam)
{
	SOCKET CientSocket = (SOCKET)lpParam;
	int Ret = 0;
	char RecvBuffer[MAX_PATH];
	while (true)
	{
		memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
		Ret = recv(CientSocket, RecvBuffer, MAX_PATH, 0);
		if (Ret == 0 || Ret == SOCKET_ERROR)
		{
			cout << "server exit" << endl;
			break;
		}
		cout << "from server:" << RecvBuffer << endl;
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