
#include <SPI.h>
#include <TFT_eSPI.h>
// #include <XPT2046_Touchscreen.h>
// TODO maybe implement touch screen?? web page possibly better
#include "Free_Fonts.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
AsyncWebServer server(80);

const char* ssid = "SSID";
const char* password = "PASS";

const char* PARAM_INPUT = "input";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Stock Ticker <input type="text" name="input" size="4" maxlength="4" style="text-transform:uppercase">
  </form>
</body></html>)rawliteral";

// gooder api found, 60 calls per minute HELL YEAH
const char *test_root_ca = R"literal(
-----BEGIN CERTIFICATE-----
MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX
MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE
CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx
NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT
GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0
MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube
Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e
WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd
BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd
BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN
l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw
Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v
Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG
SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ
odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY
+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs
kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep
8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1
vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl
-----END CERTIFICATE-----
)literal";


WiFiClientSecure client;

String response;

// take in a string for the ticker symbol
// TODO add error handling && maybe change to AsyncHTTPClient
String HTTPSRequest(String ticker) {
  String response = "";

  WiFiClientSecure client;
  client.setCACert(test_root_ca);

  HTTPClient http;

  String apiurl = "https://finnhub.io/api/v1/quote?symbol=" + ticker + "&token=API KEY";

  // using secure client!!
  http.begin(client, apiurl); 

  int httpCode = http.GET();

  if (httpCode > 0) { 
    response = http.getString(); 
    Serial.println("Response received:");
    Serial.println(response);
  } else {
    Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return response; 
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// to upper for a whole string
void to_upper(const char *str, char *out_str)
{
  while(*str != 0) {
    *out_str = toupper(*str);
    ++str;
    ++out_str;
  }
  *out_str = 0;
}

float truncateDecimal(float value) {
    return (int)(value * 100) / 100.0;
}

TFT_eSPI tft = TFT_eSPI();


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

String inputMessage = "Waiting for input";  // the input from the web page form
bool inputUpdated = false;  // flag for when the input is updated

// temp var for upperstring
char upperString[100];

void setup() {
  Serial.begin(115200);


  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("esp32-web");

  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputParam;
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      to_upper(inputMessage.c_str(), upperString);
      inputMessage = upperString;
      inputUpdated = true;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println("Web Form Input:");
    Serial.println(inputMessage);
    request->redirect("/");
  });

  server.onNotFound(notFound);
  server.begin();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  tft.drawString("Connected", 10, 220, 2);
  String tempText = "IP: " + WiFi.localIP().toString();
  tft.drawString(tempText, 90, 220, 2);

  tft.setFreeFont(FF17);  

  tft.setTextSize(2);

  tft.drawCentreString("Waiting for input", centerX, centerY, FONT_SIZE);
}

void loop() {
  // if the input is updated, make the request for the ticker symbol
  if (inputUpdated) {
    
    // tft.fillScreen(TFT_BLACK);

    response = HTTPSRequest(inputMessage);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);

    String price;
    if (doc["c"].is<int>()) {
        price = String(doc["c"].as<int>()) + ".00";
    } else if (doc["c"].is<float>()) {
        price = String(doc["c"].as<float>(), 2);
    } else {
        price = "error";
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.fillRect(0, 0, SCREEN_WIDTH, 180, TFT_BLACK);

    tft.setTextSize(3);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(inputMessage, 10, 10);
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);
    String tempText = "$" + price;
    tft.drawString(tempText, 10, 70);

    String change;

    if (doc["d"].is<int>()) {
        change = String(doc["d"].as<int>()) + ".00";
    } else if (doc["d"].is<float>()) {
        change = String(doc["d"].as<float>(), 2);
    } else {
        change = "error";
    }

    if (doc["dp"].is<int>()) {
        change += " (" + String(doc["dp"].as<int>()) + ".00%)";
    } else if (doc["dp"].is<float>()) {
        float dp = truncateDecimal(doc["dp"].as<float>());
        change += " (" + String(dp, 2) + "%)";
    } else {
        change = "error";
    }

    if (doc["d"] > 0) {
        change = "+" + change;
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
    }

    tft.setTextSize(2);
    tft.drawString(change, 10, 140);

    inputUpdated = false; 
  }
}