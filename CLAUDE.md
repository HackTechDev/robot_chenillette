# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Arduino sketch for a Zumo Shield robot controlled remotely over Bluetooth. The Bluetooth module (SeeedStudio Bluetooth Bee or compatible) is configured in slave mode, named **BatMobile**, PIN `0000`, on a SoftwareSerial link (RxD=pin 5 ← TX module, TxD=pin 6 → RX module, 38400 baud).

## Build & Flash

Use the Arduino IDE or `arduino-cli`. There is no Makefile or CI pipeline.

```bash
# Compile (replace board/port as needed)
arduino-cli compile --fqbn arduino:avr:diecimila:cpu=atmega328 .

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:diecimila:cpu=atmega328 .
```

Required libraries (install via Arduino Library Manager or `arduino-cli lib install`):
- `ServoTimer2` — à installer manuellement depuis GitHub (ZIP). Utilise le Timer 2 au lieu du Timer 1, évitant le conflit avec le PWM moteur du ZumoShield (pins 9 et 10)
- `SoftwareSerial` (bundled)

`ZumoMotors.h/.cpp` sont une **copie locale** de la bibliothèque Pololu ZumoShield 2.1.0 (MIT, `LICENSE-ZumoShield.txt`), incluse avec `#include "ZumoMotors.h"`. Ne pas la remplacer par `<ZumoMotors.h>` : la bibliothèque complète compilerait `PololuBuzzer.cpp`, dont l'ISR Timer 2 entre en conflit avec ServoTimer2.

## Architecture

`robot_chenillette.ino` + copie locale `ZumoMotors.h/.cpp`. Le pilotage moteur est **non bloquant** (pas de `delay()` hors setup) :

1. **`setupBlueToothConnection()`** — initializes the Bluetooth module with AT commands (slave mode, name, PIN, auto-accept pairing), then drains the RX buffer.
2. **`setup()`** — LED pin, hardware Serial (9600 baud debug), Bluetooth setup **first**, then attaches servos one at a time (`HEAD_UP`, 500 ms, `CLAW_OPEN`). Order matters: servo inrush during `+INQ=1` browned out the board.
3. **`handleCommand(char c)`** — dispatches a command character via `switch`. Called from both `blueToothSerial` and `Serial`. Ignores `\r` / `\n` and records `lastCommandMs`.
4. **`drive(speed)`** — sets `targetSpeed`; the ramp starts from `currentSpeed`.
5. **`startPivot(dir)`** — starts a pivot on the spot (`+1` right, `-1` left) at `moveSpeed`; it lasts until the next command (the app sends `' '` on release).
   **`setSpeedLevel(level)`** — digit `0`–`9` → `moveSpeed = (level + 1) × SPEED_PER_LEVEL`; applied to the ongoing move (ramp target or pivot) without stopping it.
6. **`stopMotors()`** — immediate stop, LED off, resets state.
7. **`stopServos()` / `updateServos()`** — servo targets are set by `w`/`x`/`c`/`v`; `updateServos()` (every `loop()`) moves `headPos`/`clawPos` toward their target by `SERVO_STEP_US` every `SERVO_STEP_MS`; `stopServos()` (on any stop char) freezes them where they are.
8. **`updateMotors()`** — called every `loop()`: advances the ramp by 1 every `RAMP_STEP_MS` and enforces the optional `COMMAND_TIMEOUT_MS` failsafe (disabled by default).
9. **`loop()`** — reads one char from each serial link, then calls `updateMotors()` and `updateServos()`.

## Pin Assignment

| Pin | Usage |
|-----|-------|
| 5 | RxD Bluetooth (SoftwareSerial) ← TX du module |
| 6 | TxD Bluetooth (SoftwareSerial) → RX du module |
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
| `z` | Avance (rampe depuis la vitesse actuelle → `moveSpeed`) |
| `s` | Recule (rampe depuis la vitesse actuelle → `-moveSpeed`) |
| `d` | Pivote à droite jusqu'à la commande suivante |
| `q` | Pivote à gauche jusqu'à la commande suivante |
| `w` | Tête → vers `HEAD_DOWN` (1100 µs), progressif jusqu'à la commande suivante |
| `x` | Tête → vers `HEAD_UP` (750 µs), progressif jusqu'à la commande suivante |
| `c` | Pince → vers `CLAW_OPEN` (2250 µs), progressif jusqu'à la commande suivante |
| `v` | Pince → vers `CLAW_CLOSE` (~1550 µs), progressif jusqu'à la commande suivante |
| `0` … `9` | Vitesse `(n + 1) × 40` pour z/s/q/d — ne stoppe pas le mouvement (défaut `4` = 200) |
| `\r`, `\n` | Ignorés (fins de ligne) |
| tout autre | Stop (moteurs à 0, LED off, servos figés à leur position actuelle) |

**Protocole de l'appli** (`../robot_chenillette_telecommande`, React Native) : caractère envoyé à l'appui, espace (`' '` → stop) au relâchement, **aucune répétition** pendant l'appui. Ne pas réactiver un délai de keep-alive (`COMMAND_TIMEOUT_MS`) sans faire répéter les commandes par l'appli. Une perte de liaison produit un message `+BTSTATE:…` du module, qui tombe dans `default` → stop.

Motor speed range accepted by `ZumoMotors`: -400 to 400.

## Calibration

ServoTimer2 utilise des microsecondes : `750`=0°, `1500`=90°, `2250`=180°. **Plage valide : 750–2250 µs** — `ServoTimer2::write()` borne automatiquement toute valeur hors plage.

- **`COMMAND_TIMEOUT_MS`** (défaut `0` = désactivé) — délai de keep-alive ; voir le protocole de l'appli ci-dessus.
- **`RAMP_STEP_MS`** (défaut `2` ms) — intervalle entre deux incréments de vitesse de la rampe.
- **`SPEED_PER_LEVEL`** (défaut `40`) / **`SPEED_LEVEL_DEFAULT`** (défaut `4` → 200) — vitesse réglable par l'appli, commune à la ligne droite et aux pivots. Garder ces valeurs synchronisées avec les constantes du même nom dans `App.tsx` de l'appli.
- **`SERVO_STEP_US` / `SERVO_STEP_MS`** (défaut `10` µs / `20` ms) — vitesse de déplacement des servos.
- **`HEAD_DOWN`** (défaut `1100` µs) — position basse de la tête (commande `w`). Ajuster selon la mécanique.
- **`HEAD_UP`** (défaut `750` µs) — position haute de la tête (commande `x`). Minimum absolu = 750 µs.
- **`CLAW_CLOSE`** (défaut `1550` µs ≈ 85°) — position de fermeture de la pince. Augmenter si le servo force en fermant.
