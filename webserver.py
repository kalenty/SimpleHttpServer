#the poorest webserver which can return the time
import socket
import time  

HOST, PORT = '', 8888
 
listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listen_socket.bind((HOST, PORT))
listen_socket.listen(1)
print 'Serving HTTP on port %s ...' % PORT 
print time.strftime('%Y-%m-%d %A %X %Z',time.localtime(time.time()))
while True:
    client_connection, client_address = listen_socket.accept()
    request = client_connection.recv(1024)
    print request
 
    http_response = time.strftime('%Y-%m-%d %A %X %Z',time.localtime(time.time()))
    client_connection.sendall(http_response)
    client_connection.close()