String getAirState() {
  DBG_OUTPUT_PORT.println("DEBUG:getAuxState");
  return auxState;
}

String getAuxState() {
  DBG_OUTPUT_PORT.println("DEBUG:getAuxState");
  return airState;
}

String getHumidity() {
  DBG_OUTPUT_PORT.println("DEBUG:getHumidity");
  return humidity;
}

String getLightState() {
  DBG_OUTPUT_PORT.println("DEBUG:getLightState");
  return lightState;
}

String getPowerState() {
  DBG_OUTPUT_PORT.println("DEBUG:getPowerState");
  return powerState;
}
String getOutsideTemp() {
  DBG_OUTPUT_PORT.println("DEBUG:getOutSideTemp");
  return outsideTemp;
}

String getpointTemp() {
  DBG_OUTPUT_PORT.println("DEBUG:getpointTemp");
  return setpoint;
}

String getPumpState() {
  DBG_OUTPUT_PORT.println("DEBUG:getPumpState");
  return pumpState;
}

String getStatus() {
  DBG_OUTPUT_PORT.println("DEBUG:getStatus");
  return htStatus;
}

String getTime() {
  DBG_OUTPUT_PORT.println("DEBUG:getTime");
  return currentTime;
}

String getWaterTemp() {
  DBG_OUTPUT_PORT.println("DEBUG:getWaterTemp");
  return waterTemp;
}


// setters
void setAirState(String putVal, bool publish) {
  airState = putVal;
  if (publish == true) sendHotTubCommand("air", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setAirState: ");
  DBG_OUTPUT_PORT.println(airState);
}

void setAuxState(String putVal, bool publish) {
  auxState = putVal;
  if (publish == true) sendHotTubCommand("aux", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setAuxState: ");
  DBG_OUTPUT_PORT.println(auxState);
}

void setHumidity(String putVal, bool publish) {
  humidity = putVal;
  DBG_OUTPUT_PORT.print("DEBUG:setHumidity: ");
  DBG_OUTPUT_PORT.println(humidity);
}

void setLightState(String putVal, bool publish) {
  lightState = putVal;
  if (publish == true) sendHotTubCommand("light", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setLightState: ");
  DBG_OUTPUT_PORT.println(lightState);
}

void setOutsideTemp(String putVal, bool publish) {
  outsideTemp = putVal;
  DBG_OUTPUT_PORT.print("DEBUG:setOutsideTemp: ");
  DBG_OUTPUT_PORT.println(outsideTemp);
}

void setpointTemp(String putVal, bool publish) {
  setpoint = putVal;
  if (publish == true) sendHotTubCommand("setPoint", setpoint);
  DBG_OUTPUT_PORT.print("DEBUG:setpointTemp: ");
  DBG_OUTPUT_PORT.println(setpoint);
}

void setPowerState(String putVal, bool publish) {
  powerState = putVal;
  if (publish == true) sendHotTubCommand("power", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:powerButtonPressed: ");
  DBG_OUTPUT_PORT.println(putVal);
}

void setPumpState(String putVal, bool publish) {
  pumpState = putVal;
  if (publish == true) sendHotTubCommand("pump", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setPumpState: ");
  DBG_OUTPUT_PORT.println(pumpState);
}

void setStatus(String putVal, bool publish) {
  htStatus = putVal;
  if (publish == true) sendHotTubCommand("status", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setStatus: ");
  DBG_OUTPUT_PORT.println(htStatus);
}

void setTime(String putVal, bool publish) {
  currentTime = putVal;
  if (publish == true) sendHotTubCommand("time", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setTime: ");
  DBG_OUTPUT_PORT.println(currentTime);
}

void setWaterTemp(String putVal, bool publish) {
  waterTemp = putVal;
  //if (publish == true) sendHotTubCommand("temp", putVal);
  DBG_OUTPUT_PORT.print("DEBUG:setWaterTemp: ");
  DBG_OUTPUT_PORT.println(putVal);
}


