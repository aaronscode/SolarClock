#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi101.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h> // yellow 7-segment display
#include <Adafruit_SH110X.h> // OLED screen
#include <Adafruit_VS1053.h> // music maker featerwing
#include <Adafruit_NeoPixel.h> 
#include <Adafruit_AHTX0.h>  // humidity and temperature sensor
#include <Adafruit_GPS.h>
#include <Adafruit_seesaw.h> // rotary encoder I2C board

// buttons on OLED Screen featherwing
#define BUTTON_A  13
#define BUTTON_B  12
#define BUTTON_C  11

// Pin used to drive Neopixels, shared with BUTTON_A (so we have to switch the function of this pin)
#define NEOPIXEL_PIN   13

// Pins for Music Maker Featherwing (sd card and audio data)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin
#define VS1053_RESET    -1

// Pins to communciate with the wifi module
// all internal (none are exposed via headers on the feather)
#define WIFI_CS   8
#define WIFI_IRQ  7
#define WIFI_RST  4
#define WIFI_EN   2

// "pin" for rotary encoder (seems arbitrary? 24 was the value used in the example code)
#define ROT_ENCOD_SWITCH  24   // not an actual pin on feather, maybe a pin on the rotary encoder board?

#define GPSECHO false // turn off echoing of gps sentences to serial console
#define NEOPIXEL_COUNT 19 // number of Neopixels in strand

#define OLED_ADDR       0x3C
#define SEVSEG_ADDR     0x70
#define GPS_ADDR        0x10
#define ROT_ENCOD_ADDR  0x36 // I2C address for rotary encoder


// Connect to the GPS on the hardware I2C port
Adafruit_GPS GPS(&Wire);

Adafruit_AHTX0 temp_sensor;
Adafruit_SH1107 oled_display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_7segment sev_seg = Adafruit_7segment();
Adafruit_NeoPixel neopixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_seesaw rot_encoder;
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

int32_t encoder_pos, encoder_dir;
uint32_t cursor_pos = 0;
bool cursor_indent = false;
uint16_t color_vals [4] = {0, 0, 0, 0};
char color_chars [4] = {'r', 'g', 'b', 'w'};

uint32_t timer = millis();

bool disp_button_state(uint32_t pin);


void setup() {

  oled_display.begin(OLED_ADDR, true); // Address 0x3C default
  oled_display.setTextSize(1);
  oled_display.setTextColor(SH110X_WHITE);
  oled_display.setCursor(0,0);
  oled_display.setRotation(1);
  oled_display.clearDisplay();
  oled_display.display();

  neopixels.begin();
  neopixels.show();

  sev_seg.begin(SEVSEG_ADDR);

  musicPlayer.begin();
  musicPlayer.setVolume(10,10);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
  SD.begin(CARDCS);

  rot_encoder.begin(ROT_ENCOD_ADDR);
  rot_encoder.pinMode(ROT_ENCOD_SWITCH, INPUT_PULLUP);
  encoder_pos = rot_encoder.getEncoderPosition();

  temp_sensor.begin();

  GPS.begin(GPS_ADDR);  // The I2C address to use is 0x10
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  WiFi.setPins(WIFI_CS, WIFI_IRQ, WIFI_RST, WIFI_EN);

  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
}

void loop() {
  sensors_event_t humidity, temp;
  float latitude, longitude;
  char lat, lon;

  char c = GPS.read();
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    if (GPS.parse(GPS.lastNMEA())) { // this also sets the newNMEAreceived() flag to false
      if (millis() - timer > 2000) {
        timer = millis();
        if(GPS.fix){
          lat = GPS.lat;
          latitude = GPS.latitude;
          lon = GPS.lon;
          longitude = GPS.longitude;
        }
      }
    }
  }
  
  int32_t new_position = rot_encoder.getEncoderPosition();
  encoder_dir = encoder_pos - new_position;
  encoder_pos = new_position;
  if(!rot_encoder.digitalRead(ROT_ENCOD_SWITCH)) {
    cursor_indent = !cursor_indent;
    encoder_dir = 0;
  }

  if(!cursor_indent) {
    cursor_pos += encoder_dir;
    cursor_pos = constrain(cursor_pos, 0, 3);
  } else {
    color_vals[cursor_pos] += 5 * encoder_dir;
    color_vals[cursor_pos] = constrain(color_vals[cursor_pos], 0, 255);
  }

  temp_sensor.getEvent(&humidity, &temp);
  //sev_seg.print(humidity.relative_humidity);
  //sev_seg.writeDisplay();

  oled_display.clearDisplay();
  oled_display.setCursor(0,0);
  for(int i = 0; i < 4; i++) {
    if(i == cursor_pos) {
      if(!cursor_indent) {
        oled_display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        oled_display.print(color_chars[i]); 
        oled_display.print(": "); 
        oled_display.println(color_vals[i]);
        oled_display.setTextColor(SH110X_WHITE, SH110X_BLACK);
      } else {
        oled_display.print(color_chars[i]); 
        oled_display.print(": "); 
        oled_display.setTextColor(SH110X_BLACK, SH110X_WHITE);
        oled_display.println(color_vals[i]);
        oled_display.setTextColor(SH110X_WHITE, SH110X_BLACK);
      }
    } else {
      oled_display.print(color_chars[i]); 
      oled_display.print(": "); 
      oled_display.println(color_vals[i]);
    }
  }
  oled_display.display();
  /*
  oled_display.print("Temperature: ");
  oled_display.println(temp.temperature);
  oled_display.print("Lat: "); oled_display.print(latitude); oled_display.println(lat);
  oled_display.print("Lon: "); oled_display.print(longitude); oled_display.println(lon);
  oled_display.print("Encoder pos: "); oled_display.println(new_position);
  if(!disp_button_state(BUTTON_A)) oled_display.print("A");
  if(!disp_button_state(BUTTON_B)) oled_display.print("B");
  if(!disp_button_state(BUTTON_C)) oled_display.print("C");
  if  {
    if(musicPlayer.stopped()) {
      musicPlayer.startPlayingFile("/track001.mp3");
    }
    else if (! musicPlayer.paused()) {
      musicPlayer.pausePlaying(true);
    } else { 
      musicPlayer.pausePlaying(false);
    }
    }
  */
  uint32_t pixel_color = neopixels.Color(color_vals[0], color_vals[1], color_vals[2], color_vals[3]);
  neopixels.fill(pixel_color, 0, NEOPIXEL_COUNT);
  neopixels.show();

  delay(100);
  yield();

}

bool disp_button_state(uint32_t pin) {
  if(pin == BUTTON_A) {
    pinMode(BUTTON_A, INPUT_PULLUP);
  }
  int state = digitalRead(pin);
  if(pin == BUTTON_A) {
    pinMode(BUTTON_A, OUTPUT);
  }
  return bool(state);
}