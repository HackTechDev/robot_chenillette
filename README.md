# BatMobile — Zumo Bluetooth

Projet Arduino pour contrôler un robot Zumo Shield à distance via Bluetooth.

## Matériel requis

- Arduino Uno (ou compatible)
- Pololu Zumo Shield
- Module Bluetooth SeeedStudio Bluetooth Bee (ou compatible AT)
- Alimentation (piles AA x4 via le compartiment Zumo)

## Câblage Bluetooth

| Module BT | Arduino |
|-----------|---------|
| RX        | Pin 7   |
| TX        | Pin 6   |

## Installation

1. Installer les bibliothèques depuis le gestionnaire de bibliothèques Arduino :
   - `ZumoShield`
2. Ouvrir `zumo_copy_20260531153136.ino` dans l'IDE Arduino.
3. Sélectionner la carte **Arduino Uno** et le bon port série.
4. Téléverser.

## Commandes Bluetooth

Envoyer un caractère ASCII depuis une application Bluetooth (ex. Serial Bluetooth Terminal) :

| Touche | Action |
|--------|--------|
| `z` | Avancer (accélération progressive) |
| `s` | Reculer (accélération progressive) |
| `d` | Pivoter à droite de 45° |
| `q` | Pivoter à gauche de 45° |
| `w` / `x` | Servo tête *(à implémenter)* |
| `c` / `v` | Servo pince *(à implémenter)* |
| tout autre | Stop |

## Connexion Bluetooth

Le module se configure automatiquement au démarrage :

- **Nom** : `BatMobile`
- **Code PIN** : `0000`
- **Mode** : esclave, appairage automatique accepté

## Calibration de l'angle de virage

La durée du pivot est définie par `TURN_45_MS` (défaut : `150` ms) dans le sketch. Si le robot ne tourne pas exactement à 45°, ajuster cette valeur :

```cpp
#define TURN_45_MS   150  // augmenter si angle trop faible, diminuer si trop grand
```
