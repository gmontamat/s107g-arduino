#!/usr/bin/env python2

"""
Python virtual remote controller for the Syma S107G helicopter

This script sends data accumulated in a queue to the Arduino whenever it
requests it. Upload S107G_PyController.ino to the board. For more information
about this project refer to: https://github.com/gmontamat/s107g-arduino
"""

import curses
import Queue
import serial
import struct
import threading

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
        self.yaw_reset_counter = 0
        self.pitch = 63
        self.pitch_reset_counter = 0
        self.throttle = 0
        self.trim = 63

    def get_next_commands(self):
        for i in xrange(self.max_commands):
            try:
                yield self.commands.get_nowait()
            except Queue.Empty:
                break

    def send_data(self):
        read_data = self.connection.read(1)  # Wait to receive Arduino's ready signal
        if read_data:
            read_data = struct.unpack('B', read_data)[0]
        if read_data == READY_ACK:
            self.connection.write(chr(self.yaw))
            self.connection.write(chr(self.pitch))
            self.connection.write(chr(self.throttle))
            self.connection.write(chr(self.trim))

    def run(self):
        while True:
            self.yaw_reset_counter += 1
            self.pitch_reset_counter += 1
            for command in self.get_next_commands():
                if command == 'y-':
                    self.yaw += 5
                    self.yaw = min(self.yaw, 127)
                    self.yaw_reset_counter = 0
                elif command == 'y+':
                    self.yaw -= 5
                    self.yaw = max(self.yaw, 0)
                    self.yaw_reset_counter = 0
                elif command == 'p+':
                    self.pitch += 5
                    self.pitch = min(self.pitch, 127)
                    self.pitch_reset_counter = 0
                elif command == 'p-':
                    self.pitch -= 5
                    self.pitch = max(self.pitch, 0)
                    self.pitch_reset_counter = 0
                elif command == 't+':
                    self.throttle += 5
                    self.throttle = min(self.throttle, 127)
                elif command == 't-':
                    self.throttle -= 5
                    self.throttle = max(self.throttle, 0)
                elif command == 'r-' and self.trim < 127:
                    self.trim += 1
                elif command == 'r+' and self.trim > 0:
                    self.trim -= 1
                elif command == 't0':
                    self.throttle = 0
                elif command == 'q':
                    return
            if self.yaw_reset_counter > 10000:
                self.yaw = 63
                self.yaw_reset_counter = 0
            if self.pitch_reset_counter > 10000:
                self.pitch = 63
                self.pitch_reset_counter = 0
            self.send_data()


def control_ui():
    # Prepare terminal screen
    stdscr = curses.initscr()
    curses.cbreak()  # React to keys instantly
    curses.noecho()  # Turn off automatic echoing of keys to the screen
    stdscr.keypad(1)  # Enable keypad mode to read arrow keys

    # Print instructions
    stdscr.addstr(0, 0, "S107G Remote Controller")
    stdscr.addstr(2, 0, " a     : throttle up")
    stdscr.addstr(3, 0, " z     : throttle down")
    stdscr.addstr(4, 0, " Up    : pitch forward")
    stdscr.addstr(5, 0, " Down  : pitch backward")
    stdscr.addstr(6, 0, " Left  : yaw left")
    stdscr.addstr(7, 0, " Right : yaw right")
    stdscr.addstr(8, 0, " j     : trim left")
    stdscr.addstr(9, 0, " k     : trim right")
    stdscr.addstr(11, 0, "Press 'q' to quit")
    stdscr.refresh()

    # Start controller
    commands = Queue.Queue()
    controller = SerialController(commands, SERIAL_PORT, SERIAL_BAUD_RATE)
    controller.daemon = True
    controller.start()

    while True:
        c = stdscr.getch()
        if c == ord('q'):
            commands.put('q')
            controller.join()
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

    curses.endwin()


if __name__ == '__main__':
    control_ui()
