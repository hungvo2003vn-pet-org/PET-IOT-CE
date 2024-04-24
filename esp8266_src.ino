#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <ESP_EEPROM.h>
#include <HX711_ADC.h> // https://github.com/olkal/HX711_ADC
#include <LiquidCrystal_I2C.h> // LiquidCrystal_I2C library
#define ZERO_BUTTON D4

#define DT    D5
#define SCK   D6

// register step
bool registered =  false;
//

// motor control
const int dirPin = D3;
const int stepPin = D7;
const int speed = 1000;
bool high = true;
unsigned long timeForSpeed = micros();
//

HX711_ADC LoadCell(DT, SCK); // dt pin, sck pin
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD HEX address 0x27

//----MQTT server info---------------
const char* mqtt_server = "195b0bd79f294349b31afa3e976cd85b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "trucle69"; //User
const char* mqtt_password = "Mqtttrucle034569@"; //
String station_id = "";
int feed_amount = 0;
//--------------------------------------------------
WiFiClientSecure espClient;
PubSubClient client(espClient);


unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wifi");
  WiFiManager wifiManager;
  // wifiManager.resetSettings();    //Uncomment this to wipe WiFi settings from EEPROM on boot.  Comment out and recompile/upload after 1 boot cycle.
  wifiManager.autoConnect("ESP8266");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_lcd()
{
  lcd.begin(16,2); // begins connection to the LCD module
  lcd.init();
  lcd.backlight(); // turns on the backlight
  lcd.setCursor(4, 0); // set cursor to first row
  lcd.print("WELCOME "); // print out to LCD
  lcd.setCursor(0, 1); // set cursor to first row
  lcd.print(" PET IOT PROJECT "); // print out to LCD
  delay(3000);
  lcd.clear();
}

void spin_motor(int &current_weight)
{
  if (current_weight < feed_amount)
  {
    digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
    // Makes 200 pulses for making one full cycle rotation
    for(int x = 0; x < 100; x++) {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(speed);
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(speed); 
    }
    delay(700); // One second delay
    
    digitalWrite(dirPin,LOW); //Changes the rotations direction
    // Makes 400 pulses for making two full cycle rotation
    for(int x = 0; x < 100; x++) {
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(speed);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(speed);
    }
    delay(700);
  }
  else
  {
    feed_amount = 0;
  }
}

//-----Call back Method for Receiving MQTT massage---------
void callback(char* topic, byte* payload, unsigned int length) {
  String incomming = "";

  if(strcmp(topic, "station/register/server/response") == 0  && station_id == "")
  {  
    for(int i=0; i<length;i++) incomming += (char)payload[i];
      // Parse the JSON string
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, incomming);

    // Check for parsing errors
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    Serial.println("Message arived ["+String(topic)+"]"+String(doc["id"]));
    station_id = String(doc["id"]);
  }
  else if(strcmp(topic, "feedRecord/start/server/send") == 0)
  {
    for(int i=0; i<length;i++) incomming += (char)payload[i];

    DynamicJsonDocument doc_rcv(1024);
    DeserializationError error = deserializeJson(doc_rcv, incomming);

    // Check for parsing errors
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }


    Serial.println("Massage arived ["+String(topic)+"]"+String(doc_rcv["feed_amount"]));
    feed_amount = int(doc_rcv["feed_amount"]);

    LoadCell.update(); // retrieves data from the load cell
    int i = LoadCell.getData(); // get output value

    DynamicJsonDocument doc(1024);
    doc["start_amount"] = i;
    doc["box_remain"] = 0.4;
    doc["feed_amount"] = feed_amount;

    char mqtt_message[128];
    serializeJson(doc,mqtt_message);
    publishMessage("feedRecord/start/device/response", mqtt_message, true);
  } else if(strcmp(topic, "station/check/server") == 0)
  {
    registered = true;
    lcd.clear();
  }
}



//-----Method for Publishing MQTT Messages---------
void publishMessage(const char* topic, String payload, boolean retained){
  if(client.publish(topic,payload.c_str(),true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}


//------------Connect to MQTT Broker-----------------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientID =  "ESPClient-";
    clientID += String(random(0xffff),HEX);
    if (client.connect(clientID.c_str(), mqtt_username, mqtt_password)) {
      client.subscribe("station/register/server/response");
      client.subscribe("station/check/server");
      client.subscribe("feedRecord/start/server/send");

      //-------Send register signal------
      DynamicJsonDocument doc(1024);  
      doc["register"] = 1;
      char mqtt_message[128];
      serializeJson(doc,mqtt_message);
      publishMessage("station/register/device/send", mqtt_message, true);
      //---------------------------------
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void display_on_lcd(int data)
{
  lcd.setCursor(1, 0); // set cursor to first row
  lcd.print("REMAINING FOOD "); // print out to LCD 

  if (data<0)
  {
    data = 0;
    lcd.setCursor(4, 1);
    lcd.print(" ");
  }
  else
  {
    lcd.setCursor(4, 1);
    lcd.print(" ");
  }
  
  lcd.setCursor(5, 1); // set cursor to secon row
  lcd.print(data, 1); // print out the retrieved value to the second row
  lcd.print("g ");

  if (data>=5000)
  {
    data=0;
    lcd.setCursor(0, 0); // set cursor to secon row
    lcd.print("  Over Loaded   "); 
    delay(200);
  }

  if (digitalRead (ZERO_BUTTON) == LOW)
  {
    lcd.setCursor(0, 1); // set cursor to secon row
    lcd.print("   Taring...    ");
    LoadCell.start(1000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
}

void setup() {
  EEPROM.begin(512);
  Serial.begin(115200);
  while(!Serial) delay(1);

  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  LoadCell.begin(); // start connection to HX711
  LoadCell.start(1000); // load cells gets 1000ms of time to stabilize

  /////////////////////////////////////
  LoadCell.setCalFactor(480); // Calibarate your LOAD CELL with 100g weight, and change the value according to readings
  /////////////////////////////////////
  setup_lcd();

  // Declare pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

unsigned long timeUpdata=millis();
unsigned long timeConnect=millis();

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  //read ADC_HX711
  LoadCell.update(); // retrieves data from the load cell
  int data = LoadCell.getData(); // get output value
  if(data < 0) data = 0;

  if(station_id != "")
  {
    if(!registered)
    {
      // lcd.clear();
      lcd.setCursor(1, 0); // set cursor to first row
      lcd.print("DEVICE'S ID: "); // print out to LCD

      lcd.setCursor(0, 1); // set cursor to first row
      lcd.print(station_id); // print out to LCD 
    } else {
      display_on_lcd(data);
      if(data < feed_amount)
      {
          if(micros() - timeForSpeed > speed)
          {
            high = high ^ 1;
            digitalWrite(stepPin, high);
            timeForSpeed = micros();
          }
      } else {
        feed_amount = 0;
      }
    }
  }



  if(millis()-timeUpdata > 5000 && station_id != ""){

    DynamicJsonDocument doc(1024);
    doc["disk_remain"] = data;
    doc["station_id"] = station_id;
    doc["box_remain"] = 0.4;
    char mqtt_message[128];
    serializeJson(doc,mqtt_message);
    publishMessage("station/info", mqtt_message, true);

    // String mqtt_message = "Food remaining: "+ String(i)+" g";
    timeUpdata=millis();
  }
}
