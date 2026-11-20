#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *zenQuoteTodayUrlEndpoint = "https://zenquotes.io/api/today";

void print_quote(const Quote &quote){
  Serial.printf("Author: %s\n", quote.author.c_str());
  Serial.printf("Quote: %s\n", quote.quote.c_str());
  Serial.printf("Image Url: %s\n", quote.imageUrl.c_str());
  Serial.printf("Date: %s\n", quote.date.c_str());
}

bool fetch_quote_of_the_day(Quote &quote){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    http.useBuiltinCACertBundle();
    //Specify endpoint url
    http.begin(zenQuoteTodayUrlEndpoint);

    //Send the request and get the HTTP response code
    int responseCode = http.GET();
    if(responseCode == HTTP_CODE_OK){
      String payload = http.getString();

      http.end();
      return parse_json_payload(quote, payload);
    }else{
       Serial.printf("Request failed: %s\n", http.errorToString(responseCode).c_str());
       return false;
    }

    //Free resource
    http.end();
  }else{
    Serial.println("Device not connected to a wifi network, could not fetch quote.");
  }
  return false;
}

bool fetch_quote_with_retry(Quote &quote, int attempts){
  for (int i = 1; i <= attempts; i++) {
      if (fetch_quote_of_the_day(quote)) return true;
      Serial.printf("Fetch attempt %d/%d failed\n", i, attempts);
      if (i < attempts) delay(2000 * i);      // wait 2s, then 4s, ...
    }
  return false;
}

bool parse_json_payload(Quote &quote, const String &payload){
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);

  if(err){
    Serial.printf("JSON parse failed: %s\n", err.c_str());
    return false;
  }

  quote.quote = doc[0]["q"] | ""; //the text, or "" if "q" is missing
  quote.author = doc[0]["a"] | "";
  quote.imageUrl = doc[0]["i"] | "";
  quote.date = doc[0]["date"] | "";

  return true;
}