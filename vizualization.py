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
xlim = 100 #s
#input_plot = plt.figure()
x = np.linspace(0,time_step, xlim)

plt.ion()
# input_fig = plt.figure(1)
# output_fig = plt.figure(2)
plt.figure(1) # input
h1, = plt.plot([],[])
plt.xlim(0,xlim)
plt.ylim(-2, 6)
plt.suptitle('Calkowite natezenie wody wplywajacej do zbiornika')
plt.xlabel('Czas [s]')
plt.ylabel('Q [m3/s]')
plt.figure(2) #output
h2, = plt.plot([],[])
plt.xlim(0,xlim)
plt.ylim(0, 6.5)
plt.suptitle('Poziom wody w zbiorniku')
plt.xlabel('Czas [s]')
plt.ylabel('H [m]')

print("Server set, starting to listen")
while True:
    data, addr = sock.recvfrom(64)
    #print("Received raw data", data)
    unpacked_msg = struct.unpack('ddddd', data)
    #plant_input = unpacked_msg[0]
    #plant_output = unpacked_msg[1]
    #lock1 = unpacked_msg[2]
    #lock2 = unpacked_msg[3]
    #time.append(unpacked_msg[4])
    #plant_input.append(unpacked_msg[0])
    #plt.plot(time, plant_input)
    # plt.show()
    # print("1")
    plt.figure(1)
    h1.set_xdata(np.append(h1.get_xdata(), unpacked_msg[4]))
    h1.set_ydata(np.append(h1.get_ydata(), unpacked_msg[0]))
    plt.figure(2)
    h2.set_ydata(np.append(h2.get_ydata(), unpacked_msg[1]))
    h2.set_xdata(np.append(h2.get_xdata(), unpacked_msg[4]))
    plt.figure(1)
    curr_time = h1.get_xdata()[-1]
    if curr_time > xlim:
        plt.xlim(curr_time-xlim, curr_time)
    plt.figure(1).canvas.draw()
    plt.figure(1).canvas.flush_events()
    plt.draw()
    plt.figure(2).canvas.draw()
    plt.figure(2).canvas.flush_events()
    plt.draw()
    # plt.show(block = False)
    # print("2")