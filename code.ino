
// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000  
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000) (Steinhart-Hart Equation)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
// libraries
#include <TinyLoRa.h> //hello lora
#include <SPI.h> //hello lora
#include <Wire.h> //current sensor
#include <Adafruit_INA219.h> //current sensor

Adafruit_INA219 ina219; //current sensor

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x93, 0xca, 0x30, 0x22, 0x9d, 0x2f, 0xee, 0x39, 0xc6, 0x72, 0xee, 0xc9, 0x2a, 0x35, 0x32, 0xd0 }; //hello lora

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x43, 0x3b, 0x12, 0xa4, 0xce, 0x5d, 0xab, 0x49, 0x8f, 0x72, 0xbf, 0x23, 0xdc, 0x62, 0x39, 0xca }; //hello lora
// Device Address (MSB)
uint8_t DevAddr[4] = { 0x01, 0x12, 0xc9, 0x22 }; //hello lora

/************************** Example Begins Here ***********************************/
// Data Packet to Send to TTN
//unsigned char loraData[11] = { string1 }; //hello lora

// How many times data transfer should occur, in seconds
const unsigned int sendInterval = 30; //hello lora

// Pinout for Feather 32u4 LoRa
//TinyLoRa lora = TinyLoRa(7, 8, 4); //hello lora

// Pinout for Feather M0 LoRa
TinyLoRa lora = TinyLoRa(3, 8, 4); //hello lora

int samples[NUMSAMPLES]; //Thermistor

void setup()
{
  delay(2000); //hello lora example
  Serial.begin(9600); //Thermistor and Hello Lora Example
  analogReference(AR_EXTERNAL); //Configures the reference voltage used for analog input. In this case, the voltage applied to the AREF pin is used as a reference.

  while (! Serial); //While Serial interface is working
 
  // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize LoRa
  Serial.print("Starting LoRa...");
  // define channel to send data on
  lora.setChannel(CH3);
  // set datarate
  lora.setDatarate(SF7BW125); 
    if(!lora.begin()) //lora radio failure check
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }
  Serial.println("OK");

 /*******************************************************************************************Adafruit INA219*******************************/

  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  
  uint32_t currentFrequency;
    
  Serial.println("Hello!");
  
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");

} //end of void setup()


void loop() 
{
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  Serial.print("Average analog reading "); 
  Serial.println(average);
  
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
  
  float steinhart;
  int int_steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C
  
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");
  String string1 = String(steinhart) + " C";
  Serial.println(string1);
  unsigned char loraData1[8];
  for ( int i = 0; i < string1.length(); i++)
    {
      loraData1[i] = string1.charAt(i);
    }
    
  delay(5000);

  Serial.println("Sending LoRa Data...");
  lora.sendData(loraData1, sizeof(loraData1), lora.frameCounter);
  Serial.print("Frame Counter: ");Serial.println(lora.frameCounter);
  lora.frameCounter++;

  // blink LED to indicate packet sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("delaying...");
  delay(sendInterval * 1000); /*************************************************************************************************End of sending Thermistor data*/

  float current_mA = 0; //INA219

  current_mA = ina219.getCurrent_mA(); //INA219

  Serial.println("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  String string2 = String(current_mA) + " mA";
  Serial.println(string2);
  unsigned char loraData2[8];
  for ( int i = 0; i < string2.length(); i++)
    {
      loraData2[i] = string2.charAt(i);
    }
    
  delay(5000);

  Serial.println("Sending LoRa Data...");
  lora.sendData(loraData2, sizeof(loraData2), lora.frameCounter);
  Serial.print("Frame Counter: ");Serial.println(lora.frameCounter);
  lora.frameCounter++;

  // blink LED to indicate packet sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("delaying...");
  delay(sendInterval * 1000);

} //end of void loop()