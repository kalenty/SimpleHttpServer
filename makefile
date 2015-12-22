.PHONY: all clean
CFLAGS := ""

all:simpleHttpServer

simpleHttpServer: main.c rio.c sockfd.c log.c
	clang $^ -g -o $@

client:client.c sockfd.c
	clang $^ -o $@

clean:
	@rm simpleHttpServer client 2>/dev/null || true
