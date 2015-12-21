#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#define MAXLISTEN 1024
#define diehere(prog) do{ perror(prog); exit(1); }while(0)

int getlistenfd(uint16_t *port);
int getclientfd(const char *hostorip, uint16_t port);
