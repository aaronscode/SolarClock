#include "menu.h"


Menu::Menu(bool top_level) : cursor_pos(0), top_menu_item(0), top_level(top_level) {
  current_menu_count = 0;
}

Menu::Menu(bool top_level, MenuEntry *entries, uint16_t count) : cursor_pos(0), top_menu_item(0), top_level(top_level) {
  current_menu_count = count;
  //menu_entries = (MenuEntry*) malloc(sizeof(MenuEntry) * current_menu_count);
  for(int i = 0; i < current_menu_count; i++) {
    menu_entries[i] = entries[i];
  }
}

Menu::~Menu() {
}

void Menu::AddEntry(MenuEntry entry) {
}

void Menu::DrawMenu(Adafruit_SH1107& oled) {
  oled.clearDisplay();
  oled.setCursor(0,0);
  for(int i = top_menu_item; i < min(current_menu_count, top_menu_item + OLED_TEXTROWS); i++) {
    menu_entries[i].DrawEntry(oled, i==cursor_pos);
    oled.println();
  }

  // back arrow for sub-menus
  if(!top_level && cursor_pos >= current_menu_count - OLED_TEXTROWS) {
    if(cursor_pos == current_menu_count) { // if back arrow currently hilighted
      int x = oled.getCursorX();
      int y = oled.getCursorY();
      oled.fillRect(x, y, 7, 7, SH110X_WHITE);
      oled.drawBitmap(x, y, epd_bitmap_BackArrow, 5, 7, SH110X_BLACK);
    } else {
      oled.drawBitmap(oled.getCursorX(), oled.getCursorY(), epd_bitmap_BackArrow, 5, 7, SH110X_WHITE);
    }


  }
  oled.setCursor(100, 0);
  oled.print(cursor_pos);
  oled.setCursor(100, 10);
  oled.print(top_menu_item);
  oled.setCursor(100, 20);
  oled.print(menu_entries[cursor_pos].getTag());
}

Menu* Menu::UpdateMenu(int32_t encoder_dir, bool encoder_button) {
  if(encoder_button){
    if(!top_level && cursor_pos == current_menu_count) {
      // reset menu
      cursor_pos = 0;
      top_menu_item = 0;

      return previous_menu;
    }
    else if (menu_entries[cursor_pos].getTag() == SUBMENU) {
      return menu_entries[cursor_pos].getSubMenu(this);
    }
    else {
      menu_entries[cursor_pos].update(encoder_dir, encoder_button);
    }
  } else if(!(cursor_pos == current_menu_count) && menu_entries[cursor_pos].hasFocus()) {
      menu_entries[cursor_pos].update(encoder_dir, encoder_button);
  } else {
    cursor_pos += encoder_dir;
    if(!top_level) {
      cursor_pos = constrain(cursor_pos, 0, current_menu_count);
      if(cursor_pos < top_menu_item) {
        top_menu_item = cursor_pos;
      } else if (cursor_pos >= (top_menu_item + OLED_TEXTROWS)) {
        top_menu_item = cursor_pos - OLED_TEXTROWS + 1;
      }
    } else {
      cursor_pos = constrain(cursor_pos, 0, current_menu_count - 1);
      if(cursor_pos < top_menu_item) {
        top_menu_item = cursor_pos;
      } else if (cursor_pos >= (top_menu_item + OLED_TEXTROWS)) {
        top_menu_item = cursor_pos - OLED_TEXTROWS + 1;
      }

    }
  }
  
  return nullptr;
}


void Menu::setPreviousMenu(Menu *prev) {
  previous_menu = prev;
}