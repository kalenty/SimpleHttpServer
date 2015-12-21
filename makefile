.PHONY: all clean
CFLAGS := ""

all:Aowu

Aowu: main.c rio.c sockfd.c
	clang $^ -g -o $@

client:client.c sockfd.c
	clang $^ -o $@

clean:
	@rm Aowu client 2>/dev/null || true