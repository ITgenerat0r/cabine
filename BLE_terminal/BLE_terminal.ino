#include <SoftwareSerial.h>

#define RX_BLE_PIN 13
#define TX_BLE_PIN 12

SoftwareSerial ble(RX_BLE_PIN, TX_BLE_PIN);

void setup() {
  pinMode(RX_BLE_PIN, INPUT);
  pinMode(TX_BLE_PIN, OUTPUT);

  ble.begin(9600);
  Serial.begin(9600);
  Serial.println("Start");
}

char recvChar;
String dataSerial = "";


void loop() {
//  ble.begin(9600);
  if(ble.available()){
    Serial.write(ble.read());
  }


  while(Serial.available()){//check if there's any data sent from the local serial terminal, you can add the other applications here
          recvChar  = Serial.read();
          //BLE1.print(recvChar);
          if(recvChar != '\r' && recvChar != '\n' && recvChar != '-'){
            dataSerial += recvChar;
          } else {
            if(dataSerial.length() > 1 && dataSerial[0] == '/'){
              dataSerial.remove(0, 1);
              dataSerial = "AT+" + dataSerial;
            }
//            dataSerial += "\r\n";
            Serial.println("");
            Serial.println(dataSerial);
            ble.print(dataSerial);
            dataSerial = "";
          }
        }
}
