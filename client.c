/******* 客户端程序 client.c ************/    
#include  <stdio.h>   
#include  <stdlib.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include  <string.h>   
#include  <netdb.h>   
#include <netinet/in.h>   
#include <errno.h>   
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>


static struct sockaddr_in server_addr;    
static int epoll_fd;
static int currency,total_req,total_read;    
static struct epoll_event* events;

int setnonblock(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

void new_conn()
{
        if(--total_req < 0)return;
	int sockfd;
	/* 客户程序开始建立 sockfd描述符 */    
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)    
	{    
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));    
		return ;
	}    
	setnonblock(sockfd);

	//让epoll接管
	struct epoll_event event;
	event.data.fd=sockfd;
	event.events = EPOLLOUT|EPOLLIN;
	epoll_ctl(epoll_fd,EPOLL_CTL_ADD, sockfd,&event);

	/* 客户程序发起连接请求 */    
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)    
	{    
		if(errno == EINPROGRESS)
			return;
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));    
		return;
	}    


}
int main(int argc, char *argv[])    
{    
	struct hostent *host;    
	if((host=gethostbyname(argv[1]))==NULL)    
	{    
		fprintf(stderr,"Gethostname error\n");    
		exit(1);    
	}    

	int portnumber;    
	if((portnumber=atoi(argv[2]))<0)    
	{    
		fprintf(stderr,"Usage:%s hostname portnumber\a\n",argv[0]);    
		exit(1);    
	}    

	/* 客户程序填充服务端的资料 */    
	bzero(&server_addr,sizeof(server_addr));    
	server_addr.sin_family=AF_INET;    
	server_addr.sin_port=htons(portnumber);    
	server_addr.sin_addr=*((struct in_addr *)host->h_addr);    

	//并发数和总的请求数
	currency = atoi(argv[3]);
	total_req = total_read = currency * atoi(argv[4]);

	if((epoll_fd=epoll_create(1000))==-1)    
	{    
		fprintf(stderr,"epoll create Error:%s\a\n",strerror(errno));    
		exit(1);    
	}    

	events = calloc(1000,sizeof(struct epoll_event));

	//初始化并发数个连接
	int i;
	for(i=0;i<currency;i++)new_conn(); 


	while(1)
	{
		fprintf(stderr,"while\n");
		int n,j;
		n = epoll_wait(epoll_fd, events, 1000, -1);
		for(j=0;j<n;j++)
		{
			if(events[j].events & EPOLLOUT)
			{
                                fprintf(stderr, "can write\n",n);
				int fd = events[j].data.fd;
                                int optval;
                                socklen_t optlen = sizeof(optval);
                                if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&optval, &optlen) == -1)
				{
					fprintf(stderr, "getsockopt error\n",n);
				} else if(optval != 0) {
					fprintf(stderr, "connect error\n",n);
					continue;
				};
				struct epoll_event event;
				event.data.fd=fd;
				event.events = EPOLLIN;
				epoll_ctl(epoll_fd,EPOLL_CTL_MOD, fd,&event);

				char buffer2[100];
				memset(buffer2,100,0);
				int type,score,time;
				unsigned long oid;
				type= htonl(atoi(argv[5]));
				oid= htonl(atol(argv[6]));
				score= htonl(atoi(argv[7]));
				char*pass="220106aa";
				char*loc = buffer2;
				memcpy((void*)loc,(void*)(pass),8);
				loc+=8;
				memcpy((void*)loc,(void*)(&type),4);
				loc+=4;
				memcpy((void*)loc,(void*)(&oid),8);
				loc+=8;
				memcpy((void*)loc,(void*)(&score),4);
				write(fd, buffer2, 24);
				/* 连接成功了 */    
			}
			else if(events[j].events & EPOLLIN)
			{
				fprintf(stderr, "can read\n",n);
				int fd = events[j].data.fd;
				char buf[100];
				int n=read(fd,buf,100);
				close(fd);
				new_conn();
				if(n==-1)
				{
					fprintf(stderr,"read Error:%s\a\n",strerror(errno));    
					continue;
				}
				buf[n]=0;
				fprintf(stderr, "return %s\n",buf);
				fprintf(stderr, "total_read %d\n",total_read);
                                if (--total_read <= 0)return;
			}
		}
	} 
}    
