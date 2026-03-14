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

#define TFT_BL 21 // backlight

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
const char* CO2_plot_title = "CO2 Concentration (ppm)";
const char* time_scale_text = "Time Scale";

bool is_ambient = true; // default value is to record as an ambient, if false, mode is session

static lv_obj_t* start_screen;
static lv_obj_t* recording_screen;

static lv_obj_t * ta1; // text area for the building input
static lv_obj_t * ta2; // text area for the room input

#define MAX_INPUT_TEXT_LENGTH 50
char building_text[MAX_INPUT_TEXT_LENGTH];
char room_text[MAX_INPUT_TEXT_LENGTH];

static lv_obj_t * chart;
static lv_chart_series_t * ser;
static lv_chart_cursor_t * cursor;

// the width of the x axis, in seconds 
static short int time_scale = 60; 

// determines how many point will be displayed at once,
// calculated by dividing the time scale by the log rate
static short int number_plot_points; 
const short int CO2_plot_rate = 1000; // ms
static short int CO2_min = 400; // ppm
static short int CO2_max = 5000; // ppm

static lv_style_t style_radio;
static lv_style_t style_radio_chk;

const char* time_scales[] = {
  "60 s",
  "5 m",
  "1 hr",
  "24 hr",
  NULL
};

const int screen_sleep_after_time = 60000; // ms

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
    
    // turn on backlight to make screen visable
    digitalWrite(TFT_BL, HIGH);

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

// Called when the user clicks on a points on the chart
static void value_changed_event_cb(lv_event_t * e)
{
  uint32_t last_id;
  lv_obj_t * obj = lv_event_get_target_obj(e);

  last_id = lv_chart_get_pressed_point(obj);

  if(last_id != LV_CHART_POINT_NONE) {

      lv_chart_set_cursor_point(obj, cursor, NULL, last_id);

      /* Get the value from the series */
      int32_t value = ser->y_points[last_id];

      Serial.print("Point index: ");
      Serial.print(last_id);
      Serial.print("  CO2 value: ");
      Serial.println(value);
  }
}

// Creates the chart for CO2 values
void lv_chart(lv_obj_t* screen)
{
  chart = lv_chart_create(screen);
  lv_obj_set_size(chart, 200, 140); // 200x140 pixels
  lv_obj_align(chart, LV_ALIGN_CENTER, 60, -30);

  lv_obj_add_event_cb(chart, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_refresh_ext_draw_size(chart);

  cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), (lv_dir_t)(LV_DIR_LEFT | LV_DIR_BOTTOM));

  ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, CO2_min, CO2_max);
  lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, time_scale);
  lv_chart_set_point_count(chart, number_plot_points);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_div_line_count(chart, 5, 5); // (...,5 horizontal lines, 5 vertical lines)

  // Add 60 random values to the plot
  uint32_t i;
  for(i = 0; i < 60; i++) {
      lv_chart_set_next_value(chart, ser, (int32_t)lv_rand(CO2_min, CO2_max));
  }

  // Text for the plot title
  lv_obj_t * label = lv_label_create(screen);
  lv_label_set_text(label, CO2_plot_title);
  lv_obj_align_to(label, chart, LV_ALIGN_OUT_TOP_MID, -2, 0);

  // Text for the top (max) CO2 axis label
  lv_obj_t * y_label_top = lv_label_create(screen);
  lv_label_set_text_fmt(y_label_top, "%d", CO2_max);
  lv_obj_set_style_text_font(y_label_top, &lv_font_montserrat_10, 0);
  lv_obj_align_to(y_label_top, chart, LV_ALIGN_OUT_LEFT_TOP, 0, 5);

  // Text for between the top and middle CO2 axis label 
  lv_obj_t * y_label_1 = lv_label_create(screen);
  lv_label_set_text_fmt(y_label_1, "%d", CO2_min + (CO2_max - CO2_min)*3/4);
  lv_obj_set_style_text_font(y_label_1, &lv_font_montserrat_10, 0);
  lv_obj_align_to(y_label_1, chart, LV_ALIGN_OUT_LEFT_TOP, 0, lv_obj_get_height(chart)/4);

  // Text for the middle CO2 axis label
  lv_obj_t * y_label_mid = lv_label_create(screen);
  lv_label_set_text_fmt(y_label_mid, "%d", (CO2_min + CO2_max)/2);
  lv_obj_set_style_text_font(y_label_mid, &lv_font_montserrat_10, 0);
  lv_obj_align_to(y_label_mid, chart, LV_ALIGN_OUT_LEFT_MID, 0, 0);

  // Text for between the middle and bottom CO2 axis label 
  lv_obj_t * y_label_3 = lv_label_create(screen);
  lv_label_set_text_fmt(y_label_3, "%d", CO2_min + (CO2_max - CO2_min)*1/4);
  lv_obj_set_style_text_font(y_label_3, &lv_font_montserrat_10, 0);
  lv_obj_align_to(y_label_3, chart, LV_ALIGN_OUT_LEFT_BOTTOM, 0, -lv_obj_get_height(chart)/4);

  // Text for the bottom (min) CO2 axis label
  lv_obj_t * y_label_bottom = lv_label_create(screen);
  lv_label_set_text_fmt(y_label_bottom, "%d", CO2_min);
  lv_obj_set_style_text_font(y_label_bottom, &lv_font_montserrat_10, 0);
  lv_obj_align_to(y_label_bottom, chart, LV_ALIGN_OUT_LEFT_BOTTOM, 0, -5);

}

