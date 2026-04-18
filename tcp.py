import socket

s = socket.socket()
s.settimeout(30)
s.connect(("192.168.100.15", 80))
s.send(b"GET / HTTP/1.1\r\nHost: 192.168.100.15\r\n\r\n")
print(s.recv(1024))
s.close()