#!/usr/bin/python

import socket
import struct
from matplotlib import pyplot as plt
import numpy as np

UDP_IP = ""
UDP_PORT = 1100
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind((UDP_IP, UDP_PORT))


#dane wykresow
time_step = 0.5 #s
xlim = 1000 #s
x = np.linspace(0,time_step, xlim)

#inicjalizacja wykresow
plt.ion()

plt.figure(1) # input
h1, = plt.plot([],[])
plt.xlim(0,xlim)
plt.ylim(-200, 600)
plt.grid(visible=True)
plt.suptitle('Calkowite natezenie wody przeplywajacej przez zbiornik')
plt.xlabel('Czas [s]')
plt.ylabel('Q [m3/s]')

plt.figure(2) #output
h2, = plt.plot([],[])
plt.xlim(0,xlim)
plt.ylim(0, 6.5)
plt.grid(visible=True)
plt.suptitle('Poziom wody w zbiorniku')
plt.xlabel('Czas [s]')
plt.ylabel('H [m]')

plt.figure(3)
ax = plt.axes(xlim=(0,xlim),ylim=(-0.5,95))
plt.grid(visible=True)
plt.legend(["kierownica 1", "kierownica 2"])
h3, = ax.plot([],[])
h4, = ax.plot([],[])
plt.suptitle('Otwarcie kierownic')
plt.xlabel("Czas [s]")
plt.ylabel("Stopnie")

print("Server set, starting to listen")
while True:
    data, addr = sock.recvfrom(64)
    unpacked_msg = struct.unpack('ddddd', data)

    # input
    plt.figure(1)
    h1.set_xdata(np.append(h1.get_xdata(), unpacked_msg[4]))
    h1.set_ydata(np.append(h1.get_ydata(), unpacked_msg[0]))
    curr_time = h1.get_xdata()[-1]
    if curr_time > xlim:
        plt.xlim(curr_time-xlim, curr_time)
    plt.figure(1).canvas.draw()
    plt.figure(1).canvas.flush_events()
    plt.draw()

    #output
    plt.figure(2)
    h2.set_xdata(np.append(h2.get_xdata(), unpacked_msg[4]))
    h2.set_ydata(np.append(h2.get_ydata(), unpacked_msg[1]))
    if curr_time > xlim:
        plt.xlim(curr_time-xlim, curr_time)
    plt.figure(2).canvas.draw()
    plt.figure(2).canvas.flush_events()
    plt.draw()

    #kierownice
    plt.figure(3)
    h3.set_data(np.append(h3.get_xdata(),unpacked_msg[4]), np.append(h3.get_ydata(),unpacked_msg[2]))
    h4.set_data(np.append(h4.get_xdata(),unpacked_msg[4]), np.append(h4.get_ydata(),unpacked_msg[3]))
    curr_time = h1.get_xdata()[-1]
    if curr_time > xlim:
        plt.xlim(curr_time-xlim, curr_time)
    plt.legend(["kierownica 1", "kierownica 2"])
    plt.figure(3).canvas.draw()
    plt.figure(3).canvas.flush_events()
    plt.draw()
