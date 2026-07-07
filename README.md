# BatMobile — Zumo Bluetooth

Projet Arduino pour contrôler un robot Zumo Shield à distance via Bluetooth ou console série.

## Matériel requis

- Arduino Uno (ou compatible)
- Pololu Zumo Shield
- Module Bluetooth SeeedStudio Bluetooth Bee (ou compatible AT)
- Servo tête (pin 4)
- Servo pince (pin 12)
- Alimentation (piles AA x4 via le compartiment Zumo)

## Câblage

### Bluetooth

| Module BT | Arduino |
|-----------|---------|
| RX        | Pin 7   |
| TX        | Pin 6   |

### Servos

| Servo   | Pin Arduino |
|---------|-------------|
| Tête    | Pin 4       |
| Pince   | Pin 12      |

## Installation des bibliothèques

1. Depuis le gestionnaire de bibliothèques Arduino :
   - `ZumoShield`

2. Manuellement (ZIP depuis GitHub) :
   - [`ServoTimer2`](https://github.com/nabontra/ServoTimer2) — utilise le Timer 2 pour éviter le conflit avec le PWM moteur (Timer 1, pins 9 et 10)

3. Désactiver le buzzer Zumo pour éviter un conflit Timer 2 avec ServoTimer2 :
   ```bash
   mv ~/Arduino/libraries/ZumoShield/PololuBuzzer.cpp ~/Arduino/libraries/ZumoShield/PololuBuzzer.cpp.bak
   ```

## Compilation et téléversement

```bash
# Compiler
arduino-cli compile --fqbn arduino:avr:uno zumo_copy_20260531153136.ino

# Téléverser
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno zumo_copy_20260531153136.ino
```

Ou via l'IDE Arduino : sélectionner la carte **Arduino Uno** et le bon port série.

## Commandes

Les commandes sont des caractères ASCII uniques, acceptés depuis Bluetooth ou la console série :

| Touche | Action |
|--------|--------|
| `z` | Avancer (accélération progressive) |
| `s` | Reculer (accélération progressive) |
| `d` | Pivoter à droite de 45° |
| `q` | Pivoter à gauche de 45° |
| `w` | Servo tête → position basse (`HEAD_HIGH`) |
| `x` | Servo tête → position haute (`HEAD_LOW`) |
| `c` | Pince ouverte |
| `v` | Pince fermée |
| tout autre | Stop (moteurs à 0) |

## Connexion Bluetooth

Le module se configure automatiquement au démarrage :

- **Nom** : `BatMobile`
- **Code PIN** : `0000`
- **Mode** : esclave, appairage automatique accepté

## Calibration

### Angle de virage

La durée du pivot est définie par `TURN_45_MS` (défaut : `150` ms). Ajuster si le robot ne tourne pas exactement à 45° :

```cpp
#define TURN_45_MS   150  // augmenter si angle trop faible, diminuer si trop grand
```

### Servos (ServoTimer2 en microsecondes)

| Constante   | Valeur par défaut | Description |
|-------------|-------------------|-------------|
| `HEAD_HIGH` | `1100` µs         | Position basse de la tête (commande `w`) |
| `HEAD_LOW`  | `750` µs          | Position haute de la tête (commande `x`) |
| `CLAW_OPEN` | `2250` µs         | Pince ouverte (commande `c`) |
| `CLAW_CLOSE`| `1550` µs (~85°)  | Pince fermée (commande `v`) |

> **Note** : ServoTimer2 accepte des valeurs entre **750 µs** (0°) et **2250 µs** (180°). Une valeur hors de cette plage perturbe le Timer 2 et peut casser la réception Bluetooth.
