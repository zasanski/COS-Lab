import socket

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9999
BUFFER_SIZE = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

messages = ["13*3", "5+2", "10/2", "7-4", "10/0", "abc", "2137"]

for msg in messages:
    print(f"Sending: '{msg}'")
    sock.sendto(msg.encode(), (SERVER_IP, SERVER_PORT))

    data, server = sock.recvfrom(BUFFER_SIZE)
    print(f"Received: '{data.decode()}' from {server}")

sock.close()
