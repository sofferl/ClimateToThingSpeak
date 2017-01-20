#include <NanoESP.h>

#include <SoftwareSerial.h>



#include "keys.h"

#define LED_WLAN 13

#define TEMP A0
#define LIGHT A1
#define HUMIDITY A4
#define GND A5

#define DEBUG true


NanoESP nanoesp = NanoESP();

unsigned long myChannelNumber = 216014;
String ip, mqttId;

void setup() {
  Serial.begin(19200);

  nanoesp.init();

  nanoesp.configWifiMode(STATION);
  nanoesp.configWifiStation(WLANSSID, WLANPASSWORD);

  if (nanoesp.wifiConnected()) {
    debug(F("Wifi Connected"));
    digitalWrite(LED_WLAN, HIGH);
  }
  else {
    debug(F("Wifi not Connected"));
  }

  //Print IP in Terminal
  nanoesp.getIpMac( ip,  mqttId);
  debug("My IP: " + ip);


}


void loop() {
  debug("Loop. ");

  sendValues();
  delay(60000);

}

void sendValues() {
  double temp  = getTemp();
  int light = getLight();
  int hum = getRF();
  debug("TEMP " + String(temp));
  debug("Light " + String(light));
  debug("HUM " + String(hum));

  boolean success1 = sendThingPost(  String(temp), String(hum), String(light));
  if (success1 )  {
    debug("Update Send");
  } else{
    debug ("update ERROR"); 
  }

}


//-----------------------------------------ThingsSpeak Functions------------------------------------

boolean sendThingPost( String temp, String hum, String light)
{
  boolean succes = true;
  int connId = 1;
  succes = nanoesp.newConnection(connId, "TCP", "api.thingspeak.com", 80);
  if(succes) {
    Serial.println("Succes: ");
  } else {
    Serial.println("NO Succes: ");
    return succes;
  }

  String getStr = "GET /update?api_key=";
  getStr += ThingSpeakKEY;
  getStr +="&field1"; 
  getStr +="=";
  getStr += light;
  getStr +="&field2"; 
  getStr +="=";
  getStr += temp;
  getStr +="&field3"; 
  getStr +="=";
  getStr += hum;
  getStr += "\r\n\r\n";
  Serial.println(getStr);
  succes = nanoesp.sendData(connId, getStr);
  
    if(succes) {
    Serial.println(" SEND Succes: ");
  } else {
    Serial.println("SEND NO Succes: ");
    return succes;
  }
  succes = nanoesp.closeConnection(connId);
  if(succes) {
    Serial.println("Succes: ");
  } else {
    Serial.println("NO Succes: ");
    return succes;
  }
  
  return succes;
}  



void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println("-----START---------------------------------------------------------------------------------------------------");
    Serial.println( Msg);
    Serial.println("-------END-------------------------------------------------------------------------------------------------");
 }
}

//--------------------------------Sensors----------------------------------------------------

int getRF() {
  long U, R;
  double F;

  pinMode(HUMIDITY, INPUT);       //  Ddrb.3 = 0      'Eingang
  digitalWrite(HUMIDITY, HIGH);   //  Portb.3 = 1     'Pullup ein
  U = analogRead(HUMIDITY);       //  U = Getadc(3)
  digitalWrite(HUMIDITY, LOW);    //  Portb.3 = 0     'Pullup Aus
  pinMode(HUMIDITY, OUTPUT);      //  Ddrb.3 = 1 'Nierohmig

  pinMode(GND, INPUT);            //  Ddrb.4 = 0      'Eingang
  digitalWrite(GND, HIGH);        //  Portb.4 = 1     'Pullup ein
  U += analogRead(GND);           //   U = U + Getadc(2)
  digitalWrite(GND, LOW);         //  Portb.4 = 0     'Pullup Aus
  pinMode(GND, OUTPUT);           //  Ddrb.4 = 1 'Nierohmig. entladen

  U = U / 2;
  R = 3500 * U;                 //'R=35k*(U/1023-U)
  U = 1023 - U;
  R = (U > 0) ? R / U : 999999;     //alternativ writing to: if(U>0) R = R/U; else R=999999;
  R = R - 450;                      //- interner Bahnwiderstand 0,45 k

  //Calculate Humidity
  F = R / 280;
  F = log(F);                                                //ln f !!!
  F = F * 8.9;
  F = 100 - F;
  F = round(F);

  // Serial.println(String(R) + "Ohm     " + String(F) + "%");
  //delay(500);
  return F;
}

double getTemp() {
  //Source: http://playground.arduino.cc/ComponentLib/Thermistor2
  double Temp;
  int RawADC = analogRead(TEMP);
  Temp = log(10000.0 * ((1024.0 / RawADC - 1)));
  //         =log(10000.0/(1024.0/RawADC-1)) // for pull-up configuration
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  // Temp = (Temp * 9.0)/ 4.7 + 32.0; // Convert Celcius to Fahrenheit

  Temp = round(Temp * 10);
  Temp = Temp / 10;
  return Temp;
}

int getLight() {
  int vLight = analogRead(LIGHT);
  return vLight;
}



