#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#pragma warning(disable : 4996)
extern int errno;

#define MAXLINE 1024

int echo_process(int echo_fd)
{ 
	int len;
	char buf[MAXLINE]; 
	while((len = read(echo_fd, buf, sizeof(buf))) != 0)
	{ 
		if (len < 0)
		{ 
			printf("echo: 읽기 에러 - %s\n", strerror(errno)); 
			return (-1); 
		} 
		if(write(echo_fd, buf, len) < 0)
		{ 
			printf("echo: 읽기 에러 - %s\n", strerror(errno)); 
			return (-1); 
		} 
	} 
	return (0); 
}

int daytime_process(int daytime_fd)
{ 
	time_t now; 
	char buf[MAXLINE]; 
	time(&now); 
	sprintf(buf, "%s\n", ctime(&now)); 
	if(write(daytime_fd, buf, strlen(buf)) < 0)
	{ 
		printf("daytime: 쓰기 에러 - %s\n", strerror(errno)); 
		return (-1); 
	} 
	return 0; 
}

int main(int argc, char **argv)
{
	int echo_fd, daytime_fd, s;
	int echo_port, daytime_port;
	int nfds, len;
	fd_set read_fds;
	struct sockaddr_in server_addr, client_addr;
	
	if (argc != 2)
		return (-1);
	echo_port = atoi(argv[1]);

	daytime_port = echo_port + 1;

	echo_fd = socket(PF_INET, SOCK_STREAM, 0); 
	bzero((char *)&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(echo_port);
	bind(echo_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 

	daytime_fd = socket(PF_INET, SOCK_STREAM, 0); 
	server_addr.sin_port = htons(daytime_port); 
	bind(daytime_fd,(struct sockaddr *)&server_addr,sizeof(server_addr)); 

	listen(echo_fd, 5); 
	listen(daytime_fd, 5); 
	
	nfds = daytime_fd + 1; 
	FD_ZERO(&read_fds); 

	while(1)
	{ 
		FD_SET(echo_fd, &read_fds); 
		FD_SET(daytime_fd, &read_fds); 
		if(select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
		{ 
			printf("에러발생: %s\n", strerror(errno)); 
		}
		if(FD_ISSET(echo_fd, &read_fds))
		{ 
			len = sizeof(client_addr); 
			bzero((char *)&client_addr, len); 
			s = accept(echo_fd, (struct sockaddr *)&client_addr, &len); 
			echo_process(s); 
			close(s); 
		}
		if(FD_ISSET(daytime_fd, &read_fds)) { 
			len = sizeof(client_addr); 
			s = accept(daytime_fd, (struct sockaddr *)&client_addr, &len); 
			daytime_process(s);
			close(s); 
		} 
	} 
}
