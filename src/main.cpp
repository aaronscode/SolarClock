#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h> // yellow 7-segment display
#include <Adafruit_SH110X.h> // OLED screen
#include <Adafruit_VS1053.h> // music maker featerwing
#include <Adafruit_NeoPixel.h> 
#include <Adafruit_AHTX0.h>  // humidity and temperature sensor
#include <Adafruit_GPS.h>
#include <Adafruit_seesaw.h> // rotary encoder I2C board

#include "secrets.h"
#include "menu.h"
#include "menu_entry.h"

// buttons on OLED Screen featherwing
#define BUTTON_A  13
#define BUTTON_B  12
#define BUTTON_C  11
// "pin" for rotary encoder (seems arbitrary? 24 was the value used in the example code)
#define ROT_ENCOD_SWITCH  24   // not an actual pin on feather, maybe a pin on the rotary encoder board?

#define DISP_SWITCHA_IDX 0 
#define DISP_SWITCHB_IDX 1 
#define DISP_SWITCHC_IDX 2 
#define ENCOD_SWITCH_IDX 3

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

Menu main_menu = Menu(true);
Menu lamp_color_menu = Menu(true);
Menu *active_menu;

bool GPS_status, temp_sensor_status, oled_status, sev_seg_status, neopixel_status, rot_encoder_status, music_player_status;
int32_t encoder_pos, encoder_dir;
uint8_t buttons_state = 0;
int color_vals [4] = {255, 243, 0, 0};
char color_chars [4] = {'r', 'g', 'b', 'w'};
const char *color_strs[] = {"r", "g", "b", "w"};
sensors_event_t humidity, temp;
float latitude, longitude;
char lat, lon;

int wifi_status = WL_IDLE_STATUS;
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

uint32_t timer = millis();

bool disp_button_state(uint32_t pin);
uint8_t get_buttons_debounce(uint8_t last_buttons_state);
void draw_menu(Adafruit_SH1107& oled);
void update_menu(int32_t encoder_dir);



const char *main_menu_entries[] = {"Lamp On/Off", "Adjust Color", "Set Alarm", "Weather", "Device Status", "Network Info", "GPS", "test entry", "test entry2", "test entry 3", "test entry 4"};
#define MAIN_MENU_COUNT 11;
bool light_on = false;
bool cursor_indent = false;

void setup() {

  oled_display.begin(OLED_ADDR, true); // Address 0x3C default
  oled_display.setTextSize(1);
  oled_display.setTextColor(SH110X_WHITE);
  oled_display.setCursor(0,0);
  oled_display.setRotation(1);
  oled_display.clearDisplay();
  oled_display.display();

  neopixels.begin();
  neopixels.clear();
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
  wifi_status = WiFi.begin(WLAN_SSID, WLAN_PASS);
  Udp.begin(localPort);

  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  MenuEntry lamp_color_entries[4];
  for(int i = 0; i < 4; i++) {
    lamp_color_entries[i] = MenuEntry(color_strs[i], &color_vals[i], 0, 255);
  }
  lamp_color_menu = Menu(false, lamp_color_entries, 4);

  MenuEntry main_entries[11];
  main_entries[0] = MenuEntry(main_menu_entries[0], &light_on, 0, 0);
  main_entries[1] = MenuEntry(main_menu_entries[1], &lamp_color_menu);
  for(int i = 2; i < 11; i++) {
    main_entries[i] = MenuEntry(main_menu_entries[i]);
  }
  main_menu = Menu(true, main_entries, 11);
  active_menu = &main_menu;

}

