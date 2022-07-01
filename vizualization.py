#!/usr/bin/python

import socket
import struct

UDP_IP = ""
UDP_PORT = 1100
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind((UDP_IP, UDP_PORT))

print("Server set, starting to listen")

while True:
    data, addr = sock.recvfrom(64)
    #print("Received raw data", data)
    unpacked_msg = struct.unpack('ddddd', data)
    print("Unpacked", unpacked_msg)