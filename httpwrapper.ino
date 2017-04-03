void sendJSONResponse(String responseData) {
  String response = "{\"value\":\"";
  response += responseData;
  response += "\"}";
  server.send(200, "text/json", response);
  response = String();
}

void httpSetPowerState() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setPowerState(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetPowerState() {
  sendJSONResponse(getPowerState());
}


void httpSetWaterTemp() {
  // test only
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setWaterTemp(putVal,false);
  sendJSONResponse("OK");
  putVal = String();
}
  
void httpGetWaterTemp() {
  sendJSONResponse(getWaterTemp());
}

void httpSetpointTemp() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setpointTemp(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}
  
void httpGetpointTemp() {
  sendJSONResponse(getpointTemp());
}

void httpSetPumpState() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setPumpState(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetPumpState() {
  sendJSONResponse(getPumpState());
}

void httpSetAuxState() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setAuxState(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetAuxState() {
  sendJSONResponse(getAuxState());
}

void httpSetAirState() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setAirState(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetAirState() {
  sendJSONResponse(getAirState());
}

void httpSetLightState() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setLightState(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetLightState() {
  sendJSONResponse(getLightState());
}

void httpSetTime() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setTime(putVal,true);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetTime() {
  sendJSONResponse(getTime());
}

void httpGetNtpTime() {
  sendJSONResponse(getControllerTime());
}

void httpGetOutsideTemp() {
  sendJSONResponse(getOutsideTemp());
}

void httpGetPressure() {
  sendJSONResponse(getPressure());
}

void httpGetForecast() {
  sendJSONResponse(String(getForecast()));
}

void httpSetStatus() {
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String putVal = server.arg(0);
  setStatus(putVal,false);
  sendJSONResponse("OK");
  putVal = String();
}

void httpGetStatus() {
  sendJSONResponse(getStatus());
}

void httpGetHumidity() {
  sendJSONResponse(getHumidity());
}

void httpSendPresentation() {
  presentation();
  sendJSONResponse("OK");
}

void httpSendAdvertisement() {
  advertise();
  sendJSONResponse("OK");
}

void httpProgramMode() {
  Serial.swap();
  sendJSONResponse("OK");
}

void httpSendUpdateAll() {
  DBG_OUTPUT_PORT.println("Updating all sensor values");
  sendJSONResponse(updateAllSensors());
}

