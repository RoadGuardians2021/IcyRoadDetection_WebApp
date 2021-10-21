/********************************************************************************Start here************************************************/
// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000 
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000 
  
 
#include <TinyLoRa.h> //hello lora header file
#include <SPI.h> //hello lora header file

#include <Wire.h> //current sensor
#include <Adafruit_INA219.h> //current sensor

Adafruit_INA219 ina219; //current sensor

/**************************************************************************************************************************************************************************************************************************/

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0xba, 0x83, 0x83, 0x85, 0x9f, 0xc3, 0x83, 0x2c, 0xcc, 0x0d, 0x61, 0x54, 0x2b, 0x7a, 0xa1, 0x92 }; //hello lora

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x15, 0xef, 0x9b, 0x61, 0x07, 0xad, 0xa4, 0xaa, 0xfa, 0x8c, 0xdb, 0xf3, 0xe9, 0xe4, 0x0c, 0x26 }; //hello lora

// Device Address (MSB)
uint8_t DevAddr[4] = { 0x01, 0x35, 0xc1, 0x4a }; //hello lora


/************************** Example Begins Here ***********************************/

// Pinout for Feather M0 LoRa
TinyLoRa lora = TinyLoRa(3, 8, 4); //hello lora

int samples[NUMSAMPLES]; //array to hold the thermistor values to get a more accurate reading*/

void setup()
{
  delay(2000); 
  Serial.begin(9600); 
  analogReference(AR_EXTERNAL); 

  while (! Serial);
 
  // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize LoRa
  Serial.print("Starting LoRa...");
  // define channel to send data on. MULTI means data will be sent to a random channel.
  lora.setChannel(CH1); 
  // set datarate
  lora.setDatarate(SF10BW125);
  // set tx power
  lora.setPower(20);
  
  if(!lora.begin())
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }
  Serial.println("OK");

 /*******************************************************************************************Adafruit INA219*******************************/

  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens. Note, this line needs to be removed when board is not plugged into a computer.
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


void loop() /*************************************main loop*************************************/
{
  /**************************************************************************************** Getting samples of samples thermistor values*/
  
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
  
  /*************************************************************************************************Current sensor data down below********************/

  float current_mA = 0; //INA219

  current_mA = ina219.getCurrent_mA(); //INA219

  Serial.println("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  String string2 = String(current_mA) + " mA";
  Serial.println(string2);  

  String string3 = string1 + " " + string2;
  Serial.println(string3);
  Serial.println("Size of data string in bytes");
  Serial.println(sizeof(string3));

  unsigned char loraData[16];

  for (int i = 0; i < string3.length(); i++)
  {
    loraData[i] = string3.charAt(i);
  }

  Serial.println("Size of array in bytes");
  Serial.println(sizeof(loraData));

  delay(5000);
  
  /************transmitting data*****************************************************/

  Serial.println("Sending LoRa Data...");
  lora.sendData(loraData, sizeof(loraData), lora.frameCounter);
  Serial.print("Frame Counter: ");Serial.println(lora.frameCounter);
  lora.frameCounter++;

  // blink LED to indicate packet sent
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  
  Serial.println("delaying...");
  delay(60000);

} //end of void loop()
