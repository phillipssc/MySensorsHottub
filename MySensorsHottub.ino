/* 
  
*/
//#define MY_DEBUG
#define DBG_OUTPUT_PORT Serial

#define CHILD_ID_OUTSIDETEMP 2
#define CHILD_ID_HUMIDITY 3
#define CHILD_ID_LIGHT 4
#define CHILD_ID_AUX 6
#define CHILD_ID_AIR 7
#define CHILD_ID_TRIGGER 9
#define CHILD_ID_HVAC 12
#define CHILD_ID_PRESSURE 14

#define LONG_WAIT 500
#define SHORT_WAIT 50

//Error codes
#define OK 0
#define CoLd 1  // temp cold <45
#define OH 2    // temp hot 108-112
#define HLEr 3  // temp way hot >112
#define SEoP 4  // sensor open
#define SESH 5  // sensor shorted
#define PSoC 6  // pressure switch circulating
#define PSoL 7  // pressure switch low
#define PSoH 8  // pressure switch high
#define ToE 9   // micro error heartbeat

//FP1 FP2 FP3 FP4 [n/a 12:00 n/a] filtration period
//FP<n>d [0 0 240] filtration duration
//SIL [n/a 12:00 n/a] silence start time
//SILd [0 0 12] silence duration
//CLDN [30 60 180] cooldown period
//ECL [60 180 240] economy cycle length
//CHCL [0 60 180] clear channel - runs aux pump and air pump 
//UTO [10 20 60] user time out - spa goes to sleep
//PUF [30 120 180] post use filtration
//CALB [194 204 218] calibration factor

#define MY_GATEWAY_ESP8266
#define MY_ESP8266_SSID "**************"
#define MY_ESP8266_PASSWORD "*******************"
// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 2
// The port to keep open on node server mode 
#define MY_PORT 5003      
// Enable UDP communication
//#define MY_USE_UDP
// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
#define MY_ESP8266_HOSTNAME "hottub"
// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  D6
#define MY_DEFAULT_LED_BLINK_PERIOD 100
#define MY_DEFAULT_ERR_LED_PIN D0  // Error led 
#define MY_DEFAULT_RX_LED_PIN  D1  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  D2  // the PCB, on board LED

#define NETWORK_CONNECTED_PIN 22
#define CONTROLLER_CONNECTED_PIN 21
#define MALFUNCTION_INDICATOR D0

#define RELAY_ON "on"
#define RELAY_OFF "off"

#define BME_SCK D4
#define BME_CS D3
#define BME_I2C_PORT 0x76
#define SEALEVELPRESSURE_HPA (1013.25)

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <MySensors.h>

#if defined(MY_USE_UDP)
#include <WiFiUdp.h>
#endif

const char* host = MY_ESP8266_HOSTNAME;

String powerState = "off";
String waterTemp = "35";
String setpoint = "102";
String outsideTemp = "20";
String pumpState = "low";
String auxState = "off";
String airState = "off";
String lightState = "off";
String currentTime = "00:00:00";
String htStatus = "OK";
String humidity = "40";
String pressure = "32";
String wl_status = "WL_DISCONNECTED";
int    forecast = -1;

String l_powerState = "";
String l_waterTemp = "";
String l_setpoint = "";
String l_outsideTemp = "";
String l_pumpState = "";
String l_auxState = "";
String l_airState = "";
String l_lightState = "";
String l_currentTime = "";
String l_htStatus = "";
String l_humidity = "";
String l_pressure = "";
String l_wl_status = "WL_DISCONNECTED";
int    l_forecast = -1;

const int LAST_SAMPLES_COUNT = 5;
float lastPressureSamples[LAST_SAMPLES_COUNT];
#define CONVERSION_FACTOR (1.0/10.0)
const float ALTITUDE = 23; // <-- adapt this value to your location's altitude (in m). Use your smartphone GPS to get an accurate value!
int minuteCount = 0;
bool firstRound = true;
// average value is used in forecast algorithm.
float pressureAvg;
// average after 2 hours is used as reference value for the next iteration.
float pressureAvg2;

float dP_dt;
boolean metric;

String command="";

unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 60*1000;              // interval at which to read sensor

ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;

MyMessage msgPower(CHILD_ID_HVAC, V_HVAC_FLOW_STATE);
MyMessage msgSetPoint(CHILD_ID_HVAC, V_HVAC_SETPOINT_HEAT);
MyMessage msgTemp(CHILD_ID_HVAC, V_TEMP);
MyMessage msgStatus(CHILD_ID_HVAC, V_VAR1);
MyMessage msgOutsideTemp(CHILD_ID_OUTSIDETEMP, V_TEMP);
MyMessage msgHumidity(CHILD_ID_HUMIDITY, V_HUM);
MyMessage msgPressure(CHILD_ID_PRESSURE, V_PRESSURE);
MyMessage msgForecast(CHILD_ID_PRESSURE, V_FORECAST);
MyMessage msgLight(CHILD_ID_LIGHT, V_STATUS);
MyMessage msgAir(CHILD_ID_AIR, V_STATUS);
MyMessage msgAux(CHILD_ID_AUX, V_STATUS);
MyMessage msgErrorTrigger(CHILD_ID_TRIGGER, V_STATUS);

