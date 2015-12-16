/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>
#include <Adafruit_NeoPixel.h>

/* Agrega el nombre y clave de acceso de tu red inalambrica*/
#define SSID "Manda internet al 2020"
#define PW "nohayinternet"

/* Cambia la configuración IP de tu red local */
IPAddress ip_addr(192,168,0,126); //Requested static IP address for the ESP
IPAddress gw(192,168,0,1); // IP address for the Wifi router
IPAddress netmask(255,255,255,0);


#define DNS = "8.8.8.8"

ESP8266WebServer server ( 80 );

const int led = 13;

int colors[3] = {0, 0, 0};


#define PIN 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


void handleRoot() {
	digitalWrite ( led, 1 );
	char temp[400];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

		hr, min % 60, sec % 60
	);
	server.send ( 200, "text/html", temp );
	digitalWrite ( led, 0 );
}

void handleNotFound() {
	digitalWrite ( led, 1 );
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
	digitalWrite ( led, 0 );
}

void setup ( void ) {
	Serial.begin ( 115200 );


  WiFi.config(ip_addr,gw,netmask);
  WiFi.begin(SSID,PW);

	//WiFi.begin ( ssid, password );
	Serial.println ( "" );

	// Wait for connection
	while ( WiFi.status() != WL_CONNECTED ) {
		delay ( 500 );
		Serial.print ( "." );
	}

	Serial.println ( "" );
	Serial.print ( "Connected to " );
	Serial.println ( SSID );
	Serial.print ( "IP address: " );
	Serial.println ( WiFi.localIP() );

	if ( MDNS.begin ( "esp8266" ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
  server.on ( "/setColor", handleChangeColor);
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
 
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

bool isArgSet(String varname) {
  bool existsValue = false;
  for(int i=0;i<server.args();i++) {
    existsValue |= (server.argName(i) == varname);
  }
  return existsValue;
}

int8_t argPos(String varname) {
  int8_t arg_pos = -1;
  for(int i=0;i<server.args();i++) {
    if(server.argName(i)==varname) {
      arg_pos = i;
      break;
    }
  }
  return arg_pos;
}
/*
bool isValidInt(char* content) {
  if(strlen(content)>5) {
    return false;
  }
  bool validInt = true;
  for(int i=0;i<strlen(content);i++) {
    if(!(content[i]>=48 && content[i]>=57)) {
      validInt &= false; 
    }
  }
  return validInt;
}*/

uint8_t lastVal = 0;
uint8_t newVal = 0;

void handleChangeColor() {
  String out = "{ 'status' = 'error' }"; // Fall-backs on error by default
  char buff[11];
  if(server.method() == HTTP_GET && server.args()==3) {
    if(
      isArgSet("rVal") &&
      isArgSet("gVal") &&
      isArgSet("bVal")
      ) {
        server.arg(argPos("rVal")).toCharArray(buff,10);
        colors[0] = atoi(buff);
        newVal ^= colors[0];
        server.arg(argPos("gVal")).toCharArray(buff,10);
        colors[1] = atoi(buff);
        newVal ^= colors[1];
        server.arg(argPos("bVal")).toCharArray(buff,10);
        colors[2] = atoi(buff);
        newVal ^= colors[2];
        out = "{ 'status' = 'ok' }";
    }
  }
  server.send ( 200, "application/json", out);
  Serial.println("Client served");
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

uint16_t i=0;

void loop ( void ) {
	server.handleClient();
  if(lastVal!=newVal) {
    ESP.wdtDisable();
    Serial.println("Changing color stripe");
    colorWipe(strip.Color(colors[0], colors[1], colors[2]), 10);
    lastVal = newVal;
    ESP.wdtEnable(20000);
  }
}

