.PHONY: all clean
CFLAGS := ""

all:simpleHttpServer

simpleHttpServer: main.c rio.c sockfd.c log.c
	clang $^ -g -o $@

clean:
	@rm simpleHttpServer client 2>/dev/null || true
