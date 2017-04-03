const char *weather[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };
enum FORECAST
{
  STABLE = 0,     // "Stable Weather Pattern"
  SUNNY = 1,      // "Slowly rising Good Weather", "Clear/Sunny "
  CLOUDY = 2,     // "Slowly falling L-Pressure ", "Cloudy/Rain "
  UNSTABLE = 3,   // "Quickly rising H-Press",     "Not Stable"
  THUNDERSTORM = 4, // "Quickly falling L-Press",    "Thunderstorm"
  UNKNOWN = 5     // "Unknown (More Time needed)
};


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  DBG_OUTPUT_PORT.println("DEBUG:handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}


String getControllerTime() {
  DBG_OUTPUT_PORT.println("DEBUG:getControllerTime");
  requestTime();
  return "OK";
}


String updateAllSensors(){
  l_powerState = "";
  l_waterTemp = "";
  l_setpoint = "";
  l_outsideTemp = "";
  l_pumpState = "";
  l_auxState = "";
  l_airState = "";
  l_lightState = "";
  l_currentTime = "";
  l_htStatus = "";
  l_humidity = "";
  l_pressure = "";
  l_forecast = -1;
  return "OK";
}

void sendHotTubCommand( String label, String value ) {
  Serial.print("COM:");
  Serial.print(label);
  Serial.print(":");
  Serial.println(value);
}

void receiveTime(unsigned long epoch){
  // print the hour, minute and second:
  String currentTime = String((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
  currentTime += ':';
  if ( ((epoch % 3600) / 60) < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    currentTime += '0';
  }
  currentTime += String((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
  currentTime += ':';
  if ( (epoch % 60) < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    currentTime += '0';
  }
  currentTime += String(epoch % 60); // print the second
  setTime(currentTime,true);
  currentTime = String();
  updateAllSensors();
  digitalWrite(CONTROLLER_CONNECTED_PIN, HIGH);
}  

void restartESP() {
  ESP.restart();
}

void advertise() { 
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("HotTub");
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("index.html");
  SSDP.setModelName("TimeMachine");
  SSDP.setModelNumber("929000996503");
  SSDP.setModelURL("http://phillips.dnsalias.com");
  SSDP.setManufacturer("Cat-8 Electronics");
  SSDP.setManufacturerURL("http://www.cat-8.com");
  SSDP.setDeviceType("upnp:rootdevice");
  //SSDP.setDeviceType("urn:schemas-upnp-org:device:HotTubController:1");
  if(SSDP.begin()){
    DBG_OUTPUT_PORT.println("DEBUG:SSDP started");
  }
}
 
void presentation()
{
  DBG_OUTPUT_PORT.println("DEBUG:Presenting sensors...");
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hot Tub Time Machine", "1.1");
  wait(LONG_WAIT);
  // Register all sensors to gateway (they will be created as child devices)
  present(CHILD_ID_HVAC, S_HEATER); // HVAC unit
  wait(SHORT_WAIT);
  present(CHILD_ID_OUTSIDETEMP, S_TEMP);
  wait(SHORT_WAIT);
  present(CHILD_ID_HUMIDITY, S_HUM);
  wait(SHORT_WAIT);
  present(CHILD_ID_LIGHT, S_BINARY);
  wait(SHORT_WAIT);
  present(CHILD_ID_AUX, S_BINARY);
  wait(SHORT_WAIT);
  present(CHILD_ID_AIR, S_BINARY);
  wait(SHORT_WAIT);
  present(CHILD_ID_TRIGGER, S_BINARY);
  wait(SHORT_WAIT);
  present(CHILD_ID_PRESSURE, S_BARO);
  wait(SHORT_WAIT);
  metric = getControllerConfig().isMetric;
}

void receive(const MyMessage &message) {
  DBG_OUTPUT_PORT.print("DEBUG:Message type: ");
  DBG_OUTPUT_PORT.print( message.type );
  DBG_OUTPUT_PORT.print(", sensor: ");
  DBG_OUTPUT_PORT.print( message.sensor );
  DBG_OUTPUT_PORT.print(", value: ");
  DBG_OUTPUT_PORT.println(message.data);
  if (message.sensor == CHILD_ID_HVAC && (message.type == V_HVAC_FLOW_STATE || message.type == V_HVAC_SETPOINT_HEAT)) {
    String heatCmd = message.data;
    if (message.type == V_HVAC_FLOW_STATE) {
      sendHotTubCommand("power", heatCmd.startsWith("HeatOn")?RELAY_ON:RELAY_OFF);
      l_powerState = "";
    }
    if (message.type == V_HVAC_SETPOINT_HEAT) setpointTemp(message.data,true);
  }
  else if (message.sensor == CHILD_ID_LIGHT && message.type == V_STATUS) {  
    if (message.type == V_STATUS) setLightState(message.getBool()?RELAY_ON:RELAY_OFF,true);
  }
  else if (message.sensor == CHILD_ID_AUX && message.type == V_STATUS) {  
    if (message.type == V_STATUS) setAuxState(message.getBool()?RELAY_ON:RELAY_OFF,true);
  }
  else if (message.sensor == CHILD_ID_AIR && message.type == V_STATUS) {  
    if (message.type == V_STATUS) setAirState(message.getBool()?RELAY_ON:RELAY_OFF,true);
  }
  else if (message.sensor == CHILD_ID_TRIGGER && message.type == V_STATUS) {  
    if (message.type == V_STATUS) l_htStatus = "";
  }
  else {
    DBG_OUTPUT_PORT.println("DEBUG:incoming message unhandled.");
  }
}

void gettemperature() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = bme.readHumidity();
    // Read temperature as Celsius (the default)
    //float t = dht.readTemperature();
    // Read temperature as Celsius
    float c = bme.readTemperature();
  
    float p = bme.readPressure();
    
    char arr[12];
    humidity = dtostrf(h, 6, 2, arr);
    outsideTemp = dtostrf(c*9/5+32, 6, 2, arr);
    //pressure = dtostrf(p/3386.39, 6, 2, arr);  //inHg
    pressure = dtostrf(p/100.0F, 6, 2, arr);
    float pressure = p/pow((1.0 - ( ALTITUDE / 44330.0 )), 5.255); // Adjust to sea level pressure using user altitude
    forecast = sample(pressure);
  
    DBG_OUTPUT_PORT.print("outsidetemp:");
    DBG_OUTPUT_PORT.println(c*9/5+32);
    DBG_OUTPUT_PORT.print("humidity:");
    DBG_OUTPUT_PORT.println(h);
    DBG_OUTPUT_PORT.print("pressure:");
    DBG_OUTPUT_PORT.println(p/3386.39);
    DBG_OUTPUT_PORT.print("forecast:");
    DBG_OUTPUT_PORT.println(forecast);
  }
}

float getLastPressureSamplesAverage()
{
  float lastPressureSamplesAverage = 0;
  for (int i = 0; i < LAST_SAMPLES_COUNT; i++)
  {
    lastPressureSamplesAverage += lastPressureSamples[i];
  }
  lastPressureSamplesAverage /= LAST_SAMPLES_COUNT;

  return lastPressureSamplesAverage;
}

// Algorithm found here
// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
// Pressure in hPa -->  forecast done by calculating kPa/h
int sample(float pressure)
{
  // Calculate the average of the last n minutes.
  int index = minuteCount % LAST_SAMPLES_COUNT;
  lastPressureSamples[index] = pressure;

  minuteCount++;
  if (minuteCount > 185)
  {
    minuteCount = 6;
  }

  if (minuteCount == 5)
  {
    pressureAvg = getLastPressureSamplesAverage();
  }
  else if (minuteCount == 35)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change * 2; // note this is for t = 0.5hour
    }
    else
    {
      dP_dt = change / 1.5; // divide by 1.5 as this is the difference in time from 0 value.
    }
  }
  else if (minuteCount == 65)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) //first time initial 3 hour
    {
      dP_dt = change; //note this is for t = 1 hour
    }
    else
    {
      dP_dt = change / 2; //divide by 2 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 95)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 1.5; // note this is for t = 1.5 hour
    }
    else
    {
      dP_dt = change / 2.5; // divide by 2.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 125)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    pressureAvg2 = lastPressureAvg; // store for later use.
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2; // note this is for t = 2 hour
    }
    else
    {
      dP_dt = change / 3; // divide by 3 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 155)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2.5; // note this is for t = 2.5 hour
    }
    else
    {
      dP_dt = change / 3.5; // divide by 3.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 185)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 3; // note this is for t = 3 hour
    }
    else
    {
      dP_dt = change / 4; // divide by 4 as this is the difference in time from 0 value
    }
    pressureAvg = pressureAvg2; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
    firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
  }

  int forecast = UNKNOWN;
  if (minuteCount < 35 && firstRound) //if time is less than 35 min on the first 3 hour interval.
  {
    forecast = UNKNOWN;
  }
  else if (dP_dt < (-0.25))
  {
    forecast = THUNDERSTORM;
  }
  else if (dP_dt > 0.25)
  {
    forecast = UNSTABLE;
  }
  else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05)))
  {
    forecast = CLOUDY;
  }
  else if ((dP_dt > 0.05) && (dP_dt < 0.25))
  {
    forecast = SUNNY;
  }
  else if ((dP_dt >(-0.05)) && (dP_dt < 0.05))
  {
    forecast = STABLE;
  }
  else
  {
    forecast = UNKNOWN;
  }

  // uncomment when debugging
  //Serial.print(F("Forecast at minute "));
  //Serial.print(minuteCount);
  //Serial.print(F(" dP/dt = "));
  //Serial.print(dP_dt);
  //Serial.print(F("kPa/h --> "));
  //Serial.println(weather[forecast]);

  return forecast;
}

