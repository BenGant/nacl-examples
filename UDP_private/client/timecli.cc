#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv) {
	int retval;
	WSADATA wsaData;
	if ((retval = WSAStartup(0x202, &wsaData)) != 0) {
		fprintf(stderr,"Server: WSAStartup() failed with error %d\n", retval);
		WSACleanup();
		return -1;
	}
	
	int sockfdd;
	int n;
	struct sockaddr_in servaddr,cliaddr;
	char sendline[1000];
	char recvline[1000];
	
	int so_broadcast;
	sockfdd=socket(AF_INET,SOCK_DGRAM,0);

#ifdef BCAST
	retval = setsockopt(sockfdd, SOL_SOCKET, SO_BROADCAST, (const char*)&so_broadcast, sizeof(so_broadcast));

	if ( retval ) {
		printf("retval = %d\n", WSAGetLastError());
	}
#endif
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servaddr.sin_port=htons(32000);

	while (fgets(sendline, 10000,stdin) != NULL)
	{
		n = sendto(sockfdd,sendline,strlen(sendline),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
		printf("retval = %d\n", WSAGetLastError());
		memset(&sendline[0], 0, sizeof(sendline));
		memset(&recvline[0], 0, sizeof(recvline));
		int rlen = sizeof(servaddr);
		n=recvfrom(sockfdd,recvline,10000,0,(struct sockaddr *)&servaddr,&rlen);
		printf("retval = %d\n", WSAGetLastError());
		recvline[n]=0;
		fputs(recvline,stdout);
	}

	return 0;
}