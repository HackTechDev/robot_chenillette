/*
 * This example uses the ZumoMotors library to drive each motor on the Zumo
 * forward, then backward. The yellow user LED is on when a motor should be
 * running forward and off when a motor should be running backward. If a
 * motor on your Zumo has been flipped, you can correct its direction by
 * uncommenting the call to flipLeftMotor() or flipRightMotor() in the setup()
 * function.
 */

#include <Wire.h>
#include <ZumoShield.h>
#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 6
#define TxD 7


#define LED_PIN 13

SoftwareSerial blueToothSerial(RxD, TxD);

ZumoMotors motors;

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();

  // uncomment one or both of the following lines if your motors' directions need to be flipped
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);
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
          // run left motor forward

          digitalWrite(LED_PIN, HIGH);

          for (int speed = 0; speed <= 400; speed++)
          {
            motors.setLeftSpeed(speed);
            delay(2);
          }

          for (int speed = 400; speed >= 0; speed--)
          {
            motors.setLeftSpeed(speed);
            delay(2);
          }
          break;
        
        case 's':
          // run left motor forward

          digitalWrite(LED_PIN, HIGH);

          for (int speed = 0; speed <= 400; speed++)
          {
            motors.setLeftSpeed(speed);
            motors.setRightSpeed(speed);
            delay(2);
          }

          
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
  blueToothSerial.print("\r\n+STNA=SeeedBTSlave\r\n"); //set the bluetooth name as "SeeedBTSlave"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  blueToothSerial.flush();
}