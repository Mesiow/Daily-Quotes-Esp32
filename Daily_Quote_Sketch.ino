#include <WiFi.h>
#include <HTTPClient.h>
#include <optional>
#include "Quote.h"
#include "secrets.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World");

  //Connect to our network
  const String ssid = WIFI_SSID;
  const String pass = WIFI_PASS;
  connect_to_wifi(ssid, pass);

  //Make a simple network request
  make_network_request();
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("Running...");
  //delay(2000);
}

void connect_to_wifi(const String &ssid, const String &password){
  WiFi.begin(ssid.c_str(), password.c_str());
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.printf("Connected to Wi-Fi network %s\n", ssid.c_str());
  }
}

void make_network_request(){
  struct Quote quote;
  if(fetch_quote_of_the_day(quote)){
     print_quote(quote);
  }else{
    Serial.println("Quote could not be fetched.");
  }
}

