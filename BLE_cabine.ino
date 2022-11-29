

#include <SoftwareSerial.h>   //Software Serial Port


    #define VERSION v1.0
    
   // #define ADDRESS 0x640101ff   // ID лифта
    #define LED 13   // светодиод arduino
    #define SEC 1000   // секунда
    #define DELAY_SEC SEC * 3 // Задержка в секундах для реле кнопок
    #define DEBUG_ENABLED  1   // debug (on/off)
    #define WAIT_RESPONSE 1500  // Время ответа от модуля в миллисекундах
    #define BLE_TX 13
    #define BLE_RX 12

    #define close_pin 0
    #define open_pin 0
    #define cancel_pin 0
    #define call_pin 0

 

//  ПЕРЕМЕННЫЕ НАСТРАИВАЕМЫЕ ИНДИВИДУАЛЬНО  ////////////////////////////////////////////

    const unsigned long int address = 0x640101ff; // ID лифта
    const byte max_floors = 10;

    // Пины на которые подключны реле
    const byte button_pins[max_floors] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    const byte extra_button_pins[4] = {close_pin, open_pin, cancel_pin, call_pin};
    


    

// For immitation getCurrentFloor();
int but = 0;
// End immitation



//   НЕ МЕНЯТЬ ДАЛЬНЕЙШИЕ ПЕРЕМЕННЫЕ   //////////////////////////////////

    SoftwareSerial BLE(BLE_RX, BLE_TX);
     
    bool isLedOn = false;


    // Время ответа от модуля
    unsigned long response_times;
    
    // Время сколько включена реле
    unsigned long times[max_floors + 4];
    
    // Состояние включенных реле
    bool state_switch[max_floors + 4];
    
    
    // Этаж на котором находится лифт в настоящее время
    byte current_floor = 1;

    // Массив отложенных вызовов
    bool stack[max_floors][max_floors];

    // Хранит данные получаемые с BLE модулей
    String dataBLE;
    String queue;

    // Для итераций по циклам
    byte j = 0;

    char recvChar;
    String dataSerial = "";
    int count_ble_response = 0;
    
// Functions ///////////////////////////////////////////////

void setupBleConnection();
int chToInt(char in);
int getCurrentFloor();
void readData(SoftwareSerial& serial, String& data);
void setCommand(SoftwareSerial& serial, String& cmd);
void lift(byte fl);

void startSerial(SoftwareSerial& bluetooth){
  bluetooth.begin(9600);
}


//  SETUP  ////////////////////////////////////////////////////
    void setup()
    {
      Serial.begin(9600);
      BLE.begin(9600);
      
      //delete in release (it's for imitation)
      pinMode(4, INPUT);
      // 
      response_times = 0;
      
      
      
      // Set RX BLE pins input mode
      pinMode(BLE_RX, INPUT);

      // Set TX BLE pins output mode
      pinMode(BLE_TX, OUTPUT);
       
      dataBLE = "";
      queue = "";

      // очищаем таблицу отложенных вызовов
      for(auto& i : stack){
        for(bool& j : i){
          j = false;
        }
      }
      // обнуляем таблицу состояний кнопок
      for(bool& i : state_switch){
        i = false;
      }
      // обнуляем таблицу таймеров активации кнопок
      for(unsigned long& i : times){
        i = 0;
      }
      // переводим пины кнопок в режим выхода
      for(const byte& i : button_pins){
        pinMode(i, OUTPUT);
      }
      // переводим пины дополнительных кнопок в режим выхода
      for(const byte& i : extra_button_pins){
        pinMode(i, OUTPUT);
      }
      
//      pinMode(LED, OUTPUT);
      setupBleConnection();

      Serial.println("Started.");
    }








//  LOOP   ////////////////////////////////////////////////////////////
    void loop()
    {
        // Устанавливаем в current_floor, текущий этаж
//        getCurrentFloor();

        // Проверяем какие кнопки нажаты слишком долго
        for(int i = 0; i < max_floors + 4; i++){
          if(times[i] != 0){
            if(abs(millis() - times[i]) > DELAY_SEC){
              times[i] = 0;
              digitalWrite(button_pins[i], LOW);
              state_switch[i] = false;
            }
          }
        }
        
        // Читаем данные с Bluetooth Low Energy модуля
        readData(BLE, dataBLE);

        // Запускаем отложенные вызовы текущего этажа
//        for(int i = 0; i < max_floors; i++){
//          if(stack[current_floor][i]){
//            lift(i);
//            stack[current_floor][i] = false;
//          }
//        }
        
        // Читаем данные с компьютера
        while(Serial.available()){//check if there's any data sent from the local serial terminal, you can add the other applications here
          recvChar  = Serial.read();
          //BLE1.print(recvChar);
          if(recvChar != '\r' && recvChar != '\n'){
            dataSerial += recvChar;
          } else {
            int ble = 0;
            if(dataSerial.length() > 2 && dataSerial[1] == '/'){
              Serial.print("Set ble: ");
//              ble = dataSerial[0].toInt();
              ble = chToInt(dataSerial[0]);
              Serial.println(ble);
              dataSerial.remove(0, 1);
            }
            if(dataSerial.length() > 1 && dataSerial[0] == '/'){
              dataSerial.remove(0, 1);
              dataSerial = "at+" + dataSerial;
            }
            dataSerial += "\r\n";
            Serial.print("Send to ");
            Serial.print(String(ble) + ": " + dataSerial);
            BLE.print(dataSerial);
            dataSerial = "";
          }
        }
    }







