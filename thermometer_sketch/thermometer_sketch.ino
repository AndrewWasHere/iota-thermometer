#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_PCT2075.h>
#include <SPI.h>

#include "history.h"
#include "secrets.h"

enum Trend { TREND_FALLING, TREND_STEADY, TREND_RISING };

// Configuration settings.
int const save_data_period = 5 * 1000; // Save data to history period.
int const report_period = 6 * 1000;  // Iota report period in ms.
int const update_period = 1000;  // Looping period in ms.
float const long_term_threshold = 1.0;  // Long-term temperature delta in C for trends.
float const short_term_threshold = 0.3;  // Short-term temperature delta in C for trends.

// Connected devices.
Adafruit_PCT2075 thermometer = Adafruit_PCT2075();
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Global storage.
History long_term_hist = History();
History short_term_hist = History();
unsigned int save_time = 0;
unsigned int report_time = 0;

/*
  Initialize the device.
*/
void setup() {
  // Configure hardware.
  Serial.begin(115200);
  setup_display();
  setup_thermometer();

  // Register with Iota.
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
    Serial.println("Pushing temperature to history.");

    save_time = 0;
    long_term_hist.push(temperature);
    log_hist(long_term_hist);
  }

  if (report_time >= report_period) {
    Serial.println("Pushing temperature to Iota.");

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
  Read thermometer.
*/
float update_temperature() {
  static Trend last_change = TREND_STEADY;

  float temp_c = thermometer.getTemperature();
  float temp_f = farenheit(temp_c);
  short_term_hist.push(temp_c);
  Trend short_term_change = trend(short_term_hist, short_term_threshold);
  Trend change = short_term_change == TREND_STEADY ? 
    trend(long_term_hist, long_term_threshold) :
    short_term_change;

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
  Output `h` to serial port.
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
  Determine current Trend based on `h` and `threshold`.
*/
Trend trend(History & h, float const threshold) {
  if (!h.full() && !h.empty()) {
    float delta = h.latest() - h.previous();
    if (delta >= threshold) {
      return TREND_RISING;
    }
    if (delta <= -threshold) {
      return TREND_FALLING;
    }
    return TREND_STEADY;
  }

  if (h.full()) {
    float delta = h.latest() - h.first();

    if (
      h.first() < h.previous() && h.previous() < h.latest() && 
      delta >= threshold
    ) {
      return TREND_RISING;
    }
    if (
      h.first() > h.previous() && h.previous() > h.latest() && 
      delta <= -threshold
    ) {
      return TREND_FALLING;
    }
    return TREND_STEADY;
  }

  return TREND_STEADY;
}

/*
  Human-readable Trend.
*/
char * const trend_str(Trend const t) {
  if (t == TREND_FALLING) return "falling";
  if (t == TREND_STEADY) return "steady";
  if (t == TREND_RISING) return "rising";
  else return "unknown trend";
}

/*
  Convert Celsius to Farenheit.
*/
float farenheit(float const temp) {
  return 9.0 / 5.0 * temp + 32.0;
}


/*
  Register with Iota
*/
void register_thing() {
  Serial.print("Registering with Iota as ");
  Serial.println(secrets.iota_id);
}

/*
  Send temperature data to Iota.
*/
void update_iota(float temperature) {
  Serial.print("Sending temperature data to Iota [");
  Serial.print(temperature);
  Serial.print(" ");
  Serial.print(secrets.iota_id);
  Serial.println(" ]");
}