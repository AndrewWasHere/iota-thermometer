#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_PCT2075.h>
#include <SPI.h>

#include "history.h"

// Configuration settings.
int const report_period = 5 * 1000;  // Iota report period in ms.
int const update_period = 1000; // Looping period in ms.

// Connected devices.
Adafruit_PCT2075 thermometer = Adafruit_PCT2075();
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

History long_term_hist = History();
unsigned int elapsed_time = 0;

void setup() {
  // Initialize serial port for logging.
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  setup_display();
  setup_thermometer();
}

void loop() {
  // measure temperature.
  float temperature = update_temperature();
  if (elapsed_time >= report_period) {
    elapsed_time = 0;
    Serial.println("Pushing temperature to history.");
    long_term_hist.push(temperature);
    show_hist(long_term_hist);
  }
  
  delay(update_period);
  elapsed_time += update_period;
}

void setup_display() {
  Serial.println("Initializing display...");

  // Turn on backlight.
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // Turn on TFT / I2C power supply.
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  display.init(135, 240); // Init ST7789 240x135
  display.setRotation(3);
  display.fillScreen(ST77XX_BLACK);

  Serial.println("Display initialized.");
}

void setup_thermometer() {
  Serial.println("Initializing thermometer...");
  
  if (!thermometer.begin()) {
    Serial.println("Couldn't find thermometer.");
    while (1);
  }
  
  Serial.println("Thermometer initialized.");
}

float update_temperature() {
  static int last_temp = 0;
  static Trend last_change = TREND_STEADY;
  float temp_c = thermometer.getTemperature();
  float temp_f = 9.0 / 5.0 * temp_c + 32;
  int rounded_temp = int(temp_f + 0.5);
  Trend change = long_term_hist.trend();

  log_temp(temp_c, temp_f, change);

  if (rounded_temp != last_temp || change != last_change) {
    display_temp(temp_c, temp_f, change);
    last_temp = rounded_temp;
    last_change = change;
  }

  return temp_c;
}

void display_temp(float temp_c, float temp_f, Trend change) {
  display.fillScreen(ST77XX_BLACK);
  display.setCursor(0, 0);
  display.setTextWrap(false);
  display.setTextSize(6);
  display.setTextColor(ST77XX_GREEN);
  display.print(int(temp_c + 0.5));
  display.print(" C");
  display.setCursor(0, 68);
  display.setTextColor(ST77XX_MAGENTA);
  display.print(int(temp_f + 0.5));
  display.print(" F");

  unsigned char trend = 0x01;
  uint16_t color = ST77XX_WHITE;
  if (change == TREND_FALLING) {
    trend = 0x19;
    color = ST77XX_BLUE;
  }
  else if (change == TREND_STEADY) {
    trend = 0x1A;
    color = ST77XX_YELLOW;
  }
  else if (change == TREND_RISING) {
    trend = 0x18;
    color = ST77XX_RED;
  }
  display.drawChar(204, 42, trend, color, ST77XX_BLACK, 6);
}

void log_temp(float temp_c, float temp_f, Trend change) {
  Serial.print(temp_c);
  Serial.print(" C / ");
  Serial.print(temp_f);
  Serial.print(" F -- ");
  Serial.println(trend(change));
}

void show_hist(History & h) {
  Serial.print("History: ");
  if (h.size == 0) {
    Serial.println("[]");
    return;
  }

  Serial.print("[ ");
  for (size_t idx = 0; idx < h.size; ++idx) {
    Serial.print(h.values[idx]);
    Serial.print(" ");
  }
  Serial.println("]");
}