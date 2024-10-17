
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 4

const char *ssid = "";          // your network SSID (name of wifi network)
const char *password = "";  // your network password

const char *server = "www.alphavantage.co";  // Server URL

// root certificate for alphavantage.co
// TODO find a better fucking api

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

String HTTPSRequest() {

  String response = "";

  // set the ssl/tls cert
  client.setCACert(test_root_ca);

  Serial.println("\nStarting connection to server...");

  // if da conektion failed
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
  } else { // suksess
    Serial.println("Connected to server!");
    client.println("GET https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=NVDA&apikey=API_KEY_HERE HTTP/1.0");
    client.println("Host: www.alphavantage.co");
    client.println("Connection: close");
    client.println();

    // while connected 
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    // if there are bytes available read and append them
    while (client.available()) {
      String line = client.readStringUntil('\n');
      response += line;
    }

    // TODO fix everything?
    client.stop();
  }

  // return tha juicy json
  return response;

}

void setup() {
  Serial.begin(115200);

  // start the spi for the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // set touchscreen rotation

  touchscreen.setRotation(1);

  // start display
  tft.init();
  // set DISPLAY rotation
  tft.setRotation(1);

  // clear the screen with white quickly
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  
  // screen cenetres 
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;

  // connect to ssid with the password & output console
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  // output console the ssid that was connected to
  Serial.print("Connected to ");
  Serial.println(ssid);

  // draw string to the tft that conneciton was successful && local ip
  tft.drawString("Connected", 10, 220, 2);
  tft.drawString("IP: " + WiFi.localIP().toString(), 90, 220, 2);

  // get the response from the api
  // STOP JUDGING THIS IS A MAJOR PROOF OF CONCEPT
  response = HTTPSRequest();

  // prep repsonse for json parsing w/ arduinojson
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, response);

  // parse stock symbol and price
  String ticker = doc["Global Quote"]["01. symbol"];
  String price = doc["Global Quote"]["05. price"];

  // output the unparsed json response to console
  Serial.println(response);

  // draw ticker 
  String tempText = "Ticker: " + ticker;
  tft.drawCentreString(tempText, centerX, 30, 8);

  // draw price
  tempText = "Price: " + price;
  tft.drawCentreString(tempText, centerX, 120, FONT_SIZE);
}

void loop() {
// int bank_account = 999999999;
}