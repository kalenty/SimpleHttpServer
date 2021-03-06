/*
 * 	main.c
 *	HTTP 1.0 Server
 *	Epoll/NonBlocking
 *  Created on: Nov 22, 2015
 *      Author: j
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "rio.h"
#include "cfg.h"
#include "log.h"
#include "sockfd.h"

int parse_uri(char *uri, char *filename, char *cgiargs);

void read_requesthdrs(rio_t *rp, char *cgiargs);

void serve_dynamic(int fd, char *filename, char *cgiargs);

void serve_static(int fd, char *filename, long size);

void display_error(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg);

void get_filetype(char *filename, char *filetype);

int handle_request(int fd);

int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1)
    {
	   return -1;
    }
    return 0;
}

void SigHandle(int sig){
    switch(sig){
        case SIGPIPE:
            fprintf(stderr, "\nBroken Pipe\n");
            break;
        case SIGCHLD:
            /* wait for all child processes */
            while(waitpid(-1, 0, WNOHANG) > 0) ;
            break;
    }
}


char *tBuf[BUFSIZ];
static char basedir[BUFSIZ]="/home/ubuntu/SimpleHttpServer/test";

int main(int argc, char *argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);
	int listenfd, connfd;
	uint16_t port = 54523;
    
    signal(SIGPIPE, SigHandle);
    signal(SIGCHLD, SigHandle);
    
    // setting up the basedir
    if(*basedir == 0){
        strncpy(basedir, argv[0], BUFSIZ);
        char *c = basedir + strlen(argv[0]);
        for(; *c != '/'; --c);
        *c = 0;
    }

	// enscapulation of bind() and listen() -- sockfd.h/c
	listenfd = getlistenfd(&port);
    sprintf(tBuf, "Listen to port: %d\n", port);
    logging("SimpleHttpServer:", tBuf, NORMAL);

	// set listen socket nonblocking
	setnonblocking(listenfd);
    
    //init the epoll event loop
	struct epoll_event ev, events[MAXEVENTS];
	int epollfd, nfds;
	epollfd = epoll_create1(0);
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;

	epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    logging("Staring a loop", " ", NORMAL);
	for(;;)
	{
		// argumet -1 indicated the timeout is no sure (blocking) -- we will change it depends on requirement
		nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
		for(int i = 0; i < nfds; i++)
		{
			if((events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
					|| !(events[i].events & EPOLLIN))
			{
				//close the fd that went wrong
				fprintf(stderr, "error on fd: %d: %d\n", events[i].data.fd, events[i].events);
				perror("epoll events error");
				close(events[i].data.fd);
				continue;
			}

			if(events[i].data.fd == listenfd)
			{
				//accept the client request and add fd in
				bzero(&clientaddr, sizeof(clientaddr));
				connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
				ev.data.fd = connfd;

				if (connfd == -1)
					perror("accept");

				setnonblocking(connfd);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connfd;

				if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
					perror("epoll_ctl add");
			}
			else
			{
				handle_request(events[i].data.fd);
				//after handled client request, close the fd from the events
				close(events[i].data.fd);
			}

		}
	}

	return 0;
}

