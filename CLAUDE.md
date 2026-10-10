# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino sketch for a Zumo Shield robot controlled remotely over Bluetooth. The Bluetooth module (SeeedStudio Bluetooth Bee or compatible) is configured in slave mode, named **BatMobile**, PIN `0000`, on a SoftwareSerial link (RX=pin 6, TX=pin 5 at 38400 baud).

## Build & Flash

Use the Arduino IDE or `arduino-cli`. There is no Makefile or CI pipeline.

```bash
# Compile (replace board/port as needed)
arduino-cli compile --fqbn arduino:avr:diecimila:cpu=atmega328 .

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:diecimila:cpu=atmega328 .
```

Required libraries (install via Arduino Library Manager or `arduino-cli lib install`):
- `ZumoShield` — seule `ZumoMotors.h` est incluse (pas `ZumoShield.h`) pour éviter le conflit Timer 2 avec `PololuBuzzer`
- `ServoTimer2` — à installer manuellement depuis GitHub (ZIP). Utilise le Timer 2 au lieu du Timer 1, évitant le conflit avec le PWM moteur du ZumoShield (pins 9 et 10)
- `Wire` (bundled)
- `SoftwareSerial` (bundled)

> **Important** : `PololuBuzzer.cpp` dans la bibliothèque ZumoShield doit être renommé en `.cpp.bak` pour éviter un conflit Timer 2 avec ServoTimer2 :
> `mv ~/Arduino/libraries/ZumoShield/PololuBuzzer.cpp ~/Arduino/libraries/ZumoShield/PololuBuzzer.cpp.bak`

## Architecture

Single `.ino` file:

1. **`setupBlueToothConnection()`** — initializes the Bluetooth module with AT commands: slave mode, device name, PIN, auto-accept pairing. Must be called once in `setup()`.
2. **`setup()`** — configures LED pin, hardware Serial (9600 baud for debug), attaches servos, and calls Bluetooth setup.
3. **`handleCommand(char c)`** — dispatches a command character via `switch`. Called from both `blueToothSerial` and hardware `Serial`. Ignores `\r` / `\n` and records the time of the last command.
4. **`stopMotors()`** — stops both motors, turns the LED off, clears the `moving` flag.
5. **`loop()`** — tight `while(1)` loop reading single-character commands from `blueToothSerial` or `Serial` and calling `handleCommand()`. Also enforces the failsafe: if moving and no command for `COMMAND_TIMEOUT_MS`, calls `stopMotors()`.

## Pin Assignment

| Pin | Usage |
|-----|-------|
| 5 | TxD Bluetooth (SoftwareSerial) |
| 6 | RxD Bluetooth (SoftwareSerial) |
| 7 | ZumoShield — direction moteur droit (ne pas réutiliser) |
| 8 | ZumoShield — direction moteur gauche |
| 9 | ZumoShield — PWM moteur droit |
| 10 | ZumoShield — PWM moteur gauche |
| 4 | Servo tête (`HEAD_PIN`) |
| 12 | Servo pince (`CLAW_PIN`) — partagée avec le bouton utilisateur du Zumo |
| 13 | LED |

## Command Protocol

Each command is a single ASCII character sent over Bluetooth:

| Char | Action |
|------|--------|
| `z` | Avance (rampe 0→200, moteurs restent à 200) |
| `s` | Recule (rampe 0→-200, moteurs restent à -200) |
| `d` | Pivote à droite 45° puis stop |
| `q` | Pivote à gauche 45° puis stop |
| `w` | Servo tête → position basse (`HEAD_HIGH` = 1100 µs) |
| `x` | Servo tête → position haute (`HEAD_LOW` = 750 µs) |
| `c` | Pince ouverte (`CLAW_OPEN` = 180°) |
| `v` | Pince fermée (`CLAW_CLOSE` = ~85°, calibrable) |
| `\r`, `\n` | Ignorés (fins de ligne) |
| tout autre | Stop (moteurs à 0, LED off) |

**Failsafe** : après `z` ou `s`, si aucune commande n'arrive pendant `COMMAND_TIMEOUT_MS` (500 ms), le robot s'arrête. L'appli de pilotage doit donc renvoyer la commande en continu tant que la touche est maintenue.

Motor speed range accepted by `ZumoMotors`: -400 to 400.

## Calibration

ServoTimer2 utilise des microsecondes : `750`=0°, `1500`=90°, `2250`=180°. **Plage valide : 750–2250 µs** — `ServoTimer2::write()` borne automatiquement toute valeur hors plage.

- **`COMMAND_TIMEOUT_MS`** (défaut `500` ms) — délai du failsafe. `0` désactive l'arrêt automatique.
- **`TURN_45_MS`** (défaut `150` ms) — durée du pivot, à ajuster pour obtenir exactement 45°.
- **`HEAD_HIGH`** (défaut `1100` µs) — position basse de la tête (commande `w`). Ajuster selon la mécanique.
- **`HEAD_LOW`** (défaut `750` µs) — position haute de la tête (commande `x`). Minimum absolu = 750 µs.
- **`CLAW_CLOSE`** (défaut `1550` µs ≈ 85°) — position de fermeture de la pince. Augmenter si le servo force en fermant.
