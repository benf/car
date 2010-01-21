#!/bin/bash
stty -F /dev/ttyUSB0 speed 2400
./joystick >> /dev/ttyUSB0
