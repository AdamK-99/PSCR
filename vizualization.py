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
ymin_input = -200
ymax_input = 600

#inicjalizacja wykresow
plt.ion()

plt.figure(1) # input
h1, = plt.plot([],[])
plt.xlim(0,xlim)
plt.ylim(ymin_input, ymax_input)
plt.grid(visible=True)
plt.suptitle('Calkowite natezenie wody przeplywajacej przez zbiornik')
plt.xlabel('Czas [s]')
plt.ylabel('Q [l/s]')

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

door_message_written = False
auxiliary_tank_message_written = False
previous_auxiliary_tank_condition = 0

print("Server set, starting to listen")
while True:
    data, addr = sock.recvfrom(64)
    unpacked_msg = struct.unpack('ddddddd', data)

    # input
    plt.figure(1)
    h1.set_xdata(np.append(h1.get_xdata(), unpacked_msg[4]))
    h1.set_ydata(np.append(h1.get_ydata(), unpacked_msg[0]))
    curr_time = h1.get_xdata()[-1]
    if curr_time > xlim:
        plt.xlim(curr_time-xlim, curr_time)
    if unpacked_msg[0] > ymax_input:
        plt.ylim(ymin_input, unpacked_msg[0]+10)
        ymax_input = unpacked_msg[0]
    elif unpacked_msg[0] < ymin_input:
        plt.ylim(unpacked_msg[0]-10, ymax_input)
        ymin_input = unpacked_msg[0]
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

    if unpacked_msg[5] == 1 and not door_message_written:
        minutes = unpacked_msg[4] // 60
        seconds = unpacked_msg[4] % 60
        hours = minutes // 60
        minutes = minutes % 60
        if hours != 0 and minutes != 0 and seconds != 0:
            print(int(hours),":",int(minutes),":",int(seconds)," - Sluice door opened, waiting for signal to close...")
            door_message_written = True
    
    if door_message_written and unpacked_msg[5] == 0:
        minutes = unpacked_msg[4] // 60
        seconds = unpacked_msg[4] % 60
        hours = minutes // 60
        minutes = minutes % 60
        if hours != 0 and minutes != 0 and seconds != 0:
            print(int(hours),":",int(minutes),":",int(seconds)," - Sluice door closed")
            door_message_written = False

    if(auxiliary_tank_message_written == False and unpacked_msg[6] == 0):
        minutes = unpacked_msg[4] // 60
        seconds = unpacked_msg[4] % 60
        hours = minutes // 60
        minutes = minutes % 60
        if hours != 0 and minutes != 0 and seconds != 0:
            print(int(hours),":",int(minutes),":",int(seconds)," - Auxiliary sluice tank can be used")
            auxiliary_tank_message_written = True
    
    if(previous_auxiliary_tank_condition == 1 and unpacked_msg[6] == 0):
        auxiliary_tank_message_written = False
    
    previous_auxiliary_tank_condition = unpacked_msg[6]