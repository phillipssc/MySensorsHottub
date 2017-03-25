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
  sendHotTubCommand("controllerindicator", "true");
}  

void restartESP() {
  ESP.restart();
}

void advertise() { 
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("Hot_Tub");
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("index.html");
  SSDP.setModelName("Time_Machine");
  SSDP.setModelNumber("929000996503");
  SSDP.setModelURL("http://phillips.dnsalias.com");
  SSDP.setManufacturer("Cat-8 Electronics");
  SSDP.setManufacturerURL("http://www.cat-8.com");
  SSDP.setDeviceType("urn:schemas-upnp-org:device:HotTubController:1");
  if(SSDP.begin()){
    DBG_OUTPUT_PORT.println("DEBUG:SSDP started");
  }
}
 
void presentation()
{
  DBG_OUTPUT_PORT.println("DEBUG:Presenting sensors...");
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Hot Tub Time Machine", "1.0");
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

