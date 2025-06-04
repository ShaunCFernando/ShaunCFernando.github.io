
#include <DallasTemperature.h>          //Library for DHT sensor
#include <SPI.h>                        //Serial library for SD card shield
#include <SD.h>                         //SD card library

#define ONE_WIRE_BUS A0
OneWire oneWire(ONE_WIRE_BUS);              //create instance of DHT called dht
DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;

const int chipSelect = 5;               //chip select on digital 4 for SD card shield

void setup() {

  Serial.begin(9600);                   //begin serial comms
  while (!Serial) {
    ;                                   // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

                                        // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

   
}

void loop() {
  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  delay(100);
    
                                        // make a string for assembling the data to log:
  String dataString = "";

  dataString += String(Fahrenheit);

                                        // open the file. note that only one file can be open at a time,
                                        // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

                                        // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
                                        // print to the serial port too:
    Serial.println(dataString);
  }
                                        // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
