#define SERVER_STRING "SimpleHttpServer /1.0.0\r\n"

#define GET 71
#define POST 80
#define HEAD 72

#define MAXEVENTS 1024
#define MAXARGS 20

#define strhash(s) (*(s))

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
