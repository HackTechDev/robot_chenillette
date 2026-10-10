#include <SoftwareSerial.h>
#include <ServoTimer2.h>
#include "ZumoMotors.h"  // copie locale : n'entraîne pas la compilation de PololuBuzzer

#define RxD 5  // reçoit la sortie TX du module
#define TxD 6  // vers l'entrée RX du module (pas 7 : direction moteur droit ZumoMotors)

#define LED_PIN      13
#define HEAD_PIN     4
#define CLAW_PIN     12

#define DRIVE_SPEED  200
#define RAMP_STEP_MS 2    // +1 de vitesse toutes les 2 ms (0→200 en ~400 ms)

#define TURN_SPEED   200
#define TURN_45_MS   150  // à calibrer selon le matériel réel

// Arrêt de sécurité si aucune commande reçue pendant ce délai en mouvement (0 = désactivé)
#define COMMAND_TIMEOUT_MS  500

// ServoTimer2 utilise des microsecondes : 750=0°, 1500=90°, 2250=180°
#define SERVO_180  2250

// Positions de la tête — à calibrer
#define HEAD_DOWN   1100 // commande w
#define HEAD_UP     750  // commande x

// Positions de la pince — augmenter CLAW_CLOSE si le servo force en fermant
#define CLAW_OPEN   SERVO_180
#define CLAW_CLOSE  1550  // ~85° — à calibrer

SoftwareSerial blueToothSerial(RxD, TxD);

ZumoMotors motors;

ServoTimer2 headservo;
ServoTimer2 clawservo;

int currentSpeed = 0;  // vitesse commune des deux moteurs en ligne droite
int targetSpeed = 0;
unsigned long lastRampMs = 0;

bool pivoting = false;
unsigned long pivotStartMs = 0;

unsigned long lastCommandMs = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  // Bluetooth d'abord : l'appel de courant des servos pendant l'activation du module
  // provoquait une chute de tension et un redémarrage en boucle de la carte
  setupBlueToothConnection();

  // Servos ensuite, un par un, pour étaler l'appel de courant
  headservo.attach(HEAD_PIN);
  headservo.write(HEAD_UP);
  delay(500);
  clawservo.attach(CLAW_PIN);
  clawservo.write(CLAW_OPEN);
}

void handleCommand(char c) {
  // Fins de ligne envoyées par le moniteur série ou l'appli BT : ignorées
  if (c == '\r' || c == '\n') return;

  lastCommandMs = millis();
  Serial.print(c);
  switch (c) {
    case 'z':
      drive(DRIVE_SPEED);
      break;

    case 's':
      drive(-DRIVE_SPEED);
      break;

    case 'd':
      startPivot(TURN_SPEED, -TURN_SPEED);
      break;

    case 'q':
      startPivot(-TURN_SPEED, TURN_SPEED);
      break;

    case 'w':
      headservo.write(HEAD_DOWN);
      break;

    case 'x':
      headservo.write(HEAD_UP);
      break;

    case 'c':
      clawservo.write(CLAW_OPEN);
      break;

    case 'v':
      clawservo.write(CLAW_CLOSE);
      break;

    default:
      stopMotors();
      break;
  }
}

// Ligne droite : la rampe part de la vitesse actuelle (pas d'à-coup si la commande est répétée)
void drive(int speed) {
  digitalWrite(LED_PIN, HIGH);
  pivoting = false;
  targetSpeed = speed;
}

void startPivot(int leftSpeed, int rightSpeed) {
  digitalWrite(LED_PIN, HIGH);
  currentSpeed = 0;
  targetSpeed = 0;
  pivoting = true;
  pivotStartMs = millis();
  motors.setSpeeds(leftSpeed, rightSpeed);
}

void stopMotors() {
  digitalWrite(LED_PIN, LOW);
  currentSpeed = 0;
  targetSpeed = 0;
  pivoting = false;
  motors.setSpeeds(0, 0);
}

// Appelée à chaque tour de loop() : fait avancer la rampe et termine le pivot sans bloquer
void updateMotors() {
  unsigned long now = millis();

  if (pivoting) {
    if (now - pivotStartMs >= TURN_45_MS) stopMotors();
    return;
  }

  if (currentSpeed != targetSpeed && now - lastRampMs >= RAMP_STEP_MS) {
    lastRampMs = now;
    currentSpeed += (targetSpeed > currentSpeed) ? 1 : -1;
    motors.setSpeeds(currentSpeed, currentSpeed);
  }

  if (COMMAND_TIMEOUT_MS > 0 && (currentSpeed != 0 || targetSpeed != 0) &&
      now - lastCommandMs > COMMAND_TIMEOUT_MS) {
    stopMotors();
  }
}

void loop() {
  if (blueToothSerial.available()) {
    handleCommand(blueToothSerial.read());
  }

  if (Serial.available()) {
    handleCommand(Serial.read());
  }

  updateMotors();
}

void setupBlueToothConnection() {
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=BatMobile\r\n"); //set the bluetooth name as "BatMobile"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  // Vide les réponses AT du module pour qu'elles ne soient pas lues comme des commandes
  while (blueToothSerial.available()) blueToothSerial.read();
}
