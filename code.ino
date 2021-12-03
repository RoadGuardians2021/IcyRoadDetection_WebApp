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


#include <TinyLoRa.h> //library for transmitting LoRa data.
#include <SPI.h> 

#include <Wire.h> //current sensor
#include <Adafruit_INA219.h> //current sensor

Adafruit_INA219 ina219; //current sensor

/**************************************************************************************************************************************************************************************************************************/

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x5d, 0x7f, 0x0f, 0xc5, 0x0d, 0x0e, 0x8a, 0x81, 0x8f, 0xea, 0x06, 0xc3, 0xe4, 0xc9, 0xf2, 0xe5 }; //key for encoding the data from the device to the network server.

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x24, 0x20, 0x9e, 0xe6, 0xcf, 0x81, 0xe1, 0x8d, 0x9c, 0x19, 0x38, 0x87, 0xe3, 0xcc, 0x3f, 0x6d }; //key for encoding the data from the device to the application server.  

// Device Address (MSB)
uint8_t DevAddr[4] = { 0x01, 0x4c, 0xb8, 0x28 }; //Used by the network server to identify the device.


/************************** Example Begins Here ***********************************/

// Pinout for Feather M0 LoRa
TinyLoRa lora = TinyLoRa(3, 8, 4);

int samples[NUMSAMPLES]; //array to hold the thermistor values to get a more accurate reading*/
float currentValues[15];
float tempValues[15];

void setup()
{
  delay(2000);
  Serial.begin(9600); //start the serial monitor with 9600 bps data rate
  analogReference(AR_EXTERNAL); //configure the reference voltage which in this case is the voltage applied to the AREF pin.

  // while (! Serial); //commented out so that the script runs automatically whenever the board is powered

  // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize LoRa
  Serial.print("Starting LoRa...");
  // define channel to send data on. MULTI means data will be sent to a random channel.
  lora.setChannel(CH1);
  // set datarate
  lora.setDatarate(SF9BW125);
  // set transmitter power
  lora.setPower(20);

  if (!lora.begin()) //if LoRa fails to initialize, print this to serial monitor and wait
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while (true);
  }
  Serial.println("OK");

  uint32_t currentFrequency;

  Serial.println("Hello!");
  
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip"); //If current sensor fails to initialize, print this to serial monitor and wait
    while (1) {
      delay(10);
    }
  }

  Serial.println("Measuring voltage and current with INA219 ...");

} //end of setup loop

void loop() /*************************************main loop*************************************/
{
  /****************************Getting samples of samples thermistor values************************************************************************************************************/

  uint8_t i;
  float temp = runThermistor();

  Serial.print("Average analog reading ");
  Serial.println(temp);

  // convert the value to resistance
  temp = 1023 / temp - 1;
  temp = SERIESRESISTOR / temp;
  Serial.print("Thermistor resistance ");
  Serial.println(temp);

  float steinhart;
  int int_steinhart;
  steinhart = temp / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to Celsius
  steinhart = (steinhart * 1.8) + 32;          // convert to temperature in Fahrenheit

  Serial.print("Temperature: ");
  Serial.print(steinhart);
  Serial.println(" F");
  String string1 = String(steinhart) + " F";
  Serial.println(string1);

  /*************************************************************************************************Current sensor data down below********************/

  float current_mA = 0; //INA219

  current_mA = runCurrent();

  Serial.println("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  String string2 = String(current_mA) + " mA";
  Serial.println(string2);

  String string3 = string1 + " " + string2; //concatenate the two strings with sensor data
  Serial.println(string3);
  Serial.println("Size of data string in bytes");
  Serial.println(sizeof(string3));

  unsigned char loraData[16];

  for (int i = 0; i < string3.length(); i++) //put the string data into an array for transmitting.
  {
    loraData[i] = string3.charAt(i);
  }

  Serial.println("Size of array in bytes");
  Serial.println(sizeof(loraData));

  /************transmitting data*****************************************************/

  /*****device only transmits the sensor data when temperature is below 40 degrees fahrenheit****/
  if(steinhart <= 40.0){
    Serial.println("Temperature is below 40 degrees Fahrenheit");
    Serial.println("Sending LoRa Data...");
    lora.sendData(loraData, sizeof(loraData), lora.frameCounter); //TinyLora library function to transmit data
    Serial.print("Frame Counter: "); Serial.println(lora.frameCounter);
    lora.frameCounter++; //increase frame counter.

    // blink LED to indicate packet sent
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);

    delay(300000); //wait for 300 seconds (5 minutes) before checking sensor values
  
  } /****end of if loop**************************************************************************************************************************************************************************************/
  
  else if(steinhart > 40.0){
    Serial.println("Temperature above 40 degrees Fahrenheit. The sensor data will not be transmitted");

    delay(300000); //wait for 300 seconds (5 minutes) before checking sensor values
    
  } /****end of else if loop*************************************************************************************************************************************************************************/
  
  else{
    Serial.println("Error as temperature should be below, equal to, or above 40 degrees Fahrenheit");
    
    delay(5000); //wait for 5 seconds before checking conditions again as hitting this loop indicates that the thermistor is malfunctioning and will hopefully collect a valid temperature value on the next pass.  
      } /*end of else loop*******************************************************************************************************************************************************************/
  
} //end of void loop()

