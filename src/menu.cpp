#include "menu.h"


Menu::Menu(bool top_level) : cursor_pos(0), top_menu_item(0), top_level(top_level) {
  current_menu_count = 0;
}

Menu::Menu(bool top_level, MenuEntry *entries, uint16_t count) : cursor_pos(0), top_menu_item(0), top_level(top_level) {
  current_menu_count = count;
  for(int i = 0; i < current_menu_count; i++) {
    menu_entries[i] = entries[i];
  }
}

void Menu::AddEntry(MenuEntry entry) {
  menu_entries[current_menu_count] = entry;
  current_menu_count++;
}

void Menu::DrawMenu(Adafruit_SH1107& oled) {
  oled.clearDisplay();
  oled.setCursor(0,0);
  for(int i = top_menu_item; i < min(current_menu_count, top_menu_item + OLED_TEXTROWS); i++) {
    menu_entries[i].DrawEntry(oled, i==cursor_pos);
    oled.println();
  }
  oled.setCursor(100, 0);
  oled.print(cursor_pos);
  oled.setCursor(100, 10);
  oled.print(top_menu_item);
  /*
  for(int i = 0; i < 4; i++) {
    if(i == cursor_pos) {
      if(!cursor_indent) {
        oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
      oled.print(color_chars[i]); 
      oled.print(": "); 
      oled.println(color_vals[i]);
      oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
    } else {
      oled.print(color_chars[i]); 
      oled.print(": "); 
      oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
      oled.println(color_vals[i]);
      oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
    }
  } else {
    oled.print(color_chars[i]); 
    oled.print(": "); 
    oled.println(color_vals[i]);
  }
}
*/
}

Menu* Menu::UpdateMenu(int32_t encoder_dir, bool encoder_button) {
  if(encoder_button){
    if(!top_level && cursor_pos == current_menu_count) {
      return previous_menu;
    }
    else if (menu_entries[cursor_pos].getTag() == SUBMENU) {
      return menu_entries[cursor_pos].getSubMenu();
    }
    else {
      menu_entries[cursor_pos].update(encoder_dir, encoder_button);
    }
  } else if(!(cursor_pos == current_menu_count) && menu_entries[cursor_pos].hasFocus()) {
      menu_entries[cursor_pos].update(encoder_dir, encoder_button);
  } else {
    cursor_pos += encoder_dir;
    cursor_pos = constrain(cursor_pos, 0, current_menu_count - 1);
    if(cursor_pos < top_menu_item) {
      top_menu_item = cursor_pos;
    } else if (cursor_pos >= (top_menu_item + OLED_TEXTROWS)) {
      top_menu_item = cursor_pos - OLED_TEXTROWS + 1;
    }
  }
  
  return nullptr;
}