Adafruit_BME280 bme; // I2C

void setup(void){
  //DBG_OUTPUT_PORT.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  Serial.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  //Serial.swap();
  delay(LONG_WAIT);
  DBG_OUTPUT_PORT.print("\n");

  pinMode(NETWORK_CONNECTED_PIN, OUTPUT);
  pinMode(CONTROLLER_CONNECTED_PIN, OUTPUT);
  pinMode(MALFUNCTION_INDICATOR, OUTPUT);
  digitalWrite(NETWORK_CONNECTED_PIN, LOW);
  digitalWrite(CONTROLLER_CONNECTED_PIN, LOW);
  digitalWrite(MALFUNCTION_INDICATOR, LOW);


  DBG_OUTPUT_PORT.println("DEBUG: Local files:");
  delay(LONG_WAIT);
  
  if (SPIFFS.begin())
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      //DBG_OUTPUT_PORT.printf("DEBUG:\tFile: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      DBG_OUTPUT_PORT.print("DEBUG:\tFS File: ");
      DBG_OUTPUT_PORT.print(fileName.c_str());
      DBG_OUTPUT_PORT.print(", size: ");
      DBG_OUTPUT_PORT.println(formatBytes(fileSize).c_str());
      delay(LONG_WAIT);     
    }
  }

  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("DEBUG:Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/ for web access");
  delay(LONG_WAIT);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/description.xml", HTTP_GET, [](){
      DBG_OUTPUT_PORT.println("DEBUG:/description.xml requested");  
      SSDP.schema(server.client());
  });

  server.on("/power", HTTP_PUT, httpSetPowerState);  
  server.on("/power", HTTP_GET, httpGetPowerState);
  server.on("/temp", HTTP_PUT, httpSetWaterTemp);
  server.on("/temp", HTTP_GET, httpGetWaterTemp);
  server.on("/setpoint", HTTP_PUT, httpSetpointTemp);
  server.on("/setpoint", HTTP_GET, httpGetpointTemp);
  server.on("/pump", HTTP_PUT, httpSetPumpState);
  server.on("/pump", HTTP_GET, httpGetPumpState);
  server.on("/aux", HTTP_PUT, httpSetAuxState);
  server.on("/aux", HTTP_GET, httpGetAuxState);
  server.on("/air", HTTP_PUT, httpSetAirState);
  server.on("/air", HTTP_GET, httpGetAirState);
  server.on("/light", HTTP_PUT, httpSetLightState);
  server.on("/light", HTTP_GET, httpGetLightState);
  server.on("/time", HTTP_PUT, httpSetTime);
  server.on("/time", HTTP_GET, httpGetTime);
  server.on("/status", HTTP_PUT, httpSetStatus);
  server.on("/status", HTTP_GET, httpGetStatus);
  server.on("/ntp", HTTP_PUT, httpGetNtpTime);
  server.on("/outsidetemp", HTTP_GET, httpGetOutsideTemp);
  server.on("/humidity", HTTP_GET, httpGetHumidity);
  server.on("/pressure", HTTP_GET, httpGetPressure);
  server.on("/forcast", HTTP_GET, httpGetForecast);
  server.on("/present", HTTP_PUT, httpSendPresentation);
  server.on("/advertise", HTTP_PUT, httpSendAdvertisement);
  server.on("/update", HTTP_PUT, httpSendUpdateAll);
  server.on("/program", HTTP_PUT, httpProgramMode);
  server.on("/restart", HTTP_PUT, restartESP);
  server.begin();
  DBG_OUTPUT_PORT.println("DEBUG:HTTP started");  
  digitalWrite(NETWORK_CONNECTED_PIN, HIGH);
  
  advertise();

  Wire.begin(BME_CS,BME_SCK);
  Wire.setClock(100000);

  if (!bme.begin(BME_I2C_PORT)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      //while (1);
  }

}

