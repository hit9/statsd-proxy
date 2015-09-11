# coding=utf8

import socket
import random

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('0.0.0.0', 8126))

while 1:
    data, addr = s.recvfrom(64 * 1024)
    lines = data.splitlines()
    print len(lines)
