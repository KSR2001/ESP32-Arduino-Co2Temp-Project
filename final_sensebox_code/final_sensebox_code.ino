
#include <SPI.h>
#include <Wire.h>
// Sensor Libraries
#include <Adafruit_GFX.h>      // http://librarymanager/All#Adafruit_GFX_Library
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <SparkFun_SCD30_Arduino_Library.h>  // http://librarymanager/All#SparkFun_SCD30_Arduino_Library
#include <Adafruit_HDC1000.h>
// WiFi stack
#include <WiFi.h>
#include <NetworkClientSecure.h>
#if __has_include("esp_eap_client.h")
#include "esp_eap_client.h"
#else
#include "esp_wpa2.h"
#endif
// Further files
#include "led_matrices.h"  // einbinden der TensorFlow Lite Modelldaten
#include "config.h"

// senseBox display
// #include <Adafruit_SSD1306.h>  // http://librarymanager/All#Adafruit_SSD1306
// alternative senseBox display library
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// blue display
// #include <Adafruit_SH110X.h> 



#define EAP_ANONYMOUS_IDENTITY \
  "DELETED FROM HERE FOR SECURITY REASONS"  // if connecting from another corporation, use \
                         // identity@organisation.domain in Eduroam
#define EAP_IDENTITY \
  "DELETED FROM HERE FOR SECURITY REASONS"  // if connecting from another corporation, use \
                         // identity@organisation.domain in Eduroam
// #define EAP_USERNAME \
//   "DELETED FROM HERE FOR SECURITY REASONS"             // oftentimes just a repeat of the identity
#define EAP_PASSWORD "DELETED FROM HERE FOR SECURITY REASONS"  // your Eduroam password

const char* ssid = "DELETED FROM HERE FOR SECURITY REASONS";
// const char* password = "DELETED FROM HERE FOR SECURITY REASONS";
NetworkClientSecure client;
// SHA1 fingerprint is broken. using root SDRG Root X1 valid until 04 Jun 2035
// 11:04:38 GMT ISRGRootX1.crt
const char* root_ca =
  "DELETED FROM HERE FOR SECURITY REASONS";

int co2_latest;
float co2_sum = 0;
int co2_index = 0;
float hdctemp;
float roundedTemp;
String temperatureText;
int temp;
const unsigned long sendingInterval = 60000;
const unsigned long measuringInterval = 5000;
const unsigned long displayInterval = 200;
unsigned long currentMillis = 0;
unsigned long startMeasuringMillis = 0;
unsigned long startSendingMillis = 0;
unsigned long startDisplayMillis = 0;
static const uint8_t NUM_SENSORS = 2;

const char* server = "DELETED FROM HERE FOR SECURITY REASONS";
typedef struct measurement {
  const char* sensorId;
  float value;
} measurement;

char buffer[750];
measurement measurements[NUM_SENSORS];
uint8_t num_measurements = 0;
const int lengthMultiplikator = 35;
uint16_t  greenColor = 0x37e6;
uint16_t  yellowColor = 0xfcc0;
uint16_t  redColor = 0xf800;



#define WIDTH 12
#define HEIGHT 8
Adafruit_NeoMatrix matrix_2 = Adafruit_NeoMatrix(WIDTH, HEIGHT, 2, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);
SCD30 airSensor;
Adafruit_HDC1000 hdc = Adafruit_HDC1000();
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// senseBox display
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// alternative senseBox display library
SSD1306Wire display(0x3D, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 100000);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
// blue display
// Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





void printOnDisplay(String title1, String measurement1, String unit1, String title2, String measurement2, String unit2, String timing) {
  // senseBox display:
  // display.setTextColor(WHITE, BLACK);
  // display.setCursor(0, 0);
  // display.setTextSize(1);
  // display.println(title1);
  // display.setCursor(0, 10);
  // display.setTextSize(2);
  // display.print(measurement1);
  // display.print(" ");
  // display.setTextSize(1);
  // display.println(unit1);  
  // display.setCursor(0, 30);
  // display.setTextSize(1);
  // display.println(title2);
  // display.setCursor(0, 40);
  // display.setTextSize(2);
  // display.print(measurement2);
  // display.print(" ");
  // display.setTextSize(1);
  // display.println(unit2);

  // if (timing != "None") {
  //   display.setCursor(80, 40);
  //   display.setTextSize(1);
  //   display.println("Next in:");
  //   display.setCursor(100, 50);
  //   display.setTextSize(1);
  //   display.println(timing);
  // }

// alternative senseBox display library:
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, title1);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 12, measurement1 + " " + unit1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 32, title2);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 44, measurement2 + " " + unit2);
  if (timing != "None") {
    display.setFont(ArialMT_Plain_10);
    display.drawString(80, 40, "Next in:");
    display.setFont(ArialMT_Plain_10);
    display.drawString(100, 50, timing);
  }