void loop() {

  buttons_state = get_buttons_debounce(buttons_state);

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
  bool encoder_button = false;
  encoder_dir = encoder_pos - new_position;
  encoder_pos = new_position;
  if(buttons_state & 1 << ENCOD_SWITCH_IDX) {
    encoder_button = true; 
    encoder_dir = 0;
  }

  /*
  if(!cursor_indent) {
    cursor_pos += encoder_dir;
    cursor_pos = constrain(cursor_pos, 0, 3);
  } else {
    color_vals[cursor_pos] += 5 * encoder_dir;
    color_vals[cursor_pos] = constrain(color_vals[cursor_pos], 0, 255);
  }
  */

  temp_sensor.getEvent(&humidity, &temp);
  //sev_seg.print(humidity.relative_humidity);
  //sev_seg.writeDisplay();

  Menu *update = active_menu->UpdateMenu(encoder_dir, encoder_button);
  if(update != nullptr) {
    active_menu = update;
  }
  active_menu->DrawMenu(oled_display);
  //oled_display.setCursor(100, 20);
  //oled_display.print(encoder_button);

  /*
  if(buttons_state & 1 << DISP_SWITCHA_IDX) oled_display.print("A");
  if(buttons_state & 1 << DISP_SWITCHB_IDX) oled_display.print("B");
  if(buttons_state & 1 << DISP_SWITCHC_IDX) oled_display.print("C");

  oled_display.println();
  if(wifi_status == WL_CONNECTED) {
    oled_display.println("Connected to:"); oled_display.println(WLAN_SSID);
  } else {
    oled_display.print("Not connected.\n Error code: "); oled_display.println(wifi_status);
  }
  oled_display.drawBitmap(50, 0, epd_bitmap_BackArrow, 5, 7, SH110X_WHITE);
  oled_display.print("Temperature: ");
  oled_display.println(temp.temperature);      cursor_pos = constrain(cursor_pos, 0, current_menu_count - 1);
      if(cursor_pos < top_menu_item) {
        top_menu_item = cursor_pos;
      } else if (cursor_pos >= (top_menu_item + OLED_TEXTROWS + 1)) {
        top_menu_item = cursor_pos - OLED_TEXTROWS;
      }
  oled_display.print("Lat: "); oled_display.print(latitude); oled_display.println(lat);
  oled_display.print("Lon: "); oled_display.print(longitude); oled_display.println(lon);
  oled_display.print("Encoder pos: "); oled_display.println(new_position);
  if  {
    if(musicPlayer.stopped()) {
      musicPlayer.startPlayingFile("/track001.mp3");
    }
    else if (! musicPlayer.paused()) {
      musicPlayer.pausePlaying(true);
    } else { 
      musicPlayer.pausePlaying(false);
    }  if(pin == BUTTON_A) {
    pinMode(BUTTON_A, OUTPUT);
  }
  */
  oled_display.display();

  uint32_t pixel_color = 0;
  if(light_on) {
    pixel_color = neopixels.Color(color_vals[0], color_vals[1], color_vals[2], color_vals[3]);
  }
  neopixels.fill(pixel_color);
  neopixels.show();

  yield();

}

bool disp_button_state(uint32_t pin) {
  if(pin == NEOPIXEL_PIN) {
    pinMode(NEOPIXEL_PIN, INPUT_PULLUP);
    int state = digitalRead(pin);
    pinMode(NEOPIXEL_PIN, OUTPUT);
    return bool(state);
  } else
    return bool(digitalRead(pin));
}

uint8_t get_buttons_debounce(uint8_t last_buttons_state) {
  uint8_t buttons_state = 0;
  buttons_state |= !disp_button_state(BUTTON_A);
  buttons_state |= !disp_button_state(BUTTON_B) << 1;
  buttons_state |= !disp_button_state(BUTTON_C) << 2;
  buttons_state |= !rot_encoder.digitalRead(ROT_ENCOD_SWITCH) << 3;
  delay(50);
  if(buttons_state != last_buttons_state && buttons_state > 0) {
    last_buttons_state = buttons_state;
    buttons_state = 0;
    buttons_state |= !disp_button_state(BUTTON_A);
    buttons_state |= !disp_button_state(BUTTON_B) << 1;
    buttons_state |= !disp_button_state(BUTTON_C) << 2;
    buttons_state |= !rot_encoder.digitalRead(ROT_ENCOD_SWITCH) << 3;
    buttons_state &= last_buttons_state;
  }
  return buttons_state;
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}