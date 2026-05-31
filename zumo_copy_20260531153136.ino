/*
 * This example uses the ZumoMotors library to drive each motor on the Zumo
 * forward, then backward. The yellow user LED is on when a motor should be
 * running forward and off when a motor should be running backward. If a
 * motor on your Zumo has been flipped, you can correct its direction by
 * uncommenting the call to flipLeftMotor() or flipRightMotor() in the setup()
 * function.
 */

#include <Servo.h>
#include <Wire.h>
#include <ZumoShield.h>
#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 6
#define TxD 7


#define LED_PIN 13

SoftwareSerial blueToothSerial(RxD, TxD);

ZumoMotors motors;

Servo headservo; 

int pos = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();

  
}

void loop()
{
  char recvChar;


  while(1){
    if(blueToothSerial.available()){//check if there's any data sent from the remote bluetooth shield
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);


      switch (recvChar) {
        case 'z':          
          // Avance
          digitalWrite(LED_PIN, HIGH);
          for (int speed = 0; speed <= 300; speed++) {
            motors.setLeftSpeed(speed);
            motors.setRightSpeed(speed);
            delay(2);
          }
          break;
        
        case 's':          
          // Reculer
          digitalWrite(LED_PIN, HIGH);
          for (int speed = 0; speed >= -300; speed--) {
            motors.setLeftSpeed(speed);
            motors.setRightSpeed(speed);
            delay(2);
          }
          break;
        
        case 'd':          
          // Tourner à droite
          digitalWrite(LED_PIN, HIGH);
          for (int speed = 0; speed <= 200; speed++) {
            motors.setLeftSpeed(speed);            
            delay(2);
          }
          break;

        case 'q':          
          // Tourner à gauche
          digitalWrite(LED_PIN, HIGH);
          for (int speed = 0; speed <= 200; speed++) {
            motors.setRightSpeed(speed);
            delay(2);
          }
          break;

        case 'w':          
          Serial.print("headservo");                
        break;

        case 'x':          
          Serial.print("headservo");                
        break;

        case 'c':          
          Serial.print("clawservo");                
        break;

        case 'v':          
          Serial.print("clawservo");                
          break;

        default:
          digitalWrite(LED_PIN, LOW);
          motors.setLeftSpeed(0);
          motors.setRightSpeed(0);
          break;
      }

    }

    if(Serial.available()){//check if there's any data sent from the local serial terminal, you can add the other applications here
      recvChar  = Serial.read();
      blueToothSerial.print(recvChar);
    }

  }

  
  delay(500);
}

void setupBlueToothConnection()
{
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