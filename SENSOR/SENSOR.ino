
#include <avr/pgmspace.h>
#include <EEPROM.h>

int S_Keruh = A0;
int S_Ph = A1;
int S_Gas = A2;
int S_Oxygen = A3;
float amonia, keruh, oxygen, ph;
float volt;
float tegang;
float wew;
int samples = 10;
float adc_resolution = 1024.0;
float ph_regresi_2;

float m = -0.263; //Slope
float b = 0.42; //Y-Intercept
float R0 = 2.19; //Sensor Resistance in fresh air from previous code



#define DoSensorPin  A3//dissolved oxygen sensor analog output pin to arduino mainboard
#define VREF 5000    //for arduino uno, the ADC reference is the AVCC, that is 5000mV(TYP)
float doValue;      //current dissolved oxygen value, unit; mg/L
float temperature = 25;    //default temperature is 25^C, you can use a temperature sensor to read it

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

#define ReceivedBufferLength 20
char receivedBuffer[ReceivedBufferLength + 1];  // store the serial command
byte receivedBufferIndex = 0;

#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    //store the analog value in the array, readed from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;

#define SaturationDoVoltageAddress 12          //the address of the Saturation Oxygen voltage stored in the EEPROM
#define SaturationDoTemperatureAddress 16      //the address of the Saturation Oxygen temperature stored in the EEPROM
float SaturationDoVoltage, SaturationDoTemperature;
float averageVoltage;

const float SaturationValueTab[41] PROGMEM = {      //saturation dissolved oxygen concentrations at various temperatures
  14.46, 14.22, 13.82, 13.44, 13.09,
  12.74, 12.42, 12.11, 11.81, 11.53,
  11.26, 11.01, 10.77, 10.53, 10.30,
  10.08, 9.86,  9.66,  9.46,  9.27,
  9.08,  8.90,  8.73,  8.57,  8.41,
  8.25,  8.11,  7.96,  7.82,  7.69,
  7.56,  7.43,  7.30,  7.18,  7.07,
  6.95,  6.84,  6.73,  6.63,  6.53,
  6.41,
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  readDoCharacteristicValues();
}


void loop() {
  // put your main code here, to run repeatedly:

  cek_keruh();
  cek_ph();
  cek_gas();
  cek_DO();
  kirim();

  delay(200);

}




boolean serialDataAvailable(void)
{
  char receivedChar;
  static unsigned long receivedTimeOut = millis();
  while ( Serial.available() > 0 )
  {
    if (millis() - receivedTimeOut > 500U)
    {
      receivedBufferIndex = 0;
      memset(receivedBuffer, 0, (ReceivedBufferLength + 1));
    }
    receivedTimeOut = millis();
    receivedChar = Serial.read();
    if (receivedChar == '\n' || receivedBufferIndex == ReceivedBufferLength)
    {
      receivedBufferIndex = 0;
      strupr(receivedBuffer);
      return true;
    } else {
      receivedBuffer[receivedBufferIndex] = receivedChar;
      receivedBufferIndex++;
    }
  }
  return false;
}

byte uartParse()
{
  byte modeIndex = 0;
  if (strstr(receivedBuffer, "CALIBRATION") != NULL)
    modeIndex = 1;
  else if (strstr(receivedBuffer, "EXIT") != NULL)
    modeIndex = 3;
  else if (strstr(receivedBuffer, "SATCAL") != NULL)
    modeIndex = 2;
  return modeIndex;
}

void doCalibration(byte mode)
{
  char *receivedBufferPtr;
  static boolean doCalibrationFinishFlag = 0, enterCalibrationFlag = 0;
  float voltageValueStore;
  switch (mode)
  {
    case 0:
      if (enterCalibrationFlag)
        Serial.println(F("Command Error"));
      break;

    case 1:
      enterCalibrationFlag = 1;
      doCalibrationFinishFlag = 0;
      Serial.println();
      Serial.println(F(">>>Enter Calibration Mode<<<"));
      Serial.println(F(">>>Please put the probe into the saturation oxygen water! <<<"));
      Serial.println();
      break;

    case 2:
      if (enterCalibrationFlag)
      {
        Serial.println();
        Serial.println(F(">>>Saturation Calibration Finish!<<<"));
        Serial.println();
        EEPROM_write(SaturationDoVoltageAddress, averageVoltage);
        EEPROM_write(SaturationDoTemperatureAddress, temperature);
        SaturationDoVoltage = averageVoltage;
        SaturationDoTemperature = temperature;
        doCalibrationFinishFlag = 1;
      }
      break;

    case 3:
      if (enterCalibrationFlag)
      {
        Serial.println();
        if (doCalibrationFinishFlag)
          Serial.print(F(">>>Calibration Successful"));
        else
          Serial.print(F(">>>Calibration Failed"));
        Serial.println(F(",Exit Calibration Mode<<<"));
        Serial.println();
        doCalibrationFinishFlag = 0;
        enterCalibrationFlag = 0;
      }
      break;
  }
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
  {
    bTab[i] = bArray[i];
  }
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

void readDoCharacteristicValues(void)
{
  EEPROM_read(SaturationDoVoltageAddress, SaturationDoVoltage);
  EEPROM_read(SaturationDoTemperatureAddress, SaturationDoTemperature);
  if (EEPROM.read(SaturationDoVoltageAddress) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 1) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 2) == 0xFF && EEPROM.read(SaturationDoVoltageAddress + 3) == 0xFF)
  {
    SaturationDoVoltage = 1127.6;   //default voltage:1127.6mv
    EEPROM_write(SaturationDoVoltageAddress, SaturationDoVoltage);
  }
  if (EEPROM.read(SaturationDoTemperatureAddress) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 1) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 2) == 0xFF && EEPROM.read(SaturationDoTemperatureAddress + 3) == 0xFF)
  {
    SaturationDoTemperature = 25.0;   //default temperature is 25^C
    EEPROM_write(SaturationDoTemperatureAddress, SaturationDoTemperature);
  }
}

