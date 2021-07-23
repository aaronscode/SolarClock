#ifndef CUSTOM_MENU
#define CUSTOM_MENU

#include <Arduino.h>
#include <Adafruit_SH110X.h> // OLED screen
#include "menu_entry.h"
#include "bitmap.h"

#define OLED_TEXTCOLS 21
#define OLED_TEXTROWS 8

class Menu {
  private:
    int16_t cursor_pos;
    uint16_t current_menu_count;
    uint16_t top_menu_item;
    bool top_level;
    MenuEntry menu_entries[20];
    Menu *previous_menu;
  public:
    Menu(bool top_level);
    Menu(bool top_level, MenuEntry *entries, uint16_t count);
    ~Menu();
    void AddEntry(MenuEntry entry);
    void DrawMenu(Adafruit_SH1107& oled); 
    Menu* UpdateMenu(int32_t encoder_dir, bool encoder_button);
    void setPreviousMenu(Menu *prev);
};

#endif