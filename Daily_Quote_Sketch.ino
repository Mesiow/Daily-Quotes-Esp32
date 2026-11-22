#include <WiFi.h>
#include <HTTPClient.h>
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "Quote.h"
#include "secrets.h"
#include <time.h>

const int UPDATE_HOUR = 7;
const int UPDATE_MIN = 0;
// The UL suffix makes the arithmetic unsigned long, the same type millis() returns.
const unsigned long CHECK_INTERVAL_MS = 1000UL; //check clock every sec
const unsigned long RETRY_INTERVAL_MS = 5UL * 60 * 1000; //wait 5 min after a failure

int last_update_day = -1; //tm_yday of last successful update (-1 == never)
unsigned long last_check_ms = 0;
unsigned long last_attempt_ms = 0;
bool has_attempted = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World");

  init_display();

  //Connect to our network
  connect_to_wifi(WIFI_SSID, WIFI_PASS);

  //Setup time/time zone
  setup_time_zone();

  update_quote();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);

  if(millis() - last_check_ms < CHECK_INTERVAL_MS) return; //hasnt been 30 sec yet
  last_check_ms = millis();

  struct tm time;
  if(!getLocalTime(&time, 0)) return; //clock not synced

  bool past_update_time = (time.tm_hour * 60 + time.tm_min) >= (UPDATE_HOUR * 60 + UPDATE_MIN);
  bool updated_today = (time.tm_yday == last_update_day);

  if(!past_update_time || updated_today) return; //don't update yet

  //if we've already had a fetch quote attempt and the last attempt is less than the retry interval return, otherwise update
  if(has_attempted && millis() - last_attempt_ms < RETRY_INTERVAL_MS) return; //failed, so wait

  Serial.println(&time, "%H:%M - daily update due, fetching quote");
  update_quote();
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
  struct tm time;
  if(getLocalTime(&time, 5000)){ //wait 5 seconds
    Serial.println(&time, "Time: %A %Y-%m-%d %H:%M:%S");
  }
}

bool update_quote(){
  last_attempt_ms = millis();
  has_attempted = true;

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Wi-Fi offline, reconnecting...");
    WiFi.reconnect();
    return false;
  }

  struct Quote quote;
  if(!fetch_quote_with_retry(quote, 3)){
    return false; //coudlnt fetch, quote, return
  }

  display_quote(quote);

  struct tm time;
  if(getLocalTime(&time, 0)){
    last_update_day = time.tm_yday; //tm_yday = days since jan first
  }
  return true;
}