void loop(void){
  server.handleClient();
  gettemperature();
  if(Serial.available()) // check if the comm unit is sending a message 
  {    
    char inChar = (char)Serial.read();
    if ( (inChar == '\n' || inChar == '\r') && command.length() != 0 ) {
      if ((char)Serial.peek() == '\n' || (char)Serial.peek() == '\r') inChar = (char)Serial.read();
      command.trim();
      if (command.startsWith("power:")) setPowerState(command.substring(6),false);
      if (command.startsWith("temp:")) setWaterTemp(command.substring(5),false);
      if (command.startsWith("outsidetemp:")) setOutsideTemp(command.substring(12),false);
      if (command.startsWith("humidity:")) setHumidity(command.substring(9),false);
      if (command.startsWith("pressure:")) setPressure(command.substring(9),false);
      if (command.startsWith("forecast:")) setForecast(command.substring(9).toInt(),false);
      if (command.startsWith("pump:")) setPumpState(command.substring(5),false);
      if (command.startsWith("aux:")) setAuxState(command.substring(4),false);
      if (command.startsWith("air:")) setAirState(command.substring(4),false);
      if (command.startsWith("setpoint:")) setpointTemp(command.substring(9),false);
      if (command.startsWith("light:")) setLightState(command.substring(6),false);
      if (command.startsWith("time:")) setTime(command.substring(5),false);
      if (command.startsWith("status:")) setStatus(command.substring(7),false);
      if (command.startsWith("present:")) presentation();
      if (command.startsWith("advertise:")) advertise();
      if (command.startsWith("ntp:")) getControllerTime();
      if (command.startsWith("restart:")) restartESP();
      //DBG_OUTPUT_PORT.print("ECHO:");
      //DBG_OUTPUT_PORT.println(command); // send the read character to the debug port
      command = String();
    }
    else {
      command += inChar;
    }
  }
  if ( l_powerState != powerState) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller powerState");
    send(msgPower.set(powerState == "on"?"HeatOn":"Off"));   
    l_powerState = powerState;
    delay(LONG_WAIT);
  }
  if ( l_waterTemp != waterTemp) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller waterTemp");
    char arr[12];
    waterTemp.toCharArray(arr, sizeof(arr));
    send(msgTemp.set((int32_t)atol(arr)));
    l_waterTemp = waterTemp;
    delay(LONG_WAIT);
  }
  if ( l_outsideTemp != outsideTemp) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller outsideTemp");
    char arr[12];
    outsideTemp.toCharArray(arr, sizeof(arr));
    send(msgOutsideTemp.set((int32_t)atol(arr)));
    l_outsideTemp = outsideTemp;
    delay(LONG_WAIT);
  }
  if ( l_humidity != humidity) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller humidity");
    char arr[12];
    humidity.toCharArray(arr, sizeof(arr));
    send(msgHumidity.set((int32_t)atol(arr)));
    l_humidity = humidity;
    delay(LONG_WAIT);
  }
  if ( l_pressure != pressure) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller pressure");
    char arr[12];
    pressure.toCharArray(arr, sizeof(arr));
    send(msgPressure.set((int32_t)atol(arr)));
    l_pressure = pressure;
    delay(LONG_WAIT);
  }
  if ( l_forecast != forecast) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller forecast");
    //char arr[12];
    //forecast.toCharArray(arr, sizeof(arr));
    send(msgForecast.set(forecast));
    l_forecast = forecast;
    delay(LONG_WAIT);
  }
  if ( l_lightState != lightState) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller lightState");
    send(msgLight.set(lightState == "on"));
    l_lightState = lightState;
    delay(LONG_WAIT);
  }
  if ( l_htStatus != htStatus) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller status");
    char arr[htStatus.length()+1];
    htStatus.toCharArray(arr, sizeof(arr));
    send(msgStatus.set(arr));
    delay(LONG_WAIT);
    DBG_OUTPUT_PORT.println("DEBUG:Update controller trigger");
    send(msgErrorTrigger.set(htStatus != "OK"));
    digitalWrite(MALFUNCTION_INDICATOR, htStatus != "OK"?HIGH:LOW);
    l_htStatus = htStatus;
    delay(LONG_WAIT);
  }
  if ( l_airState != airState) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller airState");
    send(msgAir.set(airState == "on"));
    l_airState = airState;
    delay(LONG_WAIT);
  }
  if ( l_auxState != auxState) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller auxState");
    send(msgAux.set(auxState == "on"));
    l_auxState = auxState;
    delay(LONG_WAIT);
  }
  if ( l_setpoint != setpoint) {
    DBG_OUTPUT_PORT.println("DEBUG:Update controller setpoint");
    char arr[12];
    setpoint.toCharArray(arr, sizeof(arr));
    send(msgSetPoint.set((int32_t)atol(arr)));
    l_setpoint = setpoint;
    delay(LONG_WAIT);
  }
  wl_status = WiFi.status();
  if ( l_wl_status != wl_status) {
    digitalWrite(NETWORK_CONNECTED_PIN, wl_status=="3"?HIGH:LOW);
    l_wl_status = wl_status;
  }
  if ( currentTime == "00:00:00" ) requestTime();
}

