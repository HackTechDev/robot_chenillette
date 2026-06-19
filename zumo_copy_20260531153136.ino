#include <Wire.h>
#include <ZumoMotors.h>
#include <SoftwareSerial.h>
#include <ServoTimer2.h>

#define RxD 6
#define TxD 7

#define LED_PIN      13
#define HEAD_PIN     11
#define CLAW_PIN     12

#define TURN_SPEED   200
#define TURN_45_MS   150  // à calibrer selon le matériel réel

// ServoTimer2 utilise des microsecondes : 750=0°, 1500=90°, 2250=180°
#define SERVO_0    750
#define SERVO_90   1500
#define SERVO_180  2250

// Positions de la pince — augmenter CLAW_CLOSE si le servo force en fermant
#define CLAW_OPEN   SERVO_180
#define CLAW_CLOSE  1550  // ~85° — à calibrer

SoftwareSerial blueToothSerial(RxD, TxD);

ZumoMotors motors;

ServoTimer2 headservo;
ServoTimer2 clawservo;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  headservo.attach(HEAD_PIN);
  clawservo.attach(CLAW_PIN);

  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
}

void handleCommand(char c) {
  Serial.print(c);
  switch (c) {
    case 'z':
      digitalWrite(LED_PIN, HIGH);
      for (int speed = 0; speed <= 200; speed++) {
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed);
        delay(2);
      }
      break;

    case 's':
      digitalWrite(LED_PIN, HIGH);
      for (int speed = 0; speed >= -200; speed--) {
        motors.setLeftSpeed(speed);
        motors.setRightSpeed(speed);
        delay(2);
      }
      break;

    case 'd':
      digitalWrite(LED_PIN, HIGH);
      motors.setLeftSpeed(TURN_SPEED);
      motors.setRightSpeed(-TURN_SPEED);
      delay(TURN_45_MS);
      motors.setLeftSpeed(0);
      motors.setRightSpeed(0);
      break;

    case 'q':
      digitalWrite(LED_PIN, HIGH);
      motors.setRightSpeed(TURN_SPEED);
      motors.setLeftSpeed(-TURN_SPEED);
      delay(TURN_45_MS);
      motors.setLeftSpeed(0);
      motors.setRightSpeed(0);
      break;

    case 'w':
      headservo.write(SERVO_90);
      break;

    case 'x':
      headservo.write(SERVO_0);
      break;

    case 'c':
      clawservo.write(CLAW_OPEN);
      break;

    case 'v':
      clawservo.write(CLAW_CLOSE);
      break;

    default:
      digitalWrite(LED_PIN, LOW);
      motors.setLeftSpeed(0);
      motors.setRightSpeed(0);
      break;
  }
}

void loop() {
  char recvChar;

  while (1) {
    if (blueToothSerial.available()) {
      recvChar = blueToothSerial.read();
      handleCommand(recvChar);
    }

    if (Serial.available()) {
      recvChar = Serial.read();
      handleCommand(recvChar);
    }
  }
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
  blueToothSerial.flush();
}