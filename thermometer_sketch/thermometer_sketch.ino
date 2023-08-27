#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_PCT2075.h>
#include <SPI.h>
#include <WiFi.h>

#include "history.h"
#include "secrets.h"
#include "trend.h"

// Configuration settings.
int const save_data_period = 30 * 1000; // Save data to history period.
int const report_period = 60 * 1000;  // Iota report period in ms.
int const update_period = 1000;  // Looping period in ms.
float const long_term_threshold = 0.3;  // Long-term temperature delta in C for trends.
float const short_term_threshold = 0.3;  // Short-term temperature delta in C for trends.

// Connected devices.
Adafruit_PCT2075 thermometer = Adafruit_PCT2075();
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Global storage.
History long_term_hist = History();
History short_term_hist = History();
unsigned int save_time = 0;
unsigned int report_time = 0;

// Network stuff.
WiFiClient http_client;
IPAddress ip_addr;

/*
  Initialize the device.
*/
void setup() {
  // Configure hardware.
  Serial.begin(115200);
  setup_display();
  setup_thermometer();

  // Network setup.
  Serial.println("Connecting wifi...");
  WiFi.begin(secrets.ssid, secrets.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to wifi.");
  log_wifi();
  
  // Register with Iota.
  ip_addr.fromString(secrets.iota_ip);
  register_thing();
}

/*
  Event loop.
*/
void loop() {
  // Measure temperature.
  float temperature = update_temperature();
  short_term_hist.push(temperature);

  if (save_time >= save_data_period) {
    save_time = 0;
    long_term_hist.push(temperature);
    log_hist(long_term_hist);
  }

  if (report_time >= report_period) {
    report_time = 0;
    update_iota(temperature);
  }
  
  delay(update_period);
  save_time += update_period;
  report_time += update_period;
}

/*
  Set up TFT display.
*/
void setup_display() {
  Serial.println("Initializing display...");

  // Turn on backlight.
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // Turn on TFT / I2C power supply.
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT.
  display.init(135, 240); // Init ST7789 240x135.
  display.setRotation(3);
  display.fillScreen(ST77XX_BLACK);

  Serial.println("Display initialized.");
}

/*
  Set up thermometer hardware.
*/
void setup_thermometer() {
  Serial.println("Initializing thermometer...");
  
  if (!thermometer.begin()) {
    Serial.println("Couldn't find thermometer.");
    while (1);
  }
  
  Serial.println("Thermometer initialized.");
}

/*
  Convert Celsius to Farenheit.
*/
float farenheit(float const temp) {
  return 9.0 / 5.0 * temp + 32.0;
}

/*
  Read thermometer.
*/
float update_temperature() {
  static Trend last_change = TREND_STEADY;

  float temp_c = thermometer.getTemperature();
  float temp_f = farenheit(temp_c);
  short_term_hist.push(temp_c);
  Trend long_term_change = trend(long_term_hist, long_term_threshold);
  Trend short_term_change = trend(short_term_hist, short_term_threshold);
  Trend change = long_term_change == TREND_STEADY ? 
    short_term_change : long_term_change;

  float last_temp = short_term_hist.previous();

  log_temp(temp_c, temp_f, change);

  if (abs(temp_f - farenheit(last_temp)) >= 0.1 || change != last_change) {
    // Display needs to be refreshed.
    Serial.println("Updating display.");
    display_temp(temp_c, temp_f, change);
    last_change = change;
  }

  return temp_c;
}

/*
  Display temperature data on TFT.
*/
void display_temp(float temp_c, float temp_f, Trend change) {
  display.fillScreen(ST77XX_BLACK);
  display.setCursor(0, 0);
  display.setTextWrap(false);
  display.setTextSize(6);
  display.setTextColor(ST77XX_GREEN);
  display.print(temp_c, 1);
  display.print("C");
  display.setCursor(0, 68);
  display.setTextColor(ST77XX_MAGENTA);
  display.print(temp_f, 1);
  display.print("F");

  unsigned char trend = 0x01;
  uint16_t color = ST77XX_WHITE;
  if (change == TREND_FALLING) {
    trend = 0x19;  // down arrow.
    color = ST77XX_BLUE;
  }
  else if (change == TREND_STEADY) {
    trend = 0x1A;  // side arrow.
    color = ST77XX_YELLOW;
  }
  else if (change == TREND_RISING) {
    trend = 0x18;  // up arrow.
    color = ST77XX_RED;
  }
  display.drawChar(204, 42, trend, color, ST77XX_BLACK, 6);
}

/*
  Output temperature info to serial port.
*/
void log_temp(float temp_c, float temp_f, Trend change) {
  Serial.print(temp_c);
  Serial.print(" C / ");
  Serial.print(temp_f);
  Serial.print(" F -- ");
  Serial.println(trend_str(change));
}

/*
  Register with Iota.
*/
void register_thing() {
  char const * fmtstr = "{ \"id\": \"%s\", \"temperature\": \"REAL\" }";
  char body[strlen(fmtstr) + strlen(secrets.iota_id) + 1] = {0};
  sprintf(body, fmtstr, secrets.iota_id);
  Serial.print("Registering with Iota as ");
  Serial.println(secrets.iota_id);

  if (http_client.connect(ip_addr, secrets.iota_port)) {
    http_client.println("POST /api/things HTTP/1.1");
    http_client.println("Content-Type: application/json");
    http_client.print("Content-Length: ");
    http_client.println(strlen(body));
    http_client.println();
    http_client.println(body);
    log_http_response();    
  } else {
    Serial.println("Registration failed. http_client.connect() failed.");
  }

  if (!http_client.connected()) {
    http_client.stop();
  }
}

/*
  Send temperature data to Iota.
*/
void update_iota(float temperature) {
  char const * fmtstr = "{ \"temperature\": %.1f }";
  char body[strlen(fmtstr) + 6];
  sprintf(body, fmtstr, temperature);

  Serial.print("Sending temperature data to Iota [");
  Serial.print(temperature);
  Serial.print(" ");
  Serial.print(secrets.iota_id);
  Serial.println("]");

  if (http_client.connect(ip_addr, secrets.iota_port)) {
    http_client.print("PUT /api/things/");
    http_client.print(secrets.iota_id); 
    http_client.println(" HTTP/1.1");
    http_client.println("Content-Type: application/json");
    http_client.print("Content-Length: ");
    http_client.println(strlen(body));
    http_client.println();
    http_client.println(body);
    log_http_response();    
  } else {
    Serial.println("Could not update Iota. http_client.connect() failed.");
  }

  if (!http_client.connected()) {
    http_client.stop();
  }
}

/*
  Log history data.
*/
void log_hist(History & h) {
  Serial.print("History: ");
  if (h.empty()) {
    Serial.println("[]");
    return;
  }

  Serial.print("[ ");
  for (size_t idx = 0; idx < 3; ++idx) {
    Serial.print(h.values[idx]);
    Serial.print(" ");
  }
  Serial.println("]");
}

/*
  Log WiFi info.
*/
void log_wifi() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/*
  Log HTTP response.
*/
void log_http_response() {
  while (http_client.available()) {
    Serial.write(
      http_client.read()
    );
  }
}