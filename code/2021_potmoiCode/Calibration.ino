
#include <EEPROM.h>
#include "GravityTDS.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include<LiquidCrystal.h>
#define Offset 0.00            //deviation compensate for pH
#define PH A2
#define ONE_WIRE_BUS A0
#define TDS A1
unsigned long int avgValue;    //Store the average value of pH sensor feedback
const int RS = 7, E = 6, D4 = 10, D5 = 11, D6 = 8, D7 = 9;
int Turbidity = A3;
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
  
void setup()
{ 
  Serial.begin(9600);
  Serial.println("Temperature (In Fahrenheit)       pH       TDS (In PPM)       Turbidity (In NTU)");
  Serial.println("");
  sensors.begin();
  gravityTds.setPin(TDS);
  gravityTds.setAref(5.0); //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024); //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin(); //initialization
 
  lcd.begin(20, 4);           //Sets up the LCD's number of columns and rows
  lcd.clear();                //Clears any existing data on LCD
  
  pinMode(ONE_WIRE_BUS, INPUT);       //Temperature sensor data pin
  pinMode(TDS, INPUT);        //TDS sensor data pin
  pinMode(PH, INPUT);         //pH sensor data pin
}
void loop()
{
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
  //if(volt < 2.5){
     //ntu = 3000;
  //}else{
    ntu = -1120.4*square(volt)+5742.3*volt-4353.8; 
  

  lcd.setCursor(0,3);
  lcd.print("Turbidity: ");
  lcd.print(ntu);
  delay(100);

  Serial.print(Fahrenheit);
  Serial.print("                            ");
  Serial.print(phValue);
  Serial.print("           ");
  Serial.print(tdsValue, 0);
  Serial.print("                 ");
  Serial.println(ntu);
  delay(2000);
}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}