// blue display:
  // display.setTextColor(SH110X_WHITE); // 'inverted' text
  // display.setCursor(0, 0);
  // display.setTextSize(1);
  // display.println(title1);
  // display.setCursor(0, 10);
  // display.setTextSize(2);
  // display.print(measurement1);
  // display.print(" ");
  // display.setTextSize(1);
  // display.println(unit1);  
  // display.setCursor(0, 30);
  // display.setTextSize(1);
  // display.println(title2);
  // display.setCursor(0, 40);
  // display.setTextSize(2);
  // display.print(measurement2);
  // display.print(" ");
  // display.setTextSize(1);
  // display.println(unit2);
  // if (timing != "None") {
  //   display.setCursor(80, 40);
  //   display.setTextSize(1);
  //   display.println("Next in:");
  //   display.setCursor(100, 50);
  //   display.setTextSize(1);
  //   display.println(timing);
  // }
}




void addMeasurement(const char* sensorId, float value) {
  measurements[num_measurements].sensorId = sensorId;
  measurements[num_measurements].value = value;
  num_measurements++;
}


void writeMeasurementsToClient() {
  // iterate throug the measurements array
  for (uint8_t i = 0; i < num_measurements; i++) {
    sprintf_P(buffer, PSTR("%s,%9.2f\n"), measurements[i].sensorId,
              measurements[i].value);
    // transmit buffer to client
    client.print(buffer);
  }
  // reset num_measurements
  num_measurements = 0;
}


void submitValues() {
  if (WiFi.status() != WL_CONNECTED) {
    for (uint8_t wifi_timeout = 5; wifi_timeout != 0; wifi_timeout--) {
      Serial.println("Connection to WiFi lost. Reconnecting.");
      WiFi.disconnect();
      WiFi.reconnect();
      delay(5000);  // wait 5s
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
    }
  } else {
    Serial.println("Connection to WiFi still available.");
  }
  if (client.connected()) {
    Serial.println("client is still connecte, will try to end connection");
    client.stop();
    delay(1000);
  }
  bool connected = false;
  char _server[strlen_P(server)];
  strcpy_P(_server, server);
  for (uint8_t timeout = 2; timeout != 0; timeout--) {
    Serial.print("Trying to connect to server: ");
    Serial.println(_server);
    connected = client.connect(_server, 443);
    if (connected == true) {
      Serial.println("Client connected");
      // construct the HTTP POST request:
      sprintf_P(buffer, PSTR("POST /boxes/%s/data HTTP/1.1\nAuthorization: DELETED FROM HERE FOR SECURITY REASONS\nHost: %s\nContent-Type: "
                             "text/csv\nConnection: close\nContent-Length: %i\n\n"),
                SENSEBOX_ID, server, num_measurements * lengthMultiplikator);
      // send the HTTP POST request:
      client.print(buffer);
      // send measurements
      writeMeasurementsToClient();
      // send empty line to end the request
      client.println();
      uint16_t response_timeout = 0;
      // allow the response to be computed
      while (response_timeout <= 5000) {
        delay(10);
        response_timeout = response_timeout + 10;
        if (client.available()) {
          break;
        }
      }
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
        // if the server's disconnected, stop the client:
        if (!client.connected()) {
          client.stop();
          break;
        }
      }
      Serial.println();
      num_measurements = 0;
      break;
    }
    delay(1000);
  }
  if (connected == false) {
    Serial.println("Connection not possible. Going to restart device.");
    delay(5000);
    noInterrupts();
    ESP.restart();
    while (1)
      ;
  }
}
// set ESP32 mode to station and connect to SSID
void initWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA);
  // Example1 (most common): a cert-file-free eduroam with PEAP (or TTLS)
#if __has_include("esp_eap_client.h")
  esp_eap_client_set_identity((uint8_t*)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));  //provide identity
  esp_eap_client_set_username((uint8_t*)EAP_IDENTITY, strlen(EAP_IDENTITY));                      //provide username
  esp_eap_client_set_password((uint8_t*)EAP_PASSWORD, strlen(EAP_PASSWORD));                      //provide password
  esp_wifi_sta_enterprise_enable();
#else
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)EAP_ANONYMOUS_IDENTITY, strlen(EAP_ANONYMOUS_IDENTITY));  //provide identity
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)EAP_IDENTITY, strlen(EAP_IDENTITY));                      //provide username
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)EAP_PASSWORD, strlen(EAP_PASSWORD));                      //provide password
  esp_wifi_sta_wpa2_ent_enable();
#endif
  WiFi.begin(ssid);
  // WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  // Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("ok.");
  // Serial.println(WiFi.localIP());
}

