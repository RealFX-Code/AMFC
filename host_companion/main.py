import serial
import mido
import mido.backends.rtmidi
import json

# You'll need to change this to your arduino's port.
SERIAL_PORT = '/dev/ttyUSB5'
BAUD_RATE = 115200

CC_MAP = {
    0: 1,
    1: 2,
    2: 3,
    3: 4
}
MIDI_CHANNEL = 0

ema_values = {}
# idk claude explain this one to me
EMA_ALPHA = 0.2
# Only change if changes by this amount
DEADBAND = 3

def ema(index, raw):
    prev = ema_values.get(index, raw)
    smoothed = EMA_ALPHA * raw + (1 - EMA_ALPHA) * prev
    ema_values[index] = smoothed
    return int(smoothed)

def scale(value, in_min, in_max, out_min=0, out_max=127):
    if (in_max <= 1024):
        return value >> 3
    
    scaled = (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    return max(out_min, min(out_max, int(scaled)))  # Clamp to 0-127

def on_fader_change(index, value, bot, top):
    midi_value = scale(value, in_min=bot, in_max=top)

    if index in CC_MAP:
        # Due to bad wires, and bad faders,
        # My configuration never reaches 0 or 127 fully.
        # This is a very hacky workaround for this.
        if midi_value == 1:
            midi_value = 0
        if midi_value == 126:
            midi_value = 127
        msg = mido.Message('control_change',
                           channel=MIDI_CHANNEL,
                           control=CC_MAP[index],
                           value=midi_value)
        midi_out.send(msg)
    print(f"Fader {index}: raw={value} bot={bot} top={top} midi={midi_value}")

arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
midi_out = mido.open_output('Fader Controller', virtual=True)

def main():
    last_values = {}
    values = []

    # This makes the arduino jump to address 0x00000000,
    # Essentially resets it.
    arduino.write("RESET".encode("ASCII"))

    # Load config file
    with open("config.json", "r") as f:
        values = json.loads(f.readline())

    while True:
        line = arduino.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        parts = line.split(',')
        if len(parts) < 2:
            print(f"[arduino] {line}")
            continue
        
        try:
            index = int(parts[0])
            value = int(parts[1])
        except ValueError:
            # Print anything other than the constant stream of values
            print(f"[arduino] {line}")
            continue

        smoothed = ema(index, value)
        if last_values.get(index) is None or abs(smoothed - last_values[index]) >= DEADBAND:
            last_values[index] = smoothed
            on_fader_change(index, smoothed, values[index][1], values[index][2])

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        exit(0)
