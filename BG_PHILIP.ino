#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>


#define FIREBASE_HOST "main-station-logger-default-rtdb.firebaseio.com"
#define WIFI_SSID "Server Project"
#define WIFI_PASSWORD "Master75wew"
String dataIn;
String dt[10];
int i;
boolean parsing = false;


void setup() {

  Serial.begin(9600);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST);
  dataIn = "";

  Firebase.setInt("Amonia/nilai", 260);

}


void loop() {

  if (Serial.available() > 0)
  {
    char inChar = (char)Serial.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }
  if (parsing)
  {
    parsingData();
    parsing = false;
    dataIn = "";
  }

}



void parsingData()
{
  int j = 0;
  //kirim data yang telah diterima sebelumnya
  Serial.print("data masuk : ");
  Serial.print(dataIn);
  Serial.print("\n");
  //inisialisasi variabel, (reset isi variabel)
  dt[j] = "";
  //proses parsing data
  for (i = 1; i < dataIn.length(); i++)
  {
    //pengecekan tiap karakter dengan karakter (#) dan (,)
    if ((dataIn[i] == '#') || (dataIn[i] == ','))
    {
      //increment variabel j, digunakan untuk merubah index array penampung
      j++;
      dt[j] = ""; //inisialisasi variabel array dt[j]
    }
    else
    {
      //proses tampung data saat pengecekan karakter selesai.
      dt[j] = dt[j] + dataIn[i];
    }
  }
  //kirim data hasil parsing
  Serial.print("data 1 : ");
  Serial.print(dt[0].toInt());
  Firebase.setInt("Amonia/nilai", dt[0].toInt());
  Serial.print("\n");
  Serial.print("data 2 : ");
  Serial.print(dt[1].toInt());
  Firebase.setInt("Keruh/nilai", dt[1].toInt());
  Serial.print("\n");
  Serial.print("data 3 : ");
  Serial.print(dt[2].toInt());
  Firebase.setInt("Oxygen/nilai", dt[2].toInt());
  Serial.print("\n");
  Serial.print("data 4 : ");
  Serial.print(dt[3].toInt());
  Firebase.setInt("Ph/nilai", dt[3].toInt());
  Serial.print("\n\n");
}


