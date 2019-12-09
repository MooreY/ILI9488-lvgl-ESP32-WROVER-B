#include <Arduino.h>
#include <esp_freertos_hooks.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <lvgl/lvgl.h>


#include "drv/disp_spi.h"
#include "drv/ili9488.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"
//#include <tpcal/tpcal.h>

#include <components/mqtt.h>

#include <gui/ui.h>
#include <gui/ui_theme.h>

#include "resource-id.h"

#define LVGL_TICK_PERIOD 20

#define USE_MQTT

/*********************
 *      DEFINES
 *********************/

static const int TERMINAL_LOG_LENGTH = 100;

using std::string;
// using json11::Json;

/**********************
 *      STYLES
 **********************/
static lv_style_t style_terminal;
static lv_style_t red_led;
static lv_style_t green_led;
static lv_style_t yellow_led;
static lv_style_t blue_led;
static lv_style_t panel_bg;

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
	ui_object_id id;
	const char* topic;
} mqtt_ui_object_t;

typedef int (*update_cb_t)(uint32_t);

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static std::string device_name = "controlpanel";

static UI ui;

static ui_tab_item_t tabview_items[] = {
    {ID_TAB1, "Woonkamer"},
    {ID_TAB2, "MeterKast"},
    {ID_TAB3, "TrapKast"},
    {ID_TAB4, "Input"},
    {ID_TAB_TERMINAL, LV_SYMBOL_DOWNLOAD},
};
static const int NUM_TAB_ITEMS = 5;

// static mqtt_ui_object_t ui_subscriber_list[] = {
//     {ID_LED_BACKLIGHT, "led/light"},
//     {ID_SWITCH_LEDS, "tft/switch"},
// };
// static const int NUM_SUBSCRIBERS = 2;

// static mqtt_ui_object_t ui_publisher_list[] = {{ID_SLIDER_LEDS, "slider/leds"},
//                                                {ID_SWITCH_LEDS, "switch/leds"},
//                                                {ID_SWITCH_GANG, "switch/gang"},
// 											   {ID_CB_BACKLIGHT, "ceckbox/backlight"},
// 											   {ID_CB_SCREENSAVER, "checkbox/screensaver"}};
// static const int NUM_PUBLISHERS = 5;

static ui_dropdown_specs *dropdown_spec = new ui_dropdown_specs({"Apple\n"
                                                                 "Banana\n"
                                                                 "Orange\n"
                                                                 "Melon\n"
                                                                 "Grape\n"
                                                                 "Raspberry"},
                                                                true);
static ui_roller_specs *roller_specs =
    new ui_roller_specs({"January\nFebruary\nMarch\nApril\nMay\nJune\nJuly\nAug"
                         "ust\nSeptember\nOctober\nNovember\nDecember"},
                        5, true);

std::vector<UI_Object_C *> ui_time_objects;

/**********************
 *      EVENT HANDLERS
 **********************/
#ifdef USE_MQTT
void mqtt_cb(const std::string& topic, const std::string& payload) {
	printf("MQTT CB %s, payload %s\n", topic.c_str(), payload.c_str());
	ui.label_add_text(ID_TERMINAL, topic.c_str(), TERMINAL_LOG_LENGTH);
	if (topic.compare("tft/switch") == 0) {
		bool state = payload.compare("ON");
		ui.set_value(ID_SWITCH_LEDS, state);
	}
	// else if (topic.compare("led/light") == 0) {
	// 	std::string err;

	// 	Json json_payl = Json::parse(payload, err);
	// 	int value = json_payl["brightness"].int_value();
	// 	ui.set_value(ID_BAR_VALUE, 1, value);
	// }
}

#endif

bool publish(UI_Object_C* ui_obj) {
	auto value = ui.get_value(ui_obj);
	if (ui_obj->has_name()) {
		std::string topic = device_name + "/" + ui.get_type_name(ui_obj->id) + "/" + ui_obj->name + "/state";
		mqtt_publish(topic, value.char_value);
		return true;
	}
	return false;
}

