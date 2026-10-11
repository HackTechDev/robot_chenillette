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
| TX        | Pin 5 (`RxD`) |
| RX        | Pin 6 (`TxD`) |

> La pin 7 est réservée par le Zumo Shield (direction du moteur droit) : ne pas y brancher le module BT.

### Servos

| Servo   | Pin Arduino |
|---------|-------------|
| Tête    | Pin 4       |
| Pince   | Pin 12      |

## Installation des bibliothèques

- [`ServoTimer2`](https://github.com/nabontra/ServoTimer2) — à installer manuellement (ZIP depuis GitHub). Utilise le Timer 2 pour éviter le conflit avec le PWM moteur (Timer 1, pins 9 et 10).

La bibliothèque `ZumoShield` n'est **plus nécessaire** : `ZumoMotors.h/.cpp` sont copiés dans le dossier du croquis (licence MIT, voir `LICENSE-ZumoShield.txt`). Cela évite de compiler `PololuBuzzer.cpp`, qui entre en conflit avec ServoTimer2 sur le Timer 2 ; il n'est donc plus nécessaire de le renommer en `.bak`.

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
| `z` | Avancer (rampe progressive depuis la vitesse actuelle jusqu'à 200) |
| `s` | Reculer (rampe progressive depuis la vitesse actuelle jusqu'à -200) |
| `d` | Pivoter à droite (jusqu'à la commande suivante) |
| `q` | Pivoter à gauche (jusqu'à la commande suivante) |
| `w` | Tête : se déplace progressivement vers `HEAD_DOWN` jusqu'à la commande suivante |
| `x` | Tête : se déplace progressivement vers `HEAD_UP` jusqu'à la commande suivante |
| `c` | Pince : s'ouvre progressivement (vers `CLAW_OPEN`) jusqu'à la commande suivante |
| `v` | Pince : se ferme progressivement (vers `CLAW_CLOSE`) jusqu'à la commande suivante |
| `0` … `9` | Vitesse de déplacement : `(n + 1) × 40` (40 à 400), sans interrompre le mouvement en cours |
| `\r`, `\n` | Ignorés (fins de ligne du moniteur série / de l'appli) |
| tout autre | Stop immédiat : moteurs à 0, LED éteinte, servos figés à leur position actuelle |

### Arrêt de sécurité

L'appli télécommande (`../robot_chenillette_telecommande`) envoie le caractère de la touche à l'appui (`z`, `s`, `q`, `d`, `w`, `x`, `c`, `v`) et un **espace** au relâchement, sans répéter la commande pendant l'appui. Le robot (ou le servo) bouge donc tant que la touche est maintenue et s'arrête au relâchement ; les servos restent à la position atteinte.

L'arrêt automatique par délai (`COMMAND_TIMEOUT_MS`) est **désactivé** (`0`) : avec cette appli, il couperait tout appui long. En cas de perte de liaison, le module envoie un message d'état (`+BTSTATE:…`) qui arrête le robot. Pour une appli qui répète les commandes en continu, on peut le réactiver (par exemple `500`).

Les rampes et les pivots ne sont pas bloquants : une commande (par exemple un stop) est prise en compte immédiatement, même pendant une accélération ou un virage.

Au démarrage, le Bluetooth est configuré en premier (environ 4 s), puis la tête est placée en position haute (`HEAD_UP`) et la pince ouverte (`CLAW_OPEN`).

## Connexion Bluetooth

Le module se configure automatiquement au démarrage :

- **Nom** : `BatMobile`
- **Code PIN** : `0000`
- **Mode** : esclave, appairage automatique accepté

## Calibration

### Vitesses

La vitesse se règle depuis l'appli (slider vertical à gauche du pad), qui envoie un chiffre `0` à `9`. Elle s'applique à la ligne droite (`z` / `s`) comme aux pivots (`q` / `d`), et un mouvement en cours la prend en compte immédiatement (via la rampe en ligne droite).

- `SPEED_PER_LEVEL` (défaut `40`) — écart entre deux niveaux : vitesse = `(niveau + 1) × SPEED_PER_LEVEL` (plage moteur : 0 à 400).
- `SPEED_LEVEL_DEFAULT` (défaut `4`, soit `200`) — niveau au démarrage. L'appli renvoie le niveau de son slider à chaque connexion.

### Servos (ServoTimer2 en microsecondes)

| Constante   | Valeur par défaut | Description |
|-------------|-------------------|-------------|
| `HEAD_DOWN` | `1100` µs         | Butée de la tête pour la commande `w` |
| `HEAD_UP`   | `750` µs          | Butée de la tête pour la commande `x` (et position au démarrage) |
| `CLAW_OPEN` | `2250` µs         | Butée pince ouverte, commande `c` (et position au démarrage) |
| `CLAW_CLOSE`| `1550` µs (~85°)  | Butée pince fermée, commande `v` |
| `SERVO_STEP_US` / `SERVO_STEP_MS` | `10` µs / `20` ms | Vitesse des servos (500 µs/s) : augmenter `SERVO_STEP_US` pour aller plus vite |

> **Note** : ServoTimer2 accepte des valeurs entre **750 µs** (0°) et **2250 µs** (180°) ; toute valeur hors plage est automatiquement ramenée dans ces bornes.

## Erreurs rencontrées et résolutions

### `cannot open port /dev/ttyUSB0: Permission denied`

```
OS error: cannot open port /dev/ttyUSB0: Permission denied
Error: unable to open port /dev/ttyUSB0 for programmer arduino
Failed uploading: uploading error: exit status 1
```

**Cause** : le port série appartient au groupe `dialout` (`crw-rw---- root dialout`) et l'utilisateur n'en fait pas partie.

**Résolution** :

```bash
# Permanent (se déconnecter puis se reconnecter à la session Linux ensuite)
sudo usermod -aG dialout $USER

# Temporaire (perdu au débranchement de la carte ou au redémarrage)
sudo chmod a+rw /dev/ttyUSB0
```

### `programmer is not responding` / `not in sync: resp=0x00`

```
Error: programmer is not responding
Warning: attempt 1 of 10: not in sync: resp=0x00
```

**Cause** : la carte n'est pas une Arduino Uno. Elle utilise une puce USB FTDI FT232 (`lsusb` : `0403:6001`) et un ancien bootloader qui communique à **57600 bauds**, alors que le profil « Arduino Uno » téléverse à 115200 bauds. Diagnostic avec `avrdude` : aucune réponse à 115200, réponse correcte à 57600 (`Device signature = 1E 95 0F (ATmega328P)`, bootloader 1.16).

**Résolution** : dans l'IDE, *Outils > Type de carte* → **Arduino Duemilanove or Diecimila**, *Processeur* → **ATmega328P**. Avec `arduino-cli` : `--fqbn arduino:avr:diecimila:cpu=atmega328`. Fermer le moniteur série avant de téléverser.

### Module Bluetooth invisible (« BatMobile » non détecté)

**Symptôme** : ni le smartphone ni le PC ne voient « BatMobile », alors que la console série affiche bien `The slave bluetooth is inquirable!`.

**Cause** : le Bluetooth Bee ne devient visible qu'après avoir reçu `+INQ=1`. Les liaisons RX/TX entre l'Arduino et le module étaient inversées par rapport au code : les commandes AT n'atteignaient jamais le module.

**Diagnostic** : un croquis de test envoyant `AT` et `+INQ=1` à 9600–115200 bauds n'obtenait aucune réponse avec RX=6/TX=5 ; en inversant (RX=5/TX=6), le module répond à **38400 bauds** par `OK` puis `+BTSTATE:2` (mode visible).

**Résolution** : `RxD` = pin 5 (sortie TX du module), `TxD` = pin 6 (entrée RX du module). Après reset, « BatMobile » apparaît (adresse `00:18:A1:12:16:C5`).

> Rappel : le Bluetooth Bee est en Bluetooth classique (SPP) ; il n'est pas visible depuis un iPhone.

### Module qui clignote en bleu, robot muet après redémarrage

**Symptôme** : après un redémarrage (USB débranché, ou simple reset), le module Bluetooth clignote en bleu, « BatMobile » n'est plus visible et la console n'affiche rien (ou un message coupé : `The slave bluetooth is i…`).

**Cause** : chute de tension au démarrage. Les servos étaient envoyés en butée (tête 750 µs, pince 2250 µs) **avant** la configuration Bluetooth ; l'appel de courant des servos cumulé à celui du module au moment de `+INQ=1` faisait redémarrer la carte en boucle. Le module, jamais configuré, reste en attente (clignotement bleu).

**Diagnostic** : la même séquence Bluetooth sans servos démarre correctement à chaque fois ; les servos seuls aussi.

**Résolution** : dans `setup()`, configuration Bluetooth **d'abord**, puis attache des servos un par un (500 ms d'écart).

> Sur batteries, utiliser des piles chargées. Si un servo vibre ou force en butée, ajuster `HEAD_UP` / `CLAW_OPEN` : un servo bloqué consomme beaucoup de courant et peut encore faire chuter la tension.

## Analyse du code et problèmes connus

### Corrigés

| Problème | Correction |
|----------|------------|
| Le TX Bluetooth (pin 7) partageait la pin de direction du moteur droit (`DIR_R` = 7 dans `ZumoMotors.cpp`) : chaque changement de sens du moteur envoyait des parasites au module BT | Liaison BT déplacée sur les **pins 5 et 6** (voir câblage) |
| `\r` / `\n` envoyés par le moniteur série ou l'appli tombaient dans le `default` et arrêtaient aussitôt le robot | Fins de ligne ignorées |
| Aucune sécurité : après `z`/`s`, le robot roulait indéfiniment si la liaison BT était perdue | Arrêt automatique après `COMMAND_TIMEOUT_MS` sans commande |
| `blueToothSerial.flush()` ne vide pas le tampon de réception : les réponses AT du module étaient lues comme des commandes | Lecture du tampon jusqu'à ce qu'il soit vide |
| Upload `not in sync: resp=0x00` | La carte (FTDI, ancien bootloader à 57600 bauds) doit être déclarée en **Duemilanove**, pas en Uno |

> Le correctif `HEAD_LOW` 500→750 µs n'avait pas d'effet réel : `ServoTimer2::write()` ramène déjà toute valeur < 750 µs à 750 µs.

### Corrigés (deuxième passe)

| Problème | Correction |
|----------|------------|
| Rampes repartant toujours de 0 (à-coup si la commande est répétée ou si le sens s'inverse) | La rampe part de la vitesse actuelle ; une inversion passe progressivement de 200 à -200 |
| Commandes bloquantes : rampe (~400 ms) et pivot (150 ms) en `delay()` | Machine à états non bloquante (`updateMotors()` appelée à chaque `loop()`, basée sur `millis()`) |
| Servos sans position initiale (1500 µs au démarrage) | Tête haute et pince ouverte dans `setup()`, **après** la configuration Bluetooth |
| Noms inversés `HEAD_HIGH` (= bas) / `HEAD_LOW` (= haut) | Renommés `HEAD_DOWN` / `HEAD_UP` |
| LED restant allumée après un pivot | Le pivot se termine par `stopMotors()`, qui éteint la LED |
| Code mort : `Wire.h`, `SERVO_0`, `SERVO_90`, `pinMode(RxD/TxD)`, `while(1)` dans `loop()` | Supprimés |
| Renommage de `PololuBuzzer.cpp` annulé à chaque mise à jour de ZumoShield | Copie locale de `ZumoMotors` : la bibliothèque ZumoShield n'est plus utilisée |

### Pivots continus (troisième passe)

| Problème | Correction |
|----------|------------|
| `q` / `d` faisaient un pivot fixe de 45° (`TURN_45_MS`), quelle que soit la durée de l'appui | Pivot continu jusqu'à la commande suivante ; l'appli envoie désormais un espace au relâchement des boutons ↺ / ↻ |
| L'arrêt par délai de 500 ms supposait une appli qui répète les commandes ; l'appli n'envoie qu'un caractère à l'appui et un espace au relâchement (constaté en enregistrant les commandes reçues) | `COMMAND_TIMEOUT_MS` mis à `0` ; la perte de liaison reste détectée via `+BTSTATE` |

### Servos progressifs (quatrième passe)

| Problème | Correction |
|----------|------------|
| `w` / `x` / `c` / `v` envoyaient le servo directement en butée : impossible de choisir une position intermédiaire | Déplacement progressif (`updateServos()`, `SERVO_STEP_US` toutes les `SERVO_STEP_MS`) tant que la touche est maintenue ; l'espace envoyé au relâchement fige le servo à sa position actuelle (`stopServos()`) |

### Vitesse réglable (cinquième passe)

| Demande | Réalisation |
|---------|-------------|
| Régler la vitesse de déplacement depuis le smartphone | Commandes `0`–`9` → `setSpeedLevel()` ; vitesse commune ligne droite / pivot (`moveSpeed`), remplace `DRIVE_SPEED` et `TURN_SPEED` |

### Contraintes matérielles (non corrigeables par logiciel)

- **Pin 12** : partagée entre le servo pince et le bouton utilisateur du Zumo (`ZUMO_BUTTON`).
  - *Rôle du bouton* : bouton libre à la disposition du programme (rien d'automatique, contrairement au bouton reset). Un appui relie la pin 12 à la masse ; il se lit avec `pinMode(12, INPUT_PULLUP)` + `digitalRead(12)` (`LOW` = appuyé) ou la classe `Pushbutton` de Pololu. Usage typique : « appuyer pour démarrer » (sumo robot), changer de mode, lancer une calibration.
  - *Dans ce projet* : le bouton n'est pas utilisé. Un appui met le signal du servo pince à la masse : la pince ne reçoit plus d'impulsions (elle peut se relâcher ou trembler) et la sortie de l'Arduino débite directement vers la masse. Sans conséquence tant qu'on n'appuie pas — **ne pas appuyer sur ce bouton**.
  - *Pour utiliser le bouton* : brancher la pince sur une pin libre (par exemple 11 ou 2) et modifier `CLAW_PIN`. Choix actuel : pas de changement, tout fonctionne.
- **Pins 4 et 5** : utilisées par la barre de capteurs de ligne Zumo ; le câblage actuel est incompatible avec son montage.
- **Pin 6** : sur les cartes à ATmega32U4 (Leonardo), le buzzer du Zumo est sur la pin 6 ; sans incidence ici (ATmega328P, buzzer sur la pin 3).