void cek_DO() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 30U)  //every 30 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(A3);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }

  static unsigned long tempSampleTimepoint = millis();
  if (millis() - tempSampleTimepoint > 500U) // every 500 milliseconds, read the temperature
  {
    tempSampleTimepoint = millis();
    //temperature = readTemperature();  // add your temperature codes here to read the temperature, unit:^C
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 1000U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the value more stable by the median filtering algorithm
    Serial.print(F("Temperature:"));
    Serial.print(temperature, 1);
    Serial.print(F("^C"));
    doValue = pgm_read_float_near( &SaturationValueTab[0] + (int)(SaturationDoTemperature + 0.5) ) * averageVoltage / SaturationDoVoltage; //calculate the do value, doValue = Voltage / SaturationDoVoltage * SaturationDoValue(with temperature compensation)
    Serial.print(F(",  DO Value:"));
    Serial.print(doValue, 2);
    Serial.println(F("mg/L"));
    oxygen = doValue;
  }
  if (serialDataAvailable() > 0)
  {
    byte modeIndex = uartParse();  //parse the uart command received
    doCalibration(modeIndex);    // If the correct calibration command is received, the calibration function should be called.
  }


}




void cek_gas() {
  float sensor_volt; //Define variable for sensor voltage
  float RS_gas; //Define variable for sensor resistance
  float ratio; //Define variable for ratio
  sensor_volt = S_Gas * (5.0 / 1023.0); //Convert analog values to voltage
  RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0; //Get value of RS in a gas
  ratio = RS_gas / R0; // Get ratio RS_gas/RS_air
  double ppm_log = (log10(ratio) - b) / m; //Get ppm value in linear scale according to the the ratio value
  double ppm = pow(10, ppm_log); //Convert ppm value to log scale
  double percentage = ppm / 10000; //Convert to percentage
  int PotSP = analogRead(A1);
  int Zero = analogRead(A2);
  int Spam = analogRead(A3);
  double ppmsp;
  double zerotrim;
  zerotrim = map(Zero, 0, 1023, 0, 2);
  ppmsp = map(PotSP, 0, 1023, 0, 5000);
  Serial.print("Amonia = ");
  Serial.println(ppmsp);
  amonia = ppmsp;
}



//https://cimpleo.com/blog/simple-arduino-ph-meter/
void cek_ph() {
  int measurings = 0;
  for (int i = 0; i < samples; i++)
  {
    measurings += analogRead(S_Ph);
    delay(10);
  }
  float voltage = 5 / adc_resolution * measurings / samples;
  ph = ph_pars(voltage);
  Serial.print("pH= ");
  float ph_regresi = 1.1067193676 * ph;
  ph_regresi_2 = ph_regresi - 0.11680238;
  Serial.println(ph_regresi_2);
}
float ph_pars (float voltage) {
  return 7 + ((2.5 - voltage) / 0.18) ;
}


void cek_keruh() {
  tegang = analogRead(S_Keruh) ;
  wew = tegang * (5.0 / 1024);
  keruh = 100 - (wew / 3.57) * 100.00;
  Serial.print("Keruh= ");
  Serial.println(keruh);
}


void kirim() {
  Serial1.print("*");
  Serial1.print(amonia);
  Serial1.print(",");
  Serial1.print(keruh);
  Serial1.print(",");
  Serial1.print(oxygen);
  Serial1.print(",");
  Serial1.print(ph_regresi_2);
  Serial1.println("#");
}
