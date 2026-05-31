# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino sketch for a Zumo Shield robot controlled remotely over Bluetooth. The Bluetooth module (SeeedStudio Bluetooth Bee or compatible) is configured in slave mode, named **BatMobile**, PIN `0000`, on a SoftwareSerial link (RX=pin 6, TX=pin 7 at 38400 baud).

## Build & Flash

Use the Arduino IDE or `arduino-cli`. There is no Makefile or CI pipeline.

```bash
# Compile (replace board/port as needed)
arduino-cli compile --fqbn arduino:avr:uno zumo_copy_20260531153136.ino

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno zumo_copy_20260531153136.ino
```

Required libraries (install via Arduino Library Manager or `arduino-cli lib install`):
- `ZumoShield`
- `Servo` (bundled with Arduino IDE)
- `Wire` (bundled)
- `SoftwareSerial` (bundled)

## Architecture

Single `.ino` file with three sections:

1. **`setupBlueToothConnection()`** — initializes the Bluetooth module with AT commands: slave mode, device name, PIN, auto-accept pairing. Must be called once in `setup()`.
2. **`setup()`** — configures LED pin, hardware Serial (9600 baud for debug), and calls Bluetooth setup.
3. **`loop()`** — tight `while(1)` loop reading single-character commands from `blueToothSerial` and dispatching via `switch`.

## Command Protocol

Each command is a single ASCII character sent over Bluetooth:

| Char | Action |
|------|--------|
| `z`  | Forward (ramp left+right motors 0→300) |
| `s`  | Backward (ramp left+right motors 0→-300) |
| `d`  | Turn right (ramp left motor only 0→200) |
| `q`  | Turn left (ramp right motor only 0→200) |
| `w`/`x` | Head servo (stub — not yet implemented) |
| `c`/`v` | Claw servo (stub — not yet implemented) |
| any other | Stop (both motors to 0, LED off) |

Motor speed range accepted by `ZumoMotors`: -400 to 400.

## Known Issues / TODOs

- `case 'q'` is missing a `break`, so it falls through into `case 'w'`.
- Head servo (`w`/`x`) and claw servo (`c`/`v`) are stubs — `headservo` and a claw servo object need to be attached to pins and commanded.
- Motor ramp loops block all other input; turning commands leave the ramped motor running at full speed until the next command resets it.
