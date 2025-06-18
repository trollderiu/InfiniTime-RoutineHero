#!/bin/bash

# sudo /var/www/html/InfiniSim/InfiniTime/bootloader/ota-dfu-python/dfu.py -z /var/www/html/InfiniSim/InfiniTime/build/output/pinetime-mcuboot-app-dfu-1.14.0.zip -a D9:54:CF:D0:1D:99 --legacy

# Set the Bluetooth MAC address of the device
DEVICE_MAC="D9:54:CF:D0:1D:99"

# Set the path to the DFU package
DFU_PACKAGE="/var/www/html/InfiniSim/InfiniTime/build/output/pinetime-mcuboot-app-dfu-1.14.0.zip"

nrfutil version
adafruit-nrfutil version

# # Function to scan for the device
# function scan_for_device {
#     echo "Scanning for device $DEVICE_MAC..."
    
#     # Use hcitool to scan for the device
#     hcitool lescan > /tmp/ble_scan_results.txt &
    
#     # Allow the scan to run for 10 seconds
#     sleep 1
    
#     # Kill the scan process after 10 seconds
#     kill $! > /dev/null 2>&1
    
#     # Check if the device was found in the scan results
#     if grep -q "$DEVICE_MAC" /tmp/ble_scan_results.txt; then
#         echo "Device $DEVICE_MAC found!"
#         return 0
#     else
#         echo "Device $DEVICE_MAC not found. Retrying in 1 seconds..."
#         return 1
#     fi
# }

# Function to scan for the device using bluetoothctl
function scan_for_device {
    echo "Scanning for device $DEVICE_MAC..."

    # Start bluetoothctl and scan for devices
    bluetoothctl --timeout 10 scan on > /tmp/ble_scan_results.txt

    # Check if the device was found in the scan results
    if grep -q "$DEVICE_MAC" /tmp/ble_scan_results.txt; then
        echo "Device $DEVICE_MAC found!"
        return 0
    else
        echo "Device $DEVICE_MAC not found. Retrying in 1 second..."
        return 1
    fi
}

# Loop until DFU succeeds
while true; do
    # First, scan for the device
    if scan_for_device; then
        echo "Attempting DFU on device $DEVICE_MAC..."
        
        # Run the DFU command
        # nrfutil dfu ble -ic NRF52 -pkg "$DFU_PACKAGE" -n "$DEVICE_MAC"
        # nrfutil dfu ble -ic NRF52 -f -zip "$DFU_PACKAGE" -n "$DEVICE_MAC"
        adafruit-nrfutil dfu -pkg "$DFU_PACKAGE" -p "$DEVICE_MAC"
        # --port /dev/null
        
        # Check if the DFU was successful
        if [ $? -eq 0 ]; then
            echo "DFU successful!"
            break
        else
            echo "DFU failed. Retrying in 1 seconds..."
        fi
    fi
    
    # Wait before retrying
    sleep 1
done
