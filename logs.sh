#!/bin/bash

# Replace with the correct handle for the logging characteristic
LOG_HANDLE="0x0025"
DEVICE_MAC="D9:54:CF:D0:1D:99"

while true; do
    gatttool -b $DEVICE_MAC --char-read --handle=$LOG_HANDLE
    sleep 1  # Adjust the interval as necessary
done