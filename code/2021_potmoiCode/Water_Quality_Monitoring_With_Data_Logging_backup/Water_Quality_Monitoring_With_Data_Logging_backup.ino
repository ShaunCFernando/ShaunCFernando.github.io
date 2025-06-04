#include <Blynk.h>
#include "RTClib.h"
#include "SD.h"
#include <Wire.h>
#include <OneWire.h>
#include <GravityTDS.h>
#include <EEPROM.h>
#include <DallasTemperature.h>
#include<LiquidCrystal.h>
#define Offset 0.00            //deviation compensate for pH
#define PH A2
#define ONE_WIRE_BUS A0
#define TDS A1
#define Pushbutton 10
#define Turbidity A3
#define Backlight 11
#define LOG_INTERVAL  1000 // mills between entries
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup() 
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
unsigned long int avgValue;    //Store the average value of pH sensor feedback
const int RS = 7, E = 6, D4 = 4, D5 = 5, D6 = 8, D7 = 9;
const int chipSelect = 10;      // for the data logging shield, we use digital pin 10 for the SD cs line
int buttonVal = 1;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
GravityTDS gravityTds;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;
float temperature = 25;
float tdsValue=0;
float volt;
float ntu;

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

uint32_t syncTime = 0; // time of last sync()
RTC_DS1307 RTC; // define the Real Time Clock object
File logfile;   // the logging file

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
    
  while(1);
}

void setup()
{ 
  Serial.begin(9600);
  Serial.println();
  RTC.adjust(DateTime(2021, 2, 14, 7, 21, 0));
  #if WAIT_TO_START
    Serial.println("Type any character to start");
    while (!Serial.available());
  #endif //WAIT_TO_START    // initialize the SD card
  Serial.print("Initializing SD card...");
  sensors.begin();
  gravityTds.setPin(TDS);
  gravityTds.setAref(5.0); //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024); //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin(); //initialization
 
  lcd.begin(20, 4);           //Sets up the LCD's number of columns and rows
  lcd.clear();                //Clears any existing data on LCD

  pinMode(Pushbutton, INPUT_PULLUP);
  pinMode(ONE_WIRE_BUS, INPUT);       //Temperature sensor data pin
  pinMode(TDS, INPUT);        //TDS sensor data pin
  pinMode(PH, INPUT);         //pH sensor data pin
  pinMode(10, OUTPUT);       // make sure that the default chip select pin is set to output, even if you don't use it:

// see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    Serial.println("");
    Serial.println("Date & Time                  Temp (In Fahrenheit)          pH          TDS (In PPM)      Turbidity (In NTU)");
    return;
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "WQDATA00.txt";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);  

Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif 
  }

  logfile.println("Date & Time                  Temp (In Fahrenheit)          pH          TDS (In PPM)      Turbidity (In NTU)");    
#if ECHO_TO_SERIAL
  Serial.println("Date & Time                  Temp (In Fahrenheit)          pH          TDS (In PPM)      Turbidity (In NTU)");
#endif ECHO_TO_SERIAL
//#if ECHO_TO_SERIAL// attempt to write out the header to the file
  //if (logfile.writeError || !logfile.sync()) {
  //  error("write header");
  //}
//#endif ECHO_TO_SERIAL
  
}

void loop()
{
  DateTime now = RTC.now();
 
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
 
  // log milliseconds since starting
 // uint32_t m = seconds();
 // logfile.print(m);           // milliseconds since start
  //logfile.print(", ");    
//#if ECHO_TO_SERIAL
 // Serial.print(m);         // milliseconds since start
  //Serial.print("                  ");  
//#endif 
 
  // fetch the time
  // log time
  //logfile.print(now.unixtime()); // seconds since 1/1/1970
 // logfile.print(", ");
  //logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" (");
  logfile.print(daysOfTheWeek[now.dayOfTheWeek()]);
  logfile.print(") ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print("          ");
#if ECHO_TO_SERIAL
  //Serial.print(now.unixtime()); // seconds since 1/1/1970
  //Serial.print(", ");
  //Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print("          ");
#endif //ECHO_TO_SERIAL

  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  lcd.setCursor(0,2);         //Sets cursor to point (0,4)
  lcd.print("Temperature: "); // Prints Temperature to LCD
  lcd.print(Fahrenheit);//Prints value of Temperature sensor to LCD
  delay(100);
  
  float phValue=(float)analogRead(PH)*5.0/1024; //convert the analog into millivolt
  phValue=3.5*phValue+Offset;                   //convert the millivolt into pH value
  lcd.setCursor(0,0);                           //Sets cursor to point (0,0)
  lcd.print("pH Level: ");                      //Prints pH Level: to LCD
  lcd.print(phValue);                           //Prints value of pH sensor to LCD
  delay(100);

  gravityTds.update(); //calculation done here from gravity library
  tdsValue = gravityTds.getTdsValue(); // then get the TDS value  
  lcd.setCursor(0,1);         //Sets cursor to point (0,2)
  lcd.print("TDS Level: ");   //Prints TDS Level: to LCD
  lcd.print(tdsValue,0); //Prints value of TDS sensor to LCD
  lcd.print("ppm");
  delay(100);

  volt = 0;
  for(int i=0; i<800; i++)
  {
      volt += ((float)analogRead(Turbidity)/1023)*5;
  }
  volt = volt/800;
  volt = round_to_dp(volt,2);
  if(volt < 2.5){
  ntu = 3000;
  }else{
    //ntu = -1120.4*square(volt)+5742.3*volt-4353.9; 
   ntu = (volt - 3.9994)/0.0008;
  }

  lcd.setCursor(0,3);
  lcd.print("Turbidity: ");
  lcd.print(ntu);
  delay(100);

  //Serial.print(Fahrenheit);
  //Serial.print("                            ");
  //Serial.print(phValue);
  //Serial.print("           ");
  //Serial.print(tdsValue, 0);
 // Serial.print("                 ");
  //Serial.println(ntu);
  //delay(2000);

//logfile.print(", ");    
  logfile.print(Fahrenheit);
  logfile.print("                 ");    
  logfile.print(phValue);
  logfile.print("               ");
  logfile.print(tdsValue, 0);
  logfile.print("               ");
  logfile.print(ntu);
#if ECHO_TO_SERIAL
  //Serial.print(", ");   
  Serial.print(Fahrenheit);
  Serial.print("                 ");
  Serial.print(phValue);
  Serial.print("               ");
  Serial.print(tdsValue, 0);
  Serial.print("               ");
  Serial.println(ntu);
#endif //ECHO_TO_SERIAL

logfile.println();
#if ECHO_TO_SERIAL
  //Serial.println();
#endif // ECHO_TO_SERIAL  

if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

logfile.flush();
}
