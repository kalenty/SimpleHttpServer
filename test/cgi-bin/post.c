#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void getvalue(char *s, char *value){
    char *p=strchr(s, '=');
    strcpy(value, p+1);
}

int main(int argc, char *argv[]){
    char *buf;
    char name[BUFSIZ], content[BUFSIZ];
    buf = getenv("QUERY_STR");
    getvalue(buf, name);

    sprintf(content, "<!DOCTYPE html>\r\n<html lang=\"zh-CN\">\r\n<head>\r\n    <meta charset=\"UTF-8\">\r\n    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=Edge\">\r\n<title>Post Page</title>\r\n</head>\r\n<body>\r\n<hr/><br/>\r\n<h3>CGI POST</h3>\r\n<form action=\"post\" method=\"POST\">\r\n<p>Please enter your name:<br/>\r\n<input type=\"text\" name=\"name\" /></p>\r\n<input type=\"submit\" value=\"Submit\" />\r\n</form>\r\n<p>Nice to meet you %s!</p>\r\n</body>\r\n</html>", name);

    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    return 0;
}