// Called when the user selects a radio button option
static void event_radio_button(lv_event_t * e)
{
  lv_obj_t * obj = lv_event_get_target_obj(e);

  // check that the checkbox is checked (radio button is selected)
  // Since this event is called twice for the unchecking of the previous
  // option and the checking of a new option
  if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
      Serial.printf("%s is selected.\n", lv_checkbox_get_text(obj));
  }
  else {
      Serial.printf("%s is not selected.\n", lv_checkbox_get_text(obj));
  }
}

// Creates checkboxes as radio buttons for the time scale selection
void lv_radio_buttons(lv_obj_t* screen)
{
  lv_style_init(&style_radio);
  lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
  lv_style_init(&style_radio_chk);
  lv_style_set_bg_image_src(&style_radio_chk, NULL);

  lv_obj_t * cont = lv_obj_create(screen);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_align(cont, LV_ALIGN_CENTER, -115, -30);

  // Create the selection options from the time_scales list of strings
  uint32_t i;
  char buf[32];
  for(int i = 0; time_scales[i] != NULL; i++) {

    lv_snprintf(buf, sizeof(buf), time_scales[i]);

    lv_obj_t * obj = lv_checkbox_create(cont);
    lv_checkbox_set_text(obj, buf);

    lv_obj_add_event_cb(obj, event_radio_button, LV_EVENT_VALUE_CHANGED, NULL);

    // This makes the checkboxes act as radio buttons
    lv_obj_set_radio_button(obj, true);

    lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
  }

  // Make the first checkbox checked
  lv_obj_add_state(lv_obj_get_child(cont, 0), LV_STATE_CHECKED);

  lv_obj_t * label = lv_label_create(screen);
  lv_label_set_text(label, time_scale_text);
  lv_obj_align_to(label, cont, LV_ALIGN_OUT_TOP_MID, -2, 0);
}

// Creates the GUI elements for the start and recording screens
void lv_create_main_gui(void) {

  number_plot_points = time_scale/(CO2_plot_rate/1000);

  // initialize the two screens
  start_screen = lv_obj_create(NULL);
  recording_screen = lv_obj_create(NULL);

  // call functions to create start screen
  lv_top_text(start_screen);
  lv_keyboard(start_screen);
  lv_switch(start_screen);
  lv_button(start_screen);

  // call functions to create recording screen
  lv_chart(recording_screen);
  lv_radio_buttons(recording_screen);

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

  // Turns the backlight off after some time of inactivity
  if(lv_disp_get_inactive_time(NULL) > screen_sleep_after_time) {
    digitalWrite(TFT_BL, LOW);
  }

}