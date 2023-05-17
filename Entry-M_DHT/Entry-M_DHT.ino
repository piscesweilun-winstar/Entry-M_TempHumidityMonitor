#include "DHT.h"  // DHT sensor library by Adafruit

#define BAUDRATE  19200

// Response
#define ACK       0x06
#define NAK       0x15

// DHT11 is an old and slow sensor, we'd better not to hurry
#define MIN_READ_PERIOD  2000

#define DHTPIN  2         // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

/*
*   Wait for response.
*   Ret: 0x6  OK, It means Entry-M / M-Series already received the command. (But not sure completed or not)
*        0x15 Error 
*        0x00 Timeout
*/
unsigned char WaitResponse(unsigned long minWaitTime, unsigned long maxWaitTime) {
  unsigned char ret = 0;
  unsigned long timer = 0;
  
  if(minWaitTime > 0)
    delay(minWaitTime);

  while(ret == 0 && ((timer < maxWaitTime) || (maxWaitTime == 0))) {
    if(Serial.available()) {
      ret = Serial.read();
      if(ret == ACK || ret == NAK)
        break;
    }
    else {
      delay(1);
      timer++;
    }
  }
  return ret;
}

/*
*   Clear read buffer
*/
void ClearResponse() {
  while(Serial.available())
    Serial.read();
}

/*
*   Write command to UART (to Device)
*/
void WriteCommand(const unsigned char * data, int count) {
  for (int i = 0; i < count; i++) {
    Serial.write(data[i]);
  }
}

/*
*   cmd[sizeof(cmd)-2] is the check sum value of cmd[0] to Command[sizeof(cmd)-3]
*   Example: 
*     cmd[sizeof(cmd)-2] = CheckSum(cmd, sizeof(cmd)-2);
*/
byte CheckSum(const char* data, int count) {
  byte BCC = 0x00;
  for (int i = 0; i < count; i++) {
    BCC += data[i];
  }
  return BCC;
}

void ShowTemperature(float deg) {
  ClearTemperature();   // Clear virtual layer

  static unsigned char XXX[33] = {0x01,0x21,0x02,0x04,0x31,0x13,0x00,0x00,0x8C,0x00,0x50,0xFF,0xFF,0xFF,0x00,0x00,0x00,'T','e','m','p',':',' ','0','0','.','0',' ','c',' ',0x0A,0xA0,0x0D};
  XXX[26] = (int)(deg * 10) % 10 + '0';
  XXX[24] = (int)deg % 10 + '0';
  XXX[23] = ((int)deg / 10) % 10 + '0';
  if(XXX[23] == '0') {
    XXX[23] = ' ';
  }
  XXX[31] = CheckSum(XXX, sizeof(XXX) - 2);
  WriteCommand(XXX, sizeof(XXX));
  WaitResponse(100, 500);
}

void ClearTemperature() {
  static unsigned char XXX[16] = {0x01,0x10,0x02,0x04,0x41,0x00,0x8C,0x00,0x50,0x01,0x72,0x00,0x78,0x0A,0x29,0x0D};
  WriteCommand(XXX, sizeof(XXX));
  WaitResponse(100, 500);
}

void ShowHumidity(float hum) {
  ClearHumidity();
  static unsigned char XXX[30] = {0x01,0x1E,0x02,0x04,0x31,0x13,0x00,0x00,0x9F,0x00,0x8C,0xFF,0xFF,0xFF,0x00,0x00,0x00,'H','u','m',':',' ','0','0',' ','%',' ',0x0A,0xE4,0x0D};
  XXX[23] = (int)hum % 10 + '0';
  XXX[22] = ((int)hum / 10) % 10 + '0';
  if(XXX[22] == '0') {
    XXX[22] = ' ';
  }
  XXX[28] = CheckSum(XXX, sizeof(XXX) - 2);
  WriteCommand(XXX, sizeof(XXX));
  WaitResponse(100, 500);
}

void ClearHumidity() {
  static unsigned char XXX[16] = {0x01,0x10,0x02,0x04,0x41,0x00,0x9F,0x00,0x8C,0x01,0x5E,0x00,0xB4,0x0A,0xA0,0x0D};
  WriteCommand(XXX, sizeof(XXX));
  WaitResponse(100, 500);
}

void setup() {
  Serial.begin(19200);

  while (!Serial) ;  // wait for serial port to connect. Needed for native USB port only

  delay(2000);
  ClearResponse();

  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(MIN_READ_PERIOD);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) /*|| isnan(f)*/) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  //float hic = dht.computeHeatIndex(t, h, false);

  /*
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  */

  ShowTemperature(t);
  ShowHumidity(h);

  // Clear read buffer
  while(Serial.available())
    Serial.read();
}
