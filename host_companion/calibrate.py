import serial
import sys
import json
from time import sleep

# You'll need to change this to your arduino's port.
SERIAL_PORT = '/dev/ttyUSB5'
BAUD_RATE = 115200

def scale(value, in_min, in_max, out_min=0, out_max=127):
    return value >> 3

arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
def main():
    values = []

    ready4cal = False
    arduino.write("RESET".encode("ASCII"))
    while True:
        for i in range(5):
            # Begin calibration
            arduino.write("CALIBRATE".encode("ASCII"))
            line = arduino.readline().decode('utf-8', errors='ignore').strip()
            if( line == "READY4CAL" ):
                ready4cal = True
                break
            else:
                print(f"Failed to initialize calibration... tried {i} times...")
        if ( ready4cal == False ):
            print("Couldn't enter calibration mode. Please manually reset the arduino.")
            continue
        else:
            break

    CAL_POINTS = ["BOTTOM", "TOP"]
    for point in CAL_POINTS:
        while True:
            line = arduino.readline().decode('utf-8', errors='ignore').strip()
            if( line != "WAIT4READY"):
                print(f" [ARDUINO] {line}")
            else:
                break
        print(f" !! Put your sliders to the {point} position and press enter!")
        _ = input()
        arduino.write("READY".encode("ASCII"))
    
    # Process all serial lines we've recieved
    while True:
        line = arduino.readline().decode('utf-8', errors='ignore').strip()
        # If we somehow don't have a line, continue loop
        if not line:
            continue

        # If we're done with the calibration data,
        # just exit out of this loop. We're done with this script.
        if ( line == "DONEWCAL" ):
            break

        parts = line.split(',')
        print(f"[arduino] {line}")
        
        try:
            index = int(parts[0])
            bot   = int(parts[1])
            top   = int(parts[2])
            values.append([index, bot, top])
        except ValueError:
            continue
        
    for val in values:
        print(f"slider {val[0]}: BOT = {val[1]}; TOP = {val[2]}")
    
    with open("config.json", "w") as f:
        f.write(json.dumps(values))


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        exit(0)