//   ФУНКЦИИ И ПРОЦЕДУРЫ   ////////////////////////////////////////////////////////
    

    // Считывает данные с Bluetooth Low Energy в строку
    void readData(SoftwareSerial& serial, String& data){
      char ch;
      if(serial.available()){
//        Serial.print("millis() = ");
//        Serial.print(millis());
//        Serial.print(", ");
        response_times = millis();
//        Serial.print("Have ");
        ch = serial.read();
//        Serial.println(ch);
        if(ch == '-'){
          if(data.length() > 0){
            if(data[data.length() - 1] == '/'){
              data.remove(data.length() - 1, 1);
              Serial.print("Received from ");
              Serial.print(j);
              Serial.print(" (code): '");
              Serial.print(data);
              Serial.println("'");
              setCommand(serial, data);
              data = "";
            }
          }
//        }
//        Serial.println(ch);
//        Serial.println(ch + 0);
//        if(ch == '/' || ch == '+'){
//          setCommand(serial, data);
//          Serial.print(data);
//          data = "";
        } else {
          data += ch;
        }
      } else {
        unsigned long t = millis();
        if(abs(t - response_times) > WAIT_RESPONSE){
          if(data != ""){
//            Serial.print("response_times = ");
//            Serial.println(t-response_times);
//            
            Serial.print("Received from ");
            Serial.print(j);
            Serial.print(" (time): '");
            Serial.print(data);
            Serial.println("'");
            setCommand(serial, data);
            data = "";
          }
        }
      }
    }


    // Исполняет команду полученную с управляющего устройства
    void setCommand(SoftwareSerial& serial, String& cmd){
      String command = "";
      int first = 0;
      int second = 0;
      int sw = 0; // switch

      // get command and options
      for(auto i : cmd){
        if(i != '_' && sw == 0){
          command += i;
        } else if(i != '_' && sw == 1){
          first *= 10;
          first += chToInt(i);
        } else if(i != '_' && sw == 2){
          second *= 10;
          second += chToInt(i);
        } else {
          sw++;
        }
      }

      // 
      if((command.substring(0, 7) == "OK+CONN")){ // ||(command.substring(0, 7) == "OK+LOST"
        command = command.substring(7);
      }
      first--;
      second--;
      Serial.println("command = " + command);

      if(command == "getAddress"){
        serial.print("addr");
        serial.print(address, HEX);
        Serial.println(address, HEX);
//        BLE[j].print(address, HEX);
      } else if (command == "getMaxFloors"){
//        BLE[j].print(max_floors);
        serial.print("flr");
        serial.print(max_floors);
        Serial.println(max_floors);
      } else if (command == "speed") {
        serial.print("speed");
        Serial.println("speed");
      } else if (command == "liftto"){
        if(second && current_floor != first){
          stack[first][second] = true; 
        } else {
          lift(second);
        }
       } else if (command == "lift"){
        lift(first);
       } else if (command == "extrabuttons"){
        byte extr = 0;
        if(extra_button_pins[0] > 0) extr += 1; // close
        if(extra_button_pins[1] > 0) extr += 2; // open
        if(extra_button_pins[2] > 0) extr += 4; // cancel
        if(extra_button_pins[3] > 0) extr += 8; // call
        serial.print(extr);
       } else if (command == "pushbutton"){
        digitalWrite(extra_button_pins[first - 1], HIGH);
        times[max_floors + first - 1] = millis();
        if(times[max_floors + first - 1] == 0) times[max_floors + first - 1]++;
        state_switch[max_floors + first - 1] = true;
       }
    }

    
    void setupBleConnection()
    {
//      BLE1.begin(9600); //Set BLE BaudRate to default baud rate 9600
//      BLE2.begin(9600);
//      BLE.print();
      //BLE.println("AT+CLEAR"); //clear all previous setting
      //BLE.println("AT+ROLE0"); //set the bluetooth name as a slaver
      //BLE.println("AT+SAVE1");  //don't save the connect information
      // mode 2
      // role 0
      // save 0
      // ibe0 0x00001101
    }

    // Конвертирует char to int
    int chToInt(char in){
      String dt = "";
      dt += in;
      return dt.toInt();
    }

    int getCurrentFloor(){
      int res = current_floor;
      res = digitalRead(4);
//      if (res == 2){
//        res--;
//      } else {
//        res++;
//      }
//      if(res > 2){
//        res = 1;
//      }
      return res + 1;
    }


    // Активация кнопки кабины
    void lift(byte fl){
      Serial.print("Button cabine is ON (");
      Serial.print(fl);
      Serial.println(")");
      if(fl < 0 || fl >= max_floors) return;
      digitalWrite(button_pins[fl], HIGH);
      state_switch[fl] = true;
      times[fl] = millis();
      if(times[fl] == 0) times[fl]++;
    }






    
