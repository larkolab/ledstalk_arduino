#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>
#include <Wire.h>

const int PIR_CALIBRATE_TIME = 10;

unsigned long last_motion_time = 0;
boolean presence_status = false;
boolean last_presence_status = false;
boolean presence_init = true;

// Temperature Sensor
int temperaureSensorPin = A0;

// LED
int ledPin = 4;

// MAC address for your Ethernet shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x82, 0x09 };

// Your Xively key to let you upload data
//char xivelyKey[] = "Yg7EpfFOFnwolGvfiNwpWNNU2vNRcPKfhmOk1x4Pzl9e3SLv";
char xivelyKey[] = "Y8bnZpN22M05lpN3qlFd26JNJKzoXCc4Ppmf7NKExv9bQecw";

// Define the string for our datastream ID
char messageId[] = "MESSAGE_CHANNEL";
char presenceId[] = "PRESENCE_CHANNEL";
char temperatureId[] = "TEMPERATURE_CHANNEL";
String message;
int message_length;

XivelyDatastream datastreams[] = {
  XivelyDatastream(messageId, strlen(messageId), DATASTREAM_STRING),
  XivelyDatastream(presenceId, strlen(presenceId), DATASTREAM_INT),
  XivelyDatastream(temperatureId, strlen(temperatureId), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(144422134, datastreams, 3 /* number of datastreams */);

EthernetClient client;
XivelyClient xivelyclient(client);

void MotionDetected() {
    //digitalWrite(ledPin, HIGH);
    last_motion_time = millis();
    Serial.println("--> PIR: motion detected");
}

float readTemp(){
    // Read analog input for temperature
    // Get value between 0 and 1024 for voltage between 0v and 5v
    int tempValue = analogRead(temperaureSensorPin);  

    // Convert value to tension
    float temp_mV = tempValue * 5000.0 / 1024.0;  
    Serial.println(temp_mV);
 
    // Convert voltage (mV) to temperature in °C
    float temperature = (temp_mV - 500) / 10;
 
    return temperature;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  
  // LED config
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // PIR config
  digitalWrite(ledPin, HIGH);
  //give the sensor some time to calibrate (10-60s)
  Serial.print("calibrating sensor ");
  for(int i = 0; i < PIR_CALIBRATE_TIME; i++){
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  digitalWrite(ledPin, LOW);
  
  attachInterrupt(0, MotionDetected, HIGH);
  
  Wire.begin(); // join i2c bus (address optional for master)
  
  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(5000);
  }
  Serial.println(Ethernet.localIP());
}

void loop() {
  Serial.println("checking for message...");
  int ret = xivelyclient.get(feed, xivelyKey);
  Serial.print("xivelyclient.get returned ");
  Serial.println(ret);

  if (ret > 0)
  {
    // Set Temperature value to Xively
    float temperature = readTemp();
    datastreams[2].setFloat(temperature);
    Serial.print("temperature = "); Serial.println(temperature);
    xivelyclient.put(feed, xivelyKey);
    
    // Set Presence status to Xively
    if (millis() > last_motion_time + 10000)
    {
      Serial.println("--> ya personne !");
      datastreams[1].setInt(0);
      presence_status = true;
    } else {
      Serial.println("--> ya quelqu'un !");
      datastreams[1].setInt(1);
      presence_status = false;      
    }
    
    if ((presence_status != last_presence_status) || (presence_init == true)) {
      xivelyclient.put(feed, xivelyKey);
      last_presence_status = presence_status;
      presence_init = false;
    }
    
    // Get Message from Xively and send to LED display */
    message = feed[0].getString();
    Serial.print("message = "); Serial.println(message);
    
    message_length = message.length() + 1;
    char txt_buffer[message_length];
    message.toCharArray(txt_buffer, message_length);
    
    Wire.beginTransmission(4); // transmit to device #4
    Wire.write((uint8_t*)txt_buffer, message_length);          // sends string as array of bytes
    Wire.endTransmission();    // stop transmitting    
  }

  delay(1000UL);
}
