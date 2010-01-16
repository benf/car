#!/bin/bash
stty -F /dev/ttyUSB0 speed 4800
./joystick >> /dev/ttyUSB0
