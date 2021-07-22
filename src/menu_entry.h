#ifndef MENU_ENTRY
#define MENU_ENTRY
#include <Arduino.h>
#include <Adafruit_SH110X.h> // OLED screen

#define OLED_TEXTCOLS 21

#define EMPTY_MENU_ITEM -1
#define TEXT_ONLY 0
#define INT_DISP 1
#define BOOL_DISP 2 
#define INT_EDIT 3
#define BOOL_EDIT 4
#define SUBMENU 5

class Menu;

typedef struct {
  int tag;
  char text[21];
  union {
    struct {int *value_ptr;} int_disp;
    struct {bool *value_ptr;} bool_disp;
    struct {int *value_ptr; int min; int max; bool focus;} int_edit;
    Menu *submenu_ptr;
  };
} Entry;

class MenuEntry {
  private:
    Entry entry; 
  public:
    MenuEntry();
    MenuEntry(const char *text);
    MenuEntry(const char *text, int *value_ptr);
    MenuEntry(const char *text, bool *value_ptr);
    MenuEntry(const char *text, int *value_ptr, int min, int max);
    MenuEntry(const char *text, bool *value_ptr, int min, int max);
    //MenuEntry(const char *text, char *value, uint32_ size);
    MenuEntry(const char *text, Menu& submenu);
    void DrawEntry(Adafruit_SH1107& oled, bool selected);
    int getTag();
    Menu* getSubMenu();
    bool hasFocus();
    void update(int32_t encoder_dir, bool encoder_button);
};

#endif