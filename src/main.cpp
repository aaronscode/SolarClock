#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi101.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GPS.h>
#include <Adafruit_NeoPixel.h> 
#include <Adafruit_seesaw.h>
#include <Adafruit_VS1053.h>

// 32u4, M0, M4, nrf52840 and 328p
#define BUTTON_A  13
#define BUTTON_B  12
#define BUTTON_C  11
#define LED_PIN   13

#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin
#define VS1053_RESET    -1

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 19

#define GPSECHO false

#define SEESAW_ADDR 0x36
#define SS_SWITCH   24

// Connect to the GPS on the hardware I2C port
Adafruit_GPS GPS(&Wire);

Adafruit_AHTX0 aht;
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_7segment matrix = Adafruit_7segment();
uint32_t timer = millis();
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_seesaw ss;
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);
bool disp_button_state(uint32_t pin);

int green_value = 0;

void setup() {
  Serial.begin(115200);
  
 

  matrix.begin(0x70);
  aht.begin();
  display.begin(0x3C, true); // Address 0x3C default
  GPS.begin(0x10);  // The I2C address to use is 0x10
  strip.begin();
  ss.begin(SEESAW_ADDR);
  musicPlayer.begin();
  SD.begin(CARDCS);


  strip.show();
  WiFi.setPins(8,7,4,2);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  uint32_t greenishwhite = strip.Color(64, 0, 0, 0);
  strip.fill(greenishwhite, 0, LED_COUNT);
  strip.show();

  display.display();
  musicPlayer.setVolume(10,10);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
  ss.pinMode(SS_SWITCH, INPUT_PULLUP);
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  //pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.display(); // actually display all of the above

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
  
  int32_t new_position = ss.getEncoderPosition();

  aht.getEvent(&humidity, &temp);
  matrix.print(humidity.relative_humidity);
  matrix.writeDisplay();

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.println(temp.temperature);
  display.print("Lat: "); display.print(latitude); display.println(lat);
  display.print("Lon: "); display.print(longitude); display.println(lon);
  display.print("Encoder pos: "); display.println(new_position);
  if(!disp_button_state(BUTTON_A)) display.print("A");
  if(!disp_button_state(BUTTON_B)) display.print("B");
  if(!disp_button_state(BUTTON_C)) display.print("C");
  if (! ss.digitalRead(SS_SWITCH)) {
    if(musicPlayer.stopped()) {
      musicPlayer.startPlayingFile("/track001.mp3");
    }
    else if (! musicPlayer.paused()) {
      musicPlayer.pausePlaying(true);
    } else { 
      musicPlayer.pausePlaying(false);
    }
    }

  uint32_t greenishwhite = strip.Color(0, green_value, 0, 0);
  strip.fill(greenishwhite, 0, LED_COUNT);
  strip.show();

  delay(100);
  yield();
  display.display();
  green_value += 5;
  green_value = green_value % 256;

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