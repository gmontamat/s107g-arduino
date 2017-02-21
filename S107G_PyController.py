#!/usr/bin/env python

"""
Python virtual remote controller for the Syma S107G helicopter

This script sends data accumulated in a queue to the Arduino whenever it
requests it. Upload S107G_PyController.ino to the board. For more information
about this project refer to: https://github.com/gmontamat/s107g-arduino
"""

import curses
import Queue
import threading
import serial
import struct


SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUD_RATE = 9600
READY_ACK = 129


class SerialController(threading.Thread):

    def __init__(self, commands, port, baud_rate, max_commands=20):
        threading.Thread.__init__(self)
        self.commands = commands
        self.connection = serial.Serial(port, baud_rate, timeout=0)
        self.max_commands = max_commands
        self.yaw = 63
        self.pitch = 63
        self.throttle = 0
        self.trim = 63

    def get_next_commands(self):
        for i in xrange(self.max_commands):
            try:
                yield self.commands.get_nowait()
            except Queue.Empty:
                break

    def send_data(self):
        # Wait to receive Arduino's ready signal
        read_data = self.connection.read(1)
        if read_data:
            read_data = struct.unpack('B', read_data)[0]
        if read_data == READY_ACK:
            self.connection.write(chr(self.yaw))
            self.connection.write(chr(self.pitch))
            self.connection.write(chr(self.throttle))
            self.connection.write(chr(self.trim))

    def run(self):
        while True:
            for command in self.get_next_commands():
                if command == 'y-' and self.yaw < 127:
                    self.yaw += 1
                elif command == 'y+' and self.yaw > 0:
                    self.yaw -= 1
                elif command == 'p+' and self.pitch < 127:
                    self.pitch += 1
                elif command == 'p-' and self.pitch > 0:
                    self.pitch -= 1
                elif command == 't+' and self.throttle < 127:
                    self.throttle += 1
                elif command == 't-' and self.throttle > 0:
                    self.throttle -= 1
                elif command == 'r-' and self.trim < 127:
                    self.trim += 1
                elif command == 'r+' and self.trim > 0:
                    self.trim -= 1
                elif command == 't0':
                    self.throttle = 0
                elif command == 'p0':
                    self.pitch = 63
                elif command == 'y0':
                    self.yaw = 63
                elif command == 'q':
                    return
            self.send_data()

def control_ui():
    # Prepare terminal screen
    stdscr = curses.initscr()
    curses.cbreak() # React to keys instantly
    curses.noecho() # Turn off automatic echoing of keys to the screen
    stdscr.keypad(1)    # Enable keypad mode to read arrow keys

    # Print instructions
    stdscr.addstr(0, 0, "S107G Remote Controller")
    stdscr.addstr(2, 0, "a/z:        throttle up/down")
    stdscr.addstr(3, 0, "Up/Down:    pitch forward/backward")
    stdscr.addstr(4, 0, "Left/Right: yaw left/right")
    stdscr.addstr(5, 0, "j/k:        trim left/right")
    stdscr.addstr(7, 0, "Press 'q' to quit")
    stdscr.refresh()

    # Start controller
    commands = Queue.Queue()
    controller = SerialController(commands, SERIAL_PORT, SERIAL_BAUD_RATE)
    controller.start()

    while True:
        c = stdscr.getch()
        if c == ord('q'):
            commands.put('q')
            break
        elif c == ord('a'):
            commands.put('t+')
        elif c == ord('z'):
            commands.put('t-')
        elif c == curses.KEY_LEFT:
            commands.put('y-')
        elif c == curses.KEY_RIGHT:
            commands.put('y+')
        elif c == curses.KEY_UP:
            commands.put('p-')
        elif c == curses.KEY_DOWN:
            commands.put('p+')
        elif c == ord('j'):
            commands.put('r-')
        elif c == ord('k'):
            commands.put('r+')
        elif c == ord('x'):
            commands.put('t0')
        elif c == ord('n'):
            commands.put('p0')
        elif c == ord('m'):
            commands.put('y0')

    curses.endwin()


if __name__ == '__main__':
    control_ui()
