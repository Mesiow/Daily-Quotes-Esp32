#include <WiFi.h>
#include <HTTPClient.h>
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "Quote.h"
#include "secrets.h"
#include <time.h>

struct Quote displayed_quote;
struct tm now;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World");

  init_display();

  //Connect to our network
  const String ssid = WIFI_SSID;
  const String pass = WIFI_PASS;
  connect_to_wifi(ssid, pass);

  //Setup time/time zone
  setup_time_zone();

  //Fetch quote initially
  if(fetch_quote_with_retry(displayed_quote, 3)){
    display_quote(displayed_quote);
  }else{
    //if fails leave yesterdays quote on the screen etc.
  }
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

void setup_time_zone(){
  //Set to Eastern time
  configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov"); //esp32 core function
  if(getLocalTime(&now, 10000)){ //wait 10 seconds
    Serial.println(&now, "Time: %A %Y-%m-%d %H:%M:%S");
  }
}