int handle_request(int fd)
{
	char buf[BUFSIZ], method[BUFSIZ], uri[BUFSIZ], version[BUFSIZ],
	         filename[BUFSIZ], cgiargs[BUFSIZ];
	struct stat sbuf;
	int is_static;
	rio_t rio;

	//read request line and header
	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, BUFSIZ);

    logging("Hanle request:", buf, NORMAL);
    
	sscanf(buf, "%s %s %s", method, uri, version);
	if(uri[strlen(uri)-1] == '/')
	        strncat(uri, "index.html", BUFSIZ);

	read_requesthdrs(&rio, cgiargs);
    logging("Read the drs:", cgiargs, NORMAL);

	switch(strhash(method)){
	case GET:
        logging("GET :", uri, NORMAL);
	    is_static = parse_uri(uri, filename, cgiargs);
	    if(stat(filename, &sbuf) < 0)
	    {
	    	display_error(fd, filename, "404", "Not found", "Can not find this file");
	        return 1;
	    }
	    if(is_static){
			if(!(S_IRUSR & sbuf.st_mode) || !(S_ISREG(sbuf.st_mode)))
			{
				display_error(fd, filename, "403", "Forbidden", "Can not read this file");
				return 1;
			}
			serve_static(fd, filename, sbuf.st_size);
	    }
	    else
	    {
	    	if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
	    	{
	    		display_error(fd, filename, "403", "Forbidden", "Can not read this file");
	    		return 1;
	        }
	        serve_dynamic(fd, filename, cgiargs);
	     }
	     break;
	case POST: 
        logging("POST :", uri, NORMAL);
	    sprintf(filename, "%s%s", basedir, uri);
		if(stat(filename, &sbuf) < 0)
		{
			display_error(fd, filename, "404", "Not found", "Can not find this file");
			return 1;
		}
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
		{
			display_error(fd, filename, "403", "Forbidden", "Can not read this file");
			return 1;
		}
		serve_dynamic(fd, filename, cgiargs);
		break;
	case HEAD:
        logging("HEAD :", uri, NORMAL);
		display_error(fd, "HEAD request", "200", "OK", "Succeed");
		break;
	default:
		display_error(fd, "", "501", "Method Not Implemented", "");
	   	return -1;
	}

	return 0;
}

void display_error(int fd, const char *cause, const char *errnum, const char *shortmsg, const char *longmsg)
{
    char buf[BUFSIZ], body[BUFSIZ];
    sprintf(body, "<html><title>%s %s</title>", errnum, shortmsg);
    strcat(body, "<body bgcolor='#ffffff'>\r\n");
    sprintf(body, "%s<h1>%s: %s</h1>\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);
    strcat(body, "<hr/>Powered by <em>SimpleHttpServer/1.0.0</em></body></html>\r\n");

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %lu\r\n\r\n", strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    // 1 -- static
    // 0 -- dynamic
    char *ptr;
    logging("Parsing uri:", uri, NORMAL);
    if(strcasestr(uri, "/cgi-bin"))
    {
        ptr = strchr(uri, '?');
        if(ptr){
            sprintf(cgiargs, "%s", ptr + 1);
            *ptr = 0;
        }
        else *cgiargs = 0;
        logging("Dynamic uri:", uri, NORMAL);
        sprintf(filename, "%s%s", basedir, uri);
        return 0;
    }
    else
    {
        *cgiargs = 0;
        logging("Static uri:", uri, NORMAL);
        sprintf(filename, "%s%s", basedir, uri);
        return 1;
    }
}

void get_filetype(char *filename, char *filetype)
{
    char *p = strrchr(filename, '.');
    if(strstr(p, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(p, ".css"))
        strcpy(filetype, "text/css");
    else if(strstr(p, ".js"))
        strcpy(filetype, "text/javascript");
    else if(strstr(p, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(p, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if(strstr(p, ".png"))
        strcpy(filetype, "image/png");
    else
        strcpy(filetype, "text/plain");
}

void read_requesthdrs(rio_t *rp, char *cgiargs)
{
    char buf[BUFSIZ], *p;
    int has_content = 0, nread = 0;
    rio_readlineb(rp, buf, BUFSIZ);
    while(strcmp(buf, "\r\n")){
        if(strcasestr(buf, "content-length")){
            has_content = 1;
            p = strchr(buf, ' ');
            nread = atoi(p);
        }
        rio_readlineb(rp, buf, BUFSIZ);
        if(errno == EAGAIN)
        	break;
    }
    if(has_content){
        rio_readnb(rp, cgiargs, nread);
        cgiargs[nread] = 0;
    }
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[BUFSIZ], *emptylist[]={NULL};
    sprintf(buf, "%s", "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    // fork the dynamic program -- no a good idea, we will find a way to replace it.
    if(fork() == 0){
        setenv("QUERY_STR", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, emptylist, __environ);
    }
    wait(NULL);
}

void serve_static(int fd, char *filename, long size)
{
    int srcfd, cread;
    char filetype[BUFSIZ], buf[BUFSIZ];
    get_filetype(filename, filetype);
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sContent-length: %ld\r\n", buf, size);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));

    srcfd = open(filename, O_RDONLY);
    while((cread = (int)rio_readn(srcfd, buf, BUFSIZ)) > 0)
    {
        rio_writen(fd, buf, cread);
    }
    close(srcfd);
}
