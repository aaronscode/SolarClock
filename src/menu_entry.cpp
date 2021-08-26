#include "menu_entry.h"
#include "menu.h"

MenuEntry::MenuEntry() {
  entry.tag = EMPTY_MENU_ITEM;
}

MenuEntry::MenuEntry(const char *text) {
  entry.tag = TEXT_ONLY;
  strcpy(entry.text, text);
}

MenuEntry::MenuEntry(const char *text, int *value_ptr) {
  entry.tag = INT_DISP;
  strcpy(entry.text, text);
  entry.int_disp.value_ptr = value_ptr;
}

MenuEntry::MenuEntry(const char *text, bool *value_ptr) {
  entry.tag = BOOL_DISP;
  strcpy(entry.text, text);
  entry.bool_disp.value_ptr = value_ptr;
}

MenuEntry::MenuEntry(const char *text, int *value_ptr, int min, int max) {
  entry.tag = INT_EDIT;
  strcpy(entry.text, text);
  entry.int_edit.value_ptr = value_ptr;
  entry.int_edit.focus = false;
  entry.int_edit.min = min;
  entry.int_edit.max = max;
}
MenuEntry::MenuEntry(const char *text, bool *value_ptr, int min, int max) {
  entry.tag = BOOL_EDIT;
  entry.bool_disp.value_ptr = value_ptr;
  strcpy(entry.text, text);
}
//MenuEntry::MenuEntry(const char *text, char *value, uint32_ size) {}
MenuEntry::MenuEntry(const char *text, Menu *submenu) {
  entry.tag = SUBMENU;
  entry.submenu_ptr = submenu;
  strcpy(entry.text, text);
}

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
      {
        int disp_value = *(entry.int_edit.value_ptr);
        int disp_min = entry.int_edit.min;
        int disp_max = entry.int_edit.max;
        float percent_full = float(disp_value - disp_min) / float(disp_max-disp_min);
        if(selected) {
          if(entry.int_edit.focus) {
            oled.print(entry.text); oled.print(":"); 
            int x = oled.getCursorX();
            int y = oled.getCursorY();
            oled.fillRect(x, y, (127-x), 8, SH110X_WHITE);
            oled.drawRect(x+1, y+1, 127-x-2, 6, SH110X_BLACK);
            oled.fillRect(x+2, y+2, (127-x-3)*percent_full, 4, SH110X_BLACK);
          } else {
            oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
            oled.print(entry.text); oled.print(":"); 
            oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
            //oled.print(*(entry.int_disp.value_ptr));
            int x = oled.getCursorX();
            int y = oled.getCursorY();
            oled.fillRect(x, y, (127-x), 8, SH110X_WHITE);
            oled.drawRect(x+1, y+1, 127-x-2, 6, SH110X_BLACK);
            oled.fillRect(x+2, y+2, (127-x-3)*percent_full, 4, SH110X_BLACK);

          }
        }
        else {
          oled.print(entry.text); oled.print(":"); 
          int x = oled.getCursorX();
          int y = oled.getCursorY();
          
          oled.drawRect(x, y+1, 127-x, 6, SH110X_WHITE);
          oled.fillRect(x+1, y+2, (127-x-2)*percent_full, 4, SH110X_WHITE);

          //oled.print(*entry.int_disp.value_ptr);
        }
        break;
      }
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
      case SUBMENU:
        if(selected) {
          oled.setTextColor(SH110X_BLACK, SH110X_WHITE);
          oled.print(entry.text);
          oled.setTextColor(SH110X_WHITE, SH110X_BLACK);
          int x = oled.getCursorX() + 2;
          int y = oled.getCursorY();
          oled.fillRect(x-2, y, 7, 8, SH110X_WHITE);
          oled.drawBitmap(x, y, epd_bitmap_ForwardArrow, 5, 7, SH110X_BLACK);
        }
        else {
          oled.print(entry.text);
          int x = oled.getCursorX() + 2;
          int y = oled.getCursorY();
          oled.fillRect(x-2, y, 7, 8, SH110X_BLACK);
          oled.drawBitmap(x, y, epd_bitmap_ForwardArrow, 5, 7, SH110X_WHITE);
        }
        break;
    default:
      break;
  }

}

int MenuEntry::getTag() {
  return entry.tag;
}

Menu* MenuEntry::getSubMenu(Menu* previous_menu) {
  Menu *submenu_ptr = entry.submenu_ptr;
  submenu_ptr->setPreviousMenu(previous_menu);
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
      if(encoder_button) {
        entry.int_edit.focus= !entry.int_edit.focus;
      }
      else {
        int *val = entry.int_edit.value_ptr;
        *val = constrain(*val + (5 * encoder_dir), entry.int_edit.min, entry.int_edit.max);
      }
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