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

	int sockfd,n;
	struct sockaddr_in servaddr,cliaddr;
	int len;
	char mesg[1000];

	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(32000);
	retval = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if (retval != 0) {
		fprintf(stderr, "Could not bind port %d\n", retval);
		return -1;
	}

	printf("Bound %d\n", servaddr.sin_addr);

	for (;;)
	{
		len = sizeof(cliaddr);
		n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
		sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
		printf("-------------------------------------------------------\n");
		mesg[n] = 0;
		printf("Received the following:\n");
		printf("%s",mesg);
		printf("-------------------------------------------------------\n");
	}

	return 0;
}