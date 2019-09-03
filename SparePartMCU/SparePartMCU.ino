#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SoftwareSerial.h>

LiquidCrystal_PCF8574 lcd(0x27);
SoftwareSerial rfid = SoftwareSerial(D5, D6);
 
/* Set these to your desired credentials. */
const char *ssid = "HOMENET";  //ENTER YOUR WIFI SETTINGS
const char *password = "0123456789";
 
//Web/Server address to read/write from 
const char *host = "suspos.site";

//Setting Destination Data
const String destination = "http://suspos.site/api/v1/getlistbarangpesanan.php";

//Variabel RFID
byte msg[10];
byte c;

//
int redPin = D8;
int yellowPin = D7;
int greenPin = D6;
 
//=======================================================================
//                    Power on setup
//=======================================================================
 
void setup() {
  delay(1000);
  Serial.begin(115200);
  rfid.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  //Deklarasi PIN
  //Pasang Mode PIN sebagai Input
  Wire.begin();
  Wire.beginTransmission(0x27); //Your LCD Address
  
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
 
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(255);
  lcd.home(); 
  lcd.clear();
  lcd.print("Selamat Datang");  
}
 
//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  // Membaca data sensor DHT11
  controldevice();
  delay(1000);  //Get Data at every 1 seconds
}

void controldevice(){
  HTTPClient http;    //Declare object of class HTTPClient
  
  String url = destination;
  
  http.begin(url);       //Specify request destination
 
  int httpCode = http.GET();   //Send the request
  if (httpCode == 200) {
      // Parsing
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(yellowPin, HIGH);
      const size_t bufferSize = 6*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 330;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      // Parameters
      int i = 1;
      while(i<=root.size() && root.size() != 0) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, LOW);
        digitalWrite(yellowPin, HIGH);
        JsonObject& data = root[String(i)];
        const String idRFID = data["idRFID"];
        const char* namaBarang = data["namaBarang"];
        bool error = root["error"];
        String datacard = "";
        int k = 0;
        // Output to LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Nama Barang : ");
        lcd.setCursor(0, 1);
        lcd.print(namaBarang);
        if (rfid.available()) {
          while(rfid.available()){
            c=rfid.read(); 
            msg[k] = c;
            k++;
          }
          datacard += "000";
          if (String(msg[3], HEX) == "0") {
            unsigned long result = 
                ((unsigned long int)msg[3]<<24) + 
                ((unsigned long int)msg[4]<<16) + 
                ((unsigned long int)msg[5]<<8) + 
                msg[6];
            datacard += String(result);
          } else if (String(msg[4], HEX) == "0") {
            unsigned long result = 
                ((unsigned long int)msg[4]<<24) + 
                ((unsigned long int)msg[5]<<16) + 
                ((unsigned long int)msg[6]<<8) + 
                msg[7];
            datacard += String(result);
          }     
          Serial.print("ID Kartu : ");
          Serial.println(datacard);
          if(datacard == idRFID) {
              digitalWrite(redPin, LOW);
              digitalWrite(greenPin, HIGH);
              digitalWrite(yellowPin, LOW);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Barang Cocok ");
              i++;
           } else {
              digitalWrite(redPin, HIGH);
              digitalWrite(greenPin, LOW);
              digitalWrite(yellowPin, LOW);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Barang ");
              lcd.setCursor(0, 1);
              lcd.print("Tidak Cocok ");          
           }
        }
        delay(5000);      
      }
      if (root.size() == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Belum ada barang");
        lcd.setCursor(0, 1);
        lcd.print("yang dicek ");      
      } else if (root.size() != 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pengecekan");
        lcd.setCursor(0, 1);
        lcd.print("barang selesai ");
        delay(5000);      
      }  
  }
  http.end();  //Close connection
}
