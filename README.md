# BatMobile — Zumo Bluetooth

Projet Arduino pour contrôler un robot Zumo Shield à distance via Bluetooth ou console série.

## Matériel requis

- Carte format Uno à puce FTDI (FT232) et ancien bootloader : sélectionner **Arduino Duemilanove or Diecimila**, processeur **ATmega328P** (upload à 57600 bauds — le profil « Arduino Uno » échoue avec `not in sync`)
- Pololu Zumo Shield
- Module Bluetooth SeeedStudio Bluetooth Bee (ou compatible AT)
- Servo tête (pin 4)
- Servo pince (pin 12)
- Alimentation (piles AA x4 via le compartiment Zumo)

## Câblage

### Bluetooth

| Module BT | Arduino |
|-----------|---------|
| RX        | Pin 5   |
| TX        | Pin 6   |

> La pin 7 est réservée par le Zumo Shield (direction du moteur droit) : ne pas y brancher le module BT.

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
arduino-cli compile --fqbn arduino:avr:diecimila:cpu=atmega328 .

# Téléverser
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:diecimila:cpu=atmega328 .
```

Ou via l'IDE Arduino : sélectionner la carte **Arduino Duemilanove or Diecimila** (processeur ATmega328P) et le bon port série.

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
| `\r`, `\n` | Ignorés (fins de ligne du moniteur série / de l'appli) |
| tout autre | Stop (moteurs à 0) |

### Arrêt de sécurité

Après `z` ou `s`, le robot s'arrête si aucune commande n'est reçue pendant `COMMAND_TIMEOUT_MS` (500 ms par défaut). L'appli de pilotage doit renvoyer la commande en continu tant que la touche est maintenue. Mettre `COMMAND_TIMEOUT_MS` à `0` pour désactiver.

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

> **Note** : ServoTimer2 accepte des valeurs entre **750 µs** (0°) et **2250 µs** (180°) ; toute valeur hors plage est automatiquement ramenée dans ces bornes.

## Analyse du code et problèmes connus

### Corrigés

| Problème | Correction |
|----------|------------|
| Le TX Bluetooth (pin 7) partageait la pin de direction du moteur droit (`DIR_R` = 7 dans `ZumoMotors.cpp`) : chaque changement de sens du moteur envoyait des parasites au module BT | TX déplacé sur la **pin 5** |
| `\r` / `\n` envoyés par le moniteur série ou l'appli tombaient dans le `default` et arrêtaient aussitôt le robot | Fins de ligne ignorées |
| Aucune sécurité : après `z`/`s`, le robot roulait indéfiniment si la liaison BT était perdue | Arrêt automatique après `COMMAND_TIMEOUT_MS` sans commande |
| `blueToothSerial.flush()` ne vide pas le tampon de réception : les réponses AT du module étaient lues comme des commandes | Lecture du tampon jusqu'à ce qu'il soit vide |
| Upload `not in sync: resp=0x00` | La carte (FTDI, ancien bootloader à 57600 bauds) doit être déclarée en **Duemilanove**, pas en Uno |

> Le correctif `HEAD_LOW` 500→750 µs n'avait pas d'effet réel : `ServoTimer2::write()` ramène déjà toute valeur < 750 µs à 750 µs.

### Restants (non corrigés)

- **Pin 12** : partagée entre le servo pince et le bouton utilisateur du Zumo (`ZUMO_BUTTON`). Appuyer sur le bouton met le signal du servo à la masse.
- **Pins 4 et 5** : utilisées par la barre de capteurs de ligne Zumo ; incompatibles avec son montage.
- **Rampes** : `z`/`s` repartent toujours de 0, d'où un à-coup si la commande est répétée ou si le sens s'inverse.
- **Commandes bloquantes** : rampe (~400 ms) et pivot (150 ms) utilisent `delay()` ; aucune commande n'est traitée pendant ce temps.
- **Servos au démarrage** : aucune position initiale, ils partent à 1500 µs (90°).
- **Noms inversés** : `HEAD_HIGH` = position basse, `HEAD_LOW` = position haute.
- **LED** : reste allumée après un pivot alors que les moteurs sont arrêtés.
- **Code mort** : `#include <Wire.h>`, `SERVO_0`, `SERVO_90` inutilisés ; `pinMode(RxD/TxD)` redondants avec SoftwareSerial.
- **`PololuBuzzer.cpp.bak`** : le renommage sera annulé à chaque mise à jour de la bibliothèque ZumoShield.
