// Configuration files and base code from: https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r
// LVGL documentation: https://docs.lvgl.io/

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

const char* top_text = "Air Quality Monitor";
const char* enter_building_text = "Building";
const char* enter_room_number_text = "Room Number";
const char* ambient_text = "Ambient";
const char* session_text = "Session";

bool is_ambient = true; // default value is to record as an ambient, if false, mode is session

lv_obj_t* start_screen;
lv_obj_t* recording_screen;

lv_obj_t * ta1; // text area for the building input
lv_obj_t * ta2; // text area for the room input

#define MAX_INPUT_TEXT_LENGTH 50
char building_text[MAX_INPUT_TEXT_LENGTH];
char room_text[MAX_INPUT_TEXT_LENGTH];

// Gets the building and room number text from the text areas, checks that the they are valid
bool get_location() {
  const char* building_text_const = lv_textarea_get_text(ta1); 
  const char* room_text_const = lv_textarea_get_text(ta2); 

  if (strlen(building_text_const) >= MAX_INPUT_TEXT_LENGTH) {
    Serial.println("Error: Building name too long!");
    return false;
  }
  if (strlen(room_text_const) >= MAX_INPUT_TEXT_LENGTH) {
    Serial.println("Error: Room number too long!");
    return false;
  }

  strncpy(building_text, building_text_const, MAX_INPUT_TEXT_LENGTH - 1);
  building_text[MAX_INPUT_TEXT_LENGTH - 1] = '\0';
  strncpy(room_text, room_text_const, MAX_INPUT_TEXT_LENGTH - 1);
  room_text[MAX_INPUT_TEXT_LENGTH - 1] = '\0';
  return true;
}


// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {

  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {

    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target_obj(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

// Creates text centered at the top of the screen
void lv_top_text(lv_obj_t* screen) {

  lv_obj_t * text_label = lv_label_create(screen);
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);    
  lv_label_set_text(text_label, top_text);
  lv_obj_set_width(text_label, 150);    // width of the text (wraps if text is longer than the input number)
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -105);
}

// Creates the keyboard when a text area is selected
// Allows the user to enter a building and room number
void lv_keyboard(lv_obj_t* screen)
{
    // Create a keyboard, automatically displays when a text box is clicked on
    lv_obj_t * kb = lv_keyboard_create(screen);

    // Enter Building Text (Above the text box)
    lv_obj_t * building_text_label = lv_label_create(screen);
    lv_label_set_long_mode(building_text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
    lv_label_set_text(building_text_label, enter_building_text);
    lv_obj_set_width(building_text_label, 150);    // Set smaller width to make the lines wrap
    lv_obj_set_style_text_align(building_text_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(building_text_label, LV_ALIGN_TOP_LEFT, 45, 30);

    // text area for the building entry. The keyboard will write here
    ta1 = lv_textarea_create(screen);
    lv_obj_align(ta1, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_obj_add_event_cb(ta1, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta1, "COE");
    lv_obj_set_size(ta1, 140, 35);
    
    // Room Number Text (Above the text box)
    lv_obj_t * room_number_text_label = lv_label_create(screen);
    lv_label_set_long_mode(room_number_text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
    lv_label_set_text(room_number_text_label, enter_room_number_text);
    lv_obj_set_width(room_number_text_label, 200);    // Set smaller width to make the lines wrap
    lv_obj_set_style_text_align(room_number_text_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(room_number_text_label, LV_ALIGN_TOP_LEFT, 185, 30);

    // text area for the room number, the keyboard will write here
    ta2 = lv_textarea_create(screen);
    lv_obj_align(ta2, LV_ALIGN_TOP_RIGHT, -10, 50);
    lv_obj_add_event_cb(ta2, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta2, "306");
    lv_obj_set_size(ta2, 140, 35);

    lv_keyboard_set_textarea(kb, ta1);

}

// Called when the Ambient/Session switch is toggled,
// used for specifying the mode
static void switch_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target_obj(e); 
    LV_UNUSED(obj);
    if(code == LV_EVENT_VALUE_CHANGED) {

        // if its set to "ON", its set to a session, otherwise its set to ambient   
        is_ambient = !(lv_obj_has_state(obj, LV_STATE_CHECKED)); 
    }
}

// Creates the ambient/session toggle switch
// located on the left, below the text areas
void lv_switch(lv_obj_t* screen)
{

  // Ambient Text (to the left of the switch)
  lv_obj_t * ambient_text_label = lv_label_create(screen);
  lv_label_set_long_mode(ambient_text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(ambient_text_label, ambient_text);
  lv_obj_set_width(ambient_text_label, 200);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(ambient_text_label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(ambient_text_label, LV_ALIGN_TOP_LEFT, 10, 95);

  // Session Text (to the right of the switch)
  lv_obj_t * session_text_label = lv_label_create(screen);
  lv_label_set_long_mode(session_text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(session_text_label, session_text);
  lv_obj_set_width(session_text_label, 200);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(session_text_label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_align(session_text_label, LV_ALIGN_TOP_LEFT, 135, 95);

  // toggle switch (centered between texts)
  lv_obj_t * sw;
  sw = lv_switch_create(screen);
  lv_obj_align(sw, LV_ALIGN_TOP_LEFT, 80, 90);
  lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_flag(sw, LV_OBJ_FLAG_EVENT_BUBBLE);

}

// Called when the Start button is pressed
static void button_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {

      // if the location text is valid, advance to the recording screen
      if (!get_location()) return;
      lv_scr_load(recording_screen);
    }
}

// Creates the start button
void lv_button(lv_obj_t* screen)
{

    // Button, located to the right of toggle switch
    lv_obj_t * label;
    lv_obj_t * btn1 = lv_button_create(screen);
    lv_obj_add_event_cb(btn1, button_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 245, 85);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    
    // Text inside the button
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Start");
    lv_obj_center(label);

}

void lv_create_main_gui(void) {

  // initialize the two screens
  start_screen = lv_obj_create(NULL);
  recording_screen = lv_obj_create(NULL);

  // call functions to create start screen
  lv_top_text(start_screen);
  lv_keyboard(start_screen);
  lv_switch(start_screen);
  lv_button(start_screen);

  // set the start screen as the active scren
  lv_scr_load(start_screen);
}

void setup() {
  Serial.begin(115200);
  
  // Start LVGL
  lv_init();

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t * disp;

  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);

  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Function to draw the GUI (text, buttons and sliders)
  lv_create_main_gui();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}