float runCurrent()
{
  float currentAverage = 0;
  bool isOutliers = true;
  float iqr;
  float upper;
  float lower;
  float iterate = 0;

  // Run while there are outliers and less than 6 iterations
  while (isOutliers && iterate < 5)
  {
    currentAverage = 0;

    // Populate array with readings
    for (int i = 0; i < 15; i++)
    {
      currentValues[i] = ina219.getCurrent_mA();
      currentAverage += currentValues[i];

      delay(100);
    }

    // Get average of values
    currentAverage /= 15;

    // Sort array
    sort(currentValues, 15);

    // Find inter-quartile range
    iqr = currentValues[11] - currentValues[3];
    iqr *= 1.5;

    // Get upper and lower bounds
    upper = iqr + currentValues[11];
    lower = currentValues[3] - iqr;

    // Check for outliers
    for (int i = 0; i < 15; i++)
    {
      // If there is an outlier
      if (currentValues[i] > upper || currentValues[i] < lower)
      {
        // Set isOutliers to true and break the for loop
        isOutliers = true;
        break;
      }
      else
      {
        isOutliers = false;
      }
    }

    iterate++;
  }

  // Check if while loop timed out
  if (isOutliers)
  {
    // Return error value
    return -1000;
  }

  // Return average value
  return currentAverage;
}

float runThermistor()
{
  float tempAverage = 0;
  bool isOutliers = true;
  float iqr;
  float upper;
  float lower;
  float iterate = 0;

  // Run while there are outliers and less than 6 iterations
  while (isOutliers && iterate < 5)
  {
    tempAverage = 0;

    // Populate array with readings
    for (int i = 0; i < 15; i++)
    {
      tempValues[i] = analogRead(THERMISTORPIN);
      tempAverage += tempValues[i];

      delay(100);
    }

    // Get average of values
    tempAverage /= 15;

    // Sort array
    sort(tempValues, 15);

    // Find inter-quartile range
    iqr = tempValues[11] - tempValues[3];
    iqr *= 1.5;

    // Get upper and lower bounds
    upper = iqr + tempValues[11];
    lower = tempValues[3] - iqr;

    // Check for outliers
    for (int i = 0; i < 15; i++)
    {
      // If there is an outlier
      if (tempValues[i] > upper || tempValues[i] < lower)
      {
        // Set isOutliers to true and break the for loop
        isOutliers = true;
        break;
      }
      else
      {
        isOutliers = false;
      }
    }

    iterate++;
  }

  // Check if while loop timed out
  if (isOutliers)
  {
    // Return error value
    return -1000;
  }

  // Return average value
  return tempAverage;
}

void sort(float arr[], int size)
{
  for (int i = 0; i < (size - 1); i++)
  {
    for (int j = 0; j < (size - (i + 1)); j++)
    {
      if (arr[j] > arr[j + 1])
      {
        int k = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = k;
      }
    }
  }
}
