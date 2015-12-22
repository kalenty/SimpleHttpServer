/*******  client.c ************/    
#include <sys/types.h>   
#include  <string.h>   
#include  <netdb.h>   
#include <netinet/in.h>   
#include <errno.h>   
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "sockfd.h"

int setnonblock(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

struct epoll_event ev, events[1024];
int epollfd, nfds;

void new_connet()
{
    int sockfd;
    sockfd = open_clientfd("127.0.0.1", (uint16_t) 54523);
    setnonblocking(sockfd);
    
	ev.data.fd = sockfd;
	ev.events =  EPOLLOUT|EPOLLIN;

	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);
    
}

int main(int argc, char *argv[]){
    char buffer[BUFSIZ];
    uint16_t port=54523;
    
    if(argc == 2)
        port=(uint16_t)atoi(argv[1]);

    

    ssize_t res;
    epollfd = epoll_create(1024);
    
    for(int i = 0; i < 1024; i++) new_connet();
    
    
    printf("Staring a loop\n");
	for(;;)
	{
		// argumet -1 indicated the timeout is no sure (blocking) -- we will change it depends on requirement
		nfds = epoll_wait(epollfd, events, 1024, -1);
		for(int i = 0; i < nfds; i++)
		{
			if(events[i].events & EPOLLOUT)
			{
				//close the fd that went wrong
				fprintf(stderr, "can write %d\n", i);
				//int fd = events[i].data.fd;
				continue;
			}
		        else if(events[i].events & EPOLLIN)
			     {
		        	     fprintf(stderr, "can read %d\n", i);
		                int fd = events[i].data.fd;
		                sprintf(buffer, "GET / HTTP/1.0\r\n\r\n");
		                write(fd, buffer, strlen(buffer));
		                res = read(fd, buffer, BUFSIZ);
		                buffer[res] = 0;
		                printf("%d read from server:\n\n", i);
		                close(fd);
				        new_connet();
		            }
			else
			{
				close(events[i].data.fd);
			}

		}
	}
    
    
    
    
    
    return 0;
}