// MOVED THIS from below setup
String formatTemperature(float temp) {
  float rounded = round(hdctemp * 10.0) / 10.0;  // Round to 1 decimal
  int intPart = int(rounded);
  float decimalPart = rounded - intPart;
  if (decimalPart == 0.0) {
    return String(intPart);  // e.g. "23"
  } else {
    return String(rounded, 1);  // e.g. "23.5"
  }
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting setup.");
  initWiFi();
  delay(1000);
  Serial.println("WiFi Connection established.");
  client.setCACert(root_ca);
  Serial.println("Client certificate set.");
  Wire.begin();
  delay(2500);
  
  //sensebox display
  // display.begin(SSD1306_SWITCHCAPVCC, 0x3D, true, false);
  
  // alternative sensebox display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  delay(100);

  //blue display
  // display.begin(0x3C, true); // Address 0x3C default

  // display.display();
  // delay(100);
  // display.clearDisplay();
  // display.clear();

  int co2InitCounter = 0;
  Serial.println("Init co2 sensor.");
  while (airSensor.begin(false) == false) {
    Serial.println("AirSensor not working properly");
    delay(3000);
    co2InitCounter++;
    if (co2InitCounter >= 5) {
      ESP.restart();
    }
  }

  delay(2000);
  // airSensor.enableDebugging();
  airSensor.useStaleData(true);
  Serial.println("Co2 sensor initalized.");

  Serial.println("Init hdc sensor.");
  hdc.begin();
  Serial.println("hdc sensor initalized.");

  matrix_2.setBrightness(50);
  matrix_2.begin();

  currentMillis = millis();
  startMeasuringMillis = currentMillis;
  startSendingMillis = currentMillis;
  startDisplayMillis = currentMillis;
}

void loop() {
  currentMillis = millis();
  if (currentMillis - startMeasuringMillis >= measuringInterval) {
    // Serial.println();
    // Serial.print(currentMillis - startMeasuringMillis);
    // Serial.println(" milliseconds since last measurement");
    co2_latest = airSensor.getCO2();
    temp = airSensor.getTemperature();
    hdctemp = hdc.readTemperature();
    temperatureText = formatTemperature(hdctemp);

    // sensebox display or blue display
    // display.clearDisplay();

    // alternative sensebox display
    display.clear();

    printOnDisplay("CO2", String(co2_latest), "ppm", "Temperature", String(temperatureText), String(char(247)) + "C", "5s");
    display.display();
    // Serial.println("CO2 " + String(co2_latest) + " ppm , Temperature " + String(temperatureText) + String(char(247)) + "C");
      if ((co2_latest < 1000)) {
        // matrix_2.drawRGBBitmap(0, 0, bitmap_custom(greenColor), WIDTH, HEIGHT);
        matrix_2.drawRGBBitmap(0, 0, bitmap_customGREEN, WIDTH, HEIGHT);
        matrix_2.show();
      } else if ((co2_latest >= 1000) && (co2_latest < 1500)) {
        // matrix_2.drawRGBBitmap(0, 0, bitmap_custom(yellowColor), WIDTH, HEIGHT);
        matrix_2.drawRGBBitmap(0, 0, bitmap_customYELLOW, WIDTH, HEIGHT);
        matrix_2.show();
      } else if ((co2_latest >= 1500)) {
        // matrix_2.drawRGBBitmap(0, 0, bitmap_custom(redColor), WIDTH, HEIGHT);
        matrix_2.drawRGBBitmap(0, 0, bitmap_customRED, WIDTH, HEIGHT);
        matrix_2.show();
    }
    co2_sum += co2_latest;
    co2_index++;
    startMeasuringMillis = currentMillis;
  }
  if (currentMillis - startSendingMillis >= sendingInterval) {
    // Serial.print(currentMillis - startSendingMillis);
    // Serial.println(" milliseconds since last transmission");
    
    addMeasurement(co2_sensor_id, co2_sum / co2_index);
    addMeasurement(temp_sensor_id, hdctemp);
    Serial.println("Submittig: CO2 " + String(co2_sum / co2_index) + " ppm , Temperature " + String(formatTemperature(hdctemp)) + " C");
    // Serial.println("Submitting values now!");
    submitValues();
    co2_sum = 0;
    co2_index = 0;
    startSendingMillis = currentMillis;
    if(hdctemp < -30) {
      Serial.println("\n-\n-\n-\n-\n-\n-");
      Serial.println("I2C issue is back! Going to restart in 1 sec.");
      Serial.println("\n-\n-\n-\n-\n-\n-");
      delay(1000);
      ESP.restart();
    }
  }

  if (currentMillis - startDisplayMillis >= displayInterval) {
    // sensebox display or blue display
    // display.clearDisplay();

    // alternative sensebox display
    display.clear();

    printOnDisplay("CO2", String(co2_latest), "ppm", "Temperature", String(temperatureText), String(char(247)) + "C", String((measuringInterval - (currentMillis - startMeasuringMillis)) / 1000) + "s");
    display.display();
    startDisplayMillis = currentMillis;
  }
}