bool publish(ui_object_id id) {
	UI_Object_C* ui_obj = ui.find_object_by_id(id);
	if (ui_obj != nullptr)
		return publish(ui_obj);
	return false;
}

void ui_handler(UI_Object_C* ui_obj, ui_event_t event) {
	auto value = ui.get_value(ui_obj);
  	if (event == ui_event_t::VALUE_CHANGED || event == ui_event_t::APPLY)
		publish(ui_obj);
	switch (ui_obj->id) {
	case ID_BUTTON_CLEAR: {
		ui.set_value(ID_TERMINAL, "");
		break;
	}
	case ID_TEXTFIELD_SEARCH: {
		if (event == ui_event_t::APPLY) {
			ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		}
		break;
	}
	}
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

//static const char *TAG_MAIN = "MAIN";

static void IRAM_ATTR lv_tick_task(void) { lv_tick_inc(portTICK_RATE_MS); }

void create_gui() {
	ui.add_tabview(ID_NONE, ID_TAB_VIEW, tabview_items, NUM_TAB_ITEMS, LV_HOR_RES, LV_VER_RES);

	ui.add_label(ID_TAB1, ID_LABEL_LEDS, "leds achter de bank", LV_ALIGN_IN_TOP_LEFT, ID_TAB1, 5, 15);
	ui.add_switch(ID_TAB1, ID_SWITCH_LEDS, "leds", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_LEDS, 10, 10);

	ui.add_label(ID_TAB1, ID_LABEL_GANG, "kleur in gang", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_LEDS, -10, 15);
	ui.add_switch(ID_TAB1, ID_SWITCH_GANG, "gang", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_GANG, 10, 10);

  ui.add_label(ID_TAB1, ID_LABEL_BLAMP, "groene lampje", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANG, -10, 15);
	ui.add_switch(ID_TAB1, ID_SWITCH_BLAMP, "blamp", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_BLAMP, 10, 10);

	ui_object_t* slider = ui.add_slider(ID_TAB1, ID_SLIDER_LEDS, "ledvalue", 0, ui_handler, LV_ALIGN_OUT_RIGHT_MID, ID_LABEL_LEDS, 40, 0);
	ui.add_label(ID_TAB1, ID_LABEL_TIME, "10:00", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SLIDER_LEDS, 5, 10);

  lv_slider_set_range(slider->lv_obj, 0, 255);


	ui.add_label(ID_TAB2, ID_LABEL_HOEDENPLANK, "hoedenplank", LV_ALIGN_IN_TOP_LEFT, ID_TAB2, 5, 15);
	slider = ui.add_slider(ID_TAB2, ID_SLIDER_HOEDENPLANK, "hoedenplank", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_HOEDENPLANK, 10, 10);
  lv_slider_set_range(slider->lv_obj, 0, 255);
  
	ui.add_label(ID_TAB2, ID_LABEL_GANGLAMP, "gang lampje", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SLIDER_HOEDENPLANK, -10, 15);
	ui.add_switch(ID_TAB2, ID_SWITCH_GANGLAMP, "ganglamp", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_GANGLAMP, 10, 10);

  ui.add_label(ID_TAB2, ID_LABEL_SCHUUR, "schuur", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANGLAMP, -10, 15);
	ui.add_switch(ID_TAB2, ID_SWITCH_SCHUUR, "schuur", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LABEL_SCHUUR, 10, 10);

	// ui.add_roller(ID_TAB1, ID_ROLLER, "roller", roller_specs, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANG, 0, 60);
	// ui.add_dropdown(ID_TAB1, ID_DROPDOWN, "dropdown", dropdown_spec, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANG, 0, 10);
	// ui.set_size(ID_DROPDOWN, 130, 0);
	// ui.set_size(ID_ROLLER, ID_DROPDOWN);

	// ui.add_list(ID_TAB2, ID_LIST_SYMBOLS, "list1_", list_items, NUM_LIST_ITEMS, list_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB2, 5, 5);
	// ui.set_size(ID_LIST_SYMBOLS, 150, 200);
	// ui.add_list(ID_TAB2, ID_LIST2_SYMBOLS2, "list2_", list_items2, NUM_LIST_ITEMS2, list_handler, LV_ALIGN_OUT_RIGHT_TOP, ID_LIST_SYMBOLS, 10, 0);
	// ui.set_size(ID_LIST2_SYMBOLS2, 150, 270);
	// ui.add_led(ID_TAB2, ID_LED_EXTRA2, "led_extra2", false, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LIST_SYMBOLS, 10, 10);
	// ui.set_size(ID_LED_EXTRA2, 20, 20);
	// ui.add_led(ID_TAB2, ID_LED_EXTRA3, "led_extra3", false, LV_ALIGN_OUT_RIGHT_TOP, ID_LED_EXTRA2, 20, 0);
	// ui.set_size(ID_LED_EXTRA3, ID_LED_EXTRA2);
	// ui.set_style(ID_LED_EXTRA3, &yellow_led);
	// ui.add_led(ID_TAB2, ID_LED_EXTRA4, "led_extra4", false, LV_ALIGN_OUT_RIGHT_TOP, ID_LED_EXTRA3, 20, 0);
	// ui.set_size(ID_LED_EXTRA4, ID_LED_EXTRA2);
	// ui.set_style(ID_LED_EXTRA4, &blue_led);

	// ui.add_led(ID_TAB2, ID_LED_EXTRA5, "led_extra5", false, LV_ALIGN_OUT_RIGHT_TOP, ID_LED_EXTRA4, 20, 0);
	// ui.set_size(ID_LED_EXTRA5, ID_LED_EXTRA2);
	// ui.set_style(ID_LED_EXTRA5, &green_led);

	ui.add_label(ID_TAB_TERMINAL, ID_TERMINAL, "BYE");
	ui.set_style(ID_TERMINAL, &style_terminal, 0);
	ui.add_button(ID_TAB_TERMINAL, ID_BUTTON_CLEAR, ID_BUTTON_CLEAR_LABEL, "Clear", "clearbutton", ui_handler, LV_ALIGN_IN_BOTTOM_RIGHT, ID_TAB_TERMINAL, -10, 10);
	ui.set_floating(ID_BUTTON_CLEAR, ID_TAB_TERMINAL);
	ui.set_size(ID_BUTTON_CLEAR, 100, 40);
	ui.add_textarea(ID_TAB_TERMINAL, ID_TEXTFIELD_SEARCH, "search", UI_KEYB_TEXT, true, false, ui_handler, LV_ALIGN_IN_TOP_RIGHT, ID_TAB_TERMINAL, 0, 0);
	ui.set_floating(ID_TEXTFIELD_SEARCH, ID_TAB_TERMINAL);

	// ui.add_checkbox(ID_TAB3, ID_CB_SCREENSAVER, "Screensaver", "screensaver", false, ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB3, 5, 10);
	// ui.add_checkbox(ID_TAB3, ID_CB_BACKLIGHT, "Backlight", "backlight", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_CB_SCREENSAVER, 0, 0);

	//add_led_panel(ID_TAB3, ID_LED_PANEL, ID_CB_BACKLIGHT);

	// ui.add_bar(ID_TAB3, ID_BAR_VALUE, "bar", 0, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LED_PANEL, 0, 20);
	// ui.set_size(ID_BAR_VALUE, 200, 20);
	//ui.set_style(ID_BAR_VALUE, &panel_bg);

	//CALENDAR
	//ui.add_calendar(ID_TAB3, ID_CALENDAR, "calendar", ui_handler, LV_ALIGN_OUT_RIGHT_TOP, ID_CB_SCREENSAVER, 100, 5);
	//ui.set_value(ID_CALENDAR, TODAY_DATE);
	//ui.set_value(ID_CALENDAR, HIGHLIGHT_DATES, 29, 11, 2019);
	//ui.set_value(ID_CALENDAR, HIGHLIGHT_DATES, 2, 12, 2019);

	//	uint16_t party[] = { 18,11,2019 };	uint16_t church[] = { 21,11,2019 };	uint16_t kingsday[] = { 22,11,2019 };
//	ui.set_value(ID_CALENDAR, 4, HIGHLIGHT_DATES, party, church, kingsday);
//	ui.add_label(ID_TAB3, ID_LABEL_DATE, "date label", LV_ALIGN_OUT_BOTTOM_LEFT, ID_CALENDAR, 10, 10);

	//SPINBOX
	// ui_spinbox_specs* spinbox_def = new ui_spinbox_specs(3, 1, 2, 0, 100, 0, 1); // digit_count, fract, left_padding,min,max,value,step
	// ui.add_spinbox(ID_TAB3, ID_SPINBOX, "spinbox", spinbox_def, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_BAR_VALUE, 0, 20);

	// ui.add_textarea(ID_TAB4, ID_TEXTAREA_INPUT, "input", UI_KEYB_TEXT, false, false, ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB4, 0, 0);
	// ui.set_size(ID_TEXTAREA_INPUT, ID_TAB4);

}

void setup_theme() {

  lv_theme_t *th = ui_theme_init( 138, &lv_font_roboto_16); // Set a HUE value and a Font for the Night Theme

  static lv_style_t style_tv_btn_bg;
  lv_style_copy(&style_tv_btn_bg, th->style.tabview.btn.bg);
  style_tv_btn_bg.body.padding.top = 0;
  style_tv_btn_bg.body.padding.bottom = 0;
  style_tv_btn_bg.body.padding.left = 0;

  static lv_style_t style_tv_btn_rel;
  lv_style_copy(&style_tv_btn_rel, th->style.tabview.btn.rel);
  style_tv_btn_rel.body.radius = 0;
  style_tv_btn_rel.body.opa = LV_OPA_50;
  style_tv_btn_rel.body.border.width = 0;

  static lv_style_t style_tv_btn_pr;
  lv_style_copy(&style_tv_btn_pr, th->style.tabview.btn.pr);
  style_tv_btn_pr.body.radius = 0;
  style_tv_btn_pr.body.opa = LV_OPA_50;
  style_tv_btn_pr.body.border.width = 0;

  static lv_style_t style_tv_indic;
  lv_style_copy(&style_tv_indic, &lv_style_plain_color);
  style_tv_indic.body.main_color = lv_color_hex(0xffffff);
  style_tv_indic.body.grad_color = lv_color_hex(0xffffff);

  th->style.tabview.indic = &style_tv_indic;
  th->style.tabview.btn.tgl_rel = &style_tv_btn_pr;
  th->style.tabview.btn.tgl_pr = &style_tv_btn_pr;
  th->style.tabview.btn.bg = &style_tv_btn_bg;
  th->style.tabview.btn.pr = &style_tv_btn_pr;
  th->style.tabview.btn.rel = &style_tv_btn_rel;

  // TERMINAL
  lv_style_copy(&style_terminal, &lv_style_plain_color);
  style_terminal.text.color = lv_color_hex(0xffffff);
  style_terminal.text.font = &lv_font_roboto_16;

  // LEDS
  lv_style_copy(&red_led, th->style.led);
  red_led.body.main_color = lv_color_hsv_to_rgb(0, 100, 100);
  red_led.body.grad_color = lv_color_hsv_to_rgb(0, 100, 40);
  red_led.body.border.color = lv_color_hsv_to_rgb(0, 60, 60);
  red_led.body.shadow.color = lv_color_hsv_to_rgb(0, 100, 100);

  lv_style_copy(&green_led, th->style.led);
  green_led.body.main_color = lv_color_hsv_to_rgb(96, 100, 100);
  green_led.body.grad_color = lv_color_hsv_to_rgb(96, 100, 40);
  green_led.body.border.color = lv_color_hsv_to_rgb(96, 60, 60);
  green_led.body.shadow.color = lv_color_hsv_to_rgb(96, 100, 100);

  lv_style_copy(&yellow_led, th->style.led);
  yellow_led.body.main_color = lv_color_hsv_to_rgb(60, 100, 100);
  yellow_led.body.grad_color = lv_color_hsv_to_rgb(60, 100, 40);
  yellow_led.body.border.color = lv_color_hsv_to_rgb(60, 60, 60);
  yellow_led.body.shadow.color = lv_color_hsv_to_rgb(60, 100, 100);

  lv_style_copy(&blue_led, th->style.led);
  blue_led.body.main_color = lv_color_hsv_to_rgb(240, 100, 100);
  blue_led.body.grad_color = lv_color_hsv_to_rgb(240, 100, 40);
  blue_led.body.border.color = lv_color_hsv_to_rgb(240, 60, 60);
  blue_led.body.shadow.color = lv_color_hsv_to_rgb(240, 100, 100);

  lv_style_copy(&panel_bg, th->style.panel);
  panel_bg.body.main_color = lv_color_hex(0x222222);
  panel_bg.body.grad_color = lv_color_hex(0x555555);
  panel_bg.body.border.width = 3;
  panel_bg.body.border.color = lv_color_hex(0xeeeeee);

  lv_theme_set_current(th);
}

// void update_ui(lv_task_t *task) { 
//   ui.do_update_objects(task);
// }

// void setup_ui_update_objects() {
//   ui.add_update_object(ID_LABEL_TIME, 200);
//   ui.add_update_object(ID_LED_MQTT, 500);
//   ui.add_update_object(ID_LED_EXTRA1, 1000);
// }

void setup() {
  Serial.begin(115200); /* prepare for possible serial debug */
  esp_log_level_set("*", ESP_LOG_INFO);
  // ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  // ledcAttachPin(TFT_LED, 10);
  // analogReadResolution(10);
  // ledcWrite(10,0);

  disp_spi_init();
  ili9488_init();
  tp_spi_init();
  xpt2046_init();

  lv_init();

  static uint16_t buffer_size = 20;
  // static lv_color_t * buf1 = (lv_color_t *) ps_malloc(sizeof(lv_color_t) *
  // LV_HOR_RES_MAX * buffer_size); static lv_color_t * buf2 = (lv_color_t *)
  // ps_malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  static lv_color_t *buf1 =
      (lv_color_t *)malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  // static lv_color_t * buf2 = (lv_color_t *) malloc(sizeof(lv_color_t) *
  // LV_HOR_RES_MAX * buffer_size);
  static lv_disp_buf_t disp_buf;
  lv_disp_buf_init(&disp_buf, buf1, NULL, LV_HOR_RES_MAX * buffer_size);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 480;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = ili9488_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

#if ENABLE_TOUCH_INPUT
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read_cb = xpt2046_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
#endif

#ifdef USE_MQTT
  connectToWifi();
 // set_ui_object_subscriber(ui_subscriber_list, NUM_SUBSCRIBERS);
#endif

  /*Initialize the graphics library's tick*/
  esp_register_freertos_tick_hook(lv_tick_task);

  setup_theme();
  create_gui();
  //setup_ui_update_objects();

  //lv_task_t *task = lv_task_create(update_ui, 50, LV_TASK_PRIO_MID, nullptr);
}

void loop() {
  vTaskDelay(1);
#ifdef USE_MQTT
    if (!wifi_is_connected()) {
       ui.label_add_text(ID_TERMINAL, "Wifi disconnected\n",TERMINAL_LOG_LENGTH);
    }
  mqtt_loop();
#endif
  lv_task_handler();
}