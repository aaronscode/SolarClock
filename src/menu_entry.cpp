#include "menu_entry.h"
MenuEntry::MenuEntry() {
  entry.tag = EMPTY_MENU_ITEM;
}

MenuEntry::MenuEntry(const char *text) {
  entry.tag = TEXT_ONLY;
  strcpy(entry.text, text);
}

MenuEntry::MenuEntry(const char *text, int *value_ptr) {
  entry.tag = INT_DISP;
  entry.int_disp.value_ptr = value_ptr;
  strcpy(entry.text, text);
}

MenuEntry::MenuEntry(const char *text, bool *value_ptr) {
  entry.tag = BOOL_DISP;
  entry.bool_disp.value_ptr = value_ptr;
  strcpy(entry.text, text);
}

MenuEntry::MenuEntry(const char *text, int *value_ptr, int min, int max) {
  entry.tag = INT_EDIT;
  entry.int_edit.value_ptr = value_ptr;
  entry.int_edit.min = min;
  entry.int_edit.max = max;
  strcpy(entry.text, text);
}
MenuEntry::MenuEntry(const char *text, bool *value_ptr, int min, int max) {
  entry.tag = BOOL_EDIT;
  entry.bool_disp.value_ptr = value_ptr;
  strcpy(entry.text, text);
}
//MenuEntry::MenuEntry(const char *text, char *value, uint32_ size) {}
MenuEntry::MenuEntry(const char *text, Menu& submenu) {}

void MenuEntry::DrawEntry(Adafruit_SH1107& oled, bool selected){
  switch(entry.tag){
    case TEXT_ONLY:
      if(selected) {
        oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
        oled.print(entry.text);
        oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
      }
      else {
        oled.print(entry.text);
      }
      break;
    case INT_DISP:
      if(selected) {
        oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
        oled.print(entry.text); oled.print(": "); oled.print(*entry.int_disp.value_ptr);
        oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
      }
      else {
        oled.print(entry.text); oled.print(": "); oled.print(*entry.int_disp.value_ptr);
      }
      break;
    case INT_EDIT:
      break;
    case BOOL_DISP:
    case BOOL_EDIT:
      if(selected) {
        oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
        oled.print(entry.text);  
        if(*entry.bool_disp.value_ptr) {oled.print(": On");}
        else {oled.print(": Off");}
        oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
      }
      else {
        oled.print(entry.text);  
        if(*entry.bool_disp.value_ptr) {oled.print(": On");}
        else {oled.print(": Off");}
      }

      break;
    default:
      break;
  }

}

int MenuEntry::getTag() {
  return entry.tag;
}

Menu* MenuEntry::getSubMenu() {
  return entry.submenu_ptr;
}

bool MenuEntry::hasFocus() {
  if(entry.tag == INT_EDIT) {
    return entry.int_edit.focus;
  } else return false;
}

void MenuEntry::update(int32_t encoder_dir, bool encoder_button) {
  switch(entry.tag){
    case INT_EDIT:
      break;
    case BOOL_EDIT:
      if(encoder_button) {
        bool *value_ptr = entry.bool_disp.value_ptr;
        *value_ptr = !(*value_ptr);
      }
      break;
    default:
      break;
  }
}