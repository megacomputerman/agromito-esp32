#include "DHT.h"

#define MODE_MANUAL 0
#define MODE_AUTO   1
#define LED_BUILTIN 2

//Added temperature sensor
#define DHT_PIN 21
#define DHTTYPE DHT11

int ledPin = 23;
int ledChannel = 0;
int ledValue = 10;
int ledMode = MODE_MANUAL;

int ldrPin = 36;
int ldrMax = 4000;

int thresholdValue = 50;

//create an instance of DHT sensor
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    for (int i=9; i>=0; i--) {
        digitalWrite(LED_BUILTIN, i % 2 ? HIGH : LOW);
        delay(50);
    }

    ledcSetup(1, 5000, 8);
    ledcAttachPin(LED_BUILTIN, 1); // Led Builtin aceita PWM no ESP32
    
    pinMode(ledPin, OUTPUT);
    ledcSetup(ledChannel, 5000, 8);
    ledcAttachPin(ledPin, ledChannel);
    
    //call begin to start sensor
    dht.begin();
    Serial.printf("DBG agromito Initialized.\n");
}

 

void loop() {
    String serialCommand;

    while (Serial.available() > 0) {
        char serialChar = Serial.read();
        serialCommand += serialChar; 

        if (serialChar == '\n') {
            processCommand(serialCommand);
            serialCommand = "";
        }
    }

    ledUpdate();
    delay(100);
}


void processCommand(String command) {
    command.trim();
    command.toUpperCase();

    // Serial.println("DBG Received command: " + command);

    if (command.startsWith("SET_LED ")) {
        int ledTmp = command.substring(8).toInt();
        if (ledTmp >= 0 && ledTmp <= 100) {
            ledValue = ledTmp;
            ledMode  = MODE_MANUAL;
            ledUpdate();
            Serial.printf("RES SET_LED 1\n");
        }
        else {
            Serial.printf("RES SET_LED -1\n");
        }
    }
    else if (command.startsWith("SET_THRESHOLD ")) {
        int thresholdTmp = command.substring(14).toInt();
        if (thresholdTmp >= 0 && thresholdTmp <= 100) {
            thresholdValue = thresholdTmp;
            ledMode = MODE_AUTO;
            ledUpdate();
            Serial.printf("RES SET_THRESHOLD 1\n");
        }
        else {
            Serial.printf("RES SET_THRESHOLD -1\n");
        }
    }

    else if (command == "GET_LDR")
      Serial.printf("RES GET_LDR %d\n", ldrGetValue());
    //get temp
    else if (command == "GET_TEMP"){
        float myt = dht.readTemperature();
        int mytt = round(myt);
        Serial.printf("RES GET_TEMP %d\n", mytt);
    }
    else if (command == "GET_UMD"){
        float myu = dht.readHumidity();
        int myuu = round(myu);
        Serial.printf("RES GET_UMD %d\n", myuu);
    }
    else if (command == "GET_LED")
      Serial.printf("RES GET_LED %d\n", ledValue);
    else if (command == "GET_THRESHOLD")
      Serial.printf("RES GET_THRESHOLD %d\n", thresholdValue);
    
    else
      Serial.println("ERR Unknown command.");
      
}


void ledUpdate() {
    if (ledMode == MODE_MANUAL || (ledMode == MODE_AUTO && ldrGetValue() < thresholdValue)) {
        ledcWrite(ledChannel, 255.0*(ledValue/100.0));
        ledcWrite(1, 255.0*(ledValue/100.0));
    }
    else {
        ledcWrite(ledChannel, 0);
        ledcWrite(1, 0);
    }
}

int ldrGetValue() {
    int ldrAnalog = analogRead(ldrPin);
    int ldrValue = 100*ldrAnalog/ldrMax;

    // Serial.printf("DBG LDR_MAX=%d, LDR_ANALOG=%d, LDR_VALUE=%d\n", ldrMax, ldrAnalog, ldrValue);

    return ldrValue > 100 ? 100 : ldrValue;
}
