#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_freertos_hooks.h>
#include <esp_log.h>
#include <lvgl/lvgl.h>

#include "drv/disp_spi.h"
#include "drv/ili9488.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"
//#include <tpcal/tpcal.h>

#include <demo/demo.h>
#include <components/mqtt.h>
#include <styles.h>
#include <gui/ui.h>

#define LVGL_TICK_PERIOD 20

//#define USE_MQTT

/*********************
*      DEFINES
*********************/

static const int TERMINAL_LOG_LENGTH = 100;

using std::string;
//using json11::Json;

/**********************
*      TYPEDEFS
**********************/
typedef struct {
	ui_object_id id;
	const char* topic;
} mqtt_ui_object_t;

typedef int (*update_cb_t) (uint32_t);

/**********************
*  STATIC PROTOTYPES
**********************/
static void hal_init(void);
static int tick_thread(void* data);

/**********************
*  STATIC VARIABLES
**********************/

//static lv_indev_t* kb_indev;

static UI ui;

static ui_list_item_t list_items[] = {
	{ID_LIST_SYMBOLS_FILE, LV_SYMBOL_FILE,"File"},
	{ID_LIST_SYMBOLS_AUDIO, LV_SYMBOL_AUDIO,"Audio"},
	{ID_LIST_SYMBOLS_BT, LV_SYMBOL_BLUETOOTH,"Bluetooth"},
	{ID_LIST_SYMBOLS_WIFI,LV_SYMBOL_WIFI,"WiFI"},
};

static ui_list_item_t list_items2[] = {
	{ID_LIST2_SYMBOLS_FILE, LV_SYMBOL_FILE,"File"},
	{ID_LIST2_SYMBOLS_AUDIO, LV_SYMBOL_AUDIO,"Audio"},
	{ID_LIST2_SYMBOLS_BT, LV_SYMBOL_BLUETOOTH,"Bluetooth"},
	{ID_LIST2_SYMBOLS_WIFI,LV_SYMBOL_WIFI,"WiFI"},
	{ID_LIST3_SYMBOLS_FILE, LV_SYMBOL_FILE,"File"},
	{ID_LIST3_SYMBOLS_AUDIO, LV_SYMBOL_AUDIO,"Audio"},
	{ID_LIST3_SYMBOLS_BT, LV_SYMBOL_BLUETOOTH,"Bluetooth"},
	{ID_LIST3_SYMBOLS_WIFI,LV_SYMBOL_WIFI,"WiFI"},
};

static ui_tab_item_t tabview_items[] = {
	{ID_TAB1,"Woonkamer"},
	{ID_TAB2,"MeterKast"},
	{ID_TAB3,"TrapKast"},
	{ID_TAB4,"Input"},
	{ID_TAB_TERMINAL,LV_SYMBOL_DOWNLOAD},
};
static const int NUM_TAB_ITEMS = 5;

#ifdef USE_MQTT
static mqtt_ui_object_t ui_subscriber_list[] = {
	{ID_SLIDER_LEDS, "led/light" },
	{ID_SWITCH_LEDS, "tft/switch"},
};
static const int NUM_SUBSCRIBERS = 2;

static mqtt_ui_object_t ui_publisher_list[] = {
	{ID_SLIDER_LEDS, "top/light" },
	{ID_SWITCH_LEDS, "top/switch"},
	{ID_SWITCH_GANG, "gang/switch"}
};
static const int NUM_PUBLISHERS = 3;
#endif

static ui_dropdown_specs* dropdown_spec = new ui_dropdown_specs({ "Apple\n" "Banana\n"	"Orange\n"	"Melon\n"	"Grape\n"	"Raspberry" }, true);
static ui_roller_specs* roller_specs = new ui_roller_specs({ "January\nFebruary\nMarch\nApril\nMay\nJune\nJuly\nAugust\nSeptember\nOctober\nNovember\nDecember" }, 5, true);

static ui_styles_t tabview_styles[] = {
	{ LV_TABVIEW_STYLE_BTN_BG, &style_tv_btn_bg},
	{ LV_TABVIEW_STYLE_INDIC, &lv_style_plain },
	{ LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel },
	{ LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr },
	{ LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel },
	{ LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr }
};
static const int NUM_TABVIEW_STYLE_ITEMS = 6;

std::vector<ui_object_t*> ui_time_objects;

/**********************
*      EVENT HANDLERS
**********************/
#ifdef USE_MQTT
void mqtt_cb(const std::string& topic, const std::string& payload) {
	printf("MQTT CB %s, payload %s\n", topic.c_str(), payload.c_str());
	ui.label_add_text(ID_TERMINAL, topic.c_str(), TERMINAL_LOG_LENGTH);
	if (topic.compare("tft/switch") == 0) {
		bool state = payload.compare("ON");
		ui.set_value(ID_SWITCH_LEDS, 1, state);
		ui.set_value(ID_LED_MQTT, 1, state);
	}
	else if (topic.compare("led/light") == 0) {
		std::string err;
		Json json_payl = Json::parse(payload, err);
		int value = json_payl["brightness"].int_value();
		ui.set_value(ID_BAR_VALUE, 1, value);
	}
}

std::string* get_ui_publisher_topic(ui_object_id id) {
	for (int i = 0; i < NUM_PUBLISHERS; i++) {
		if (ui_publisher_list[i].id == id) {
			return new std::string(ui_publisher_list[i].topic);
		}
	}
	return nullptr;
}
#endif

void ui_handler(ui_object_t* ui_object, ui_event_t event) {
	switch (ui_object->id) {
	case ID_CB_BACKLIGHT:
	case ID_CB_SCREENSAVER:
	case ID_SWITCH_LEDS:
	case ID_SWITCH_GANG: {
		auto value = ui.get_value(ui_object);
		// auto topic = get_ui_publisher_topic(ui_object->id);
		// mqtt_publish(topic, value.get_string());
		ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		break;
	}
	case ID_SLIDER_LEDS: {
		auto value = ui.get_value(ui_object);
		// mqtt_publish(get_ui_publisher_topic(ID_SLIDER_LEDS), value.get_string());
		ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		ui.set_value(ID_BAR_VALUE, 1, value.int_value);
		break;
	}
	case ID_BUTTON_CLEAR: {
		ui.set_value(ID_TERMINAL, 1, "");
		break;
	}
	case ID_CALENDAR: {
		auto value = ui.get_value(ui_object);
		ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		ui.set_value(ID_LABEL_DATE, 1, value.char_value);
		break;
	}
	case ID_SPINBOX: {
		if (event == ui_event_t::CLICKED) {
			ui.spinbox_increment(ID_SPINBOX, true);
			auto value = ui.get_value(ui_object);
			ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		}
		break;
	}
	case ID_ROLLER: {
		if (event == ui_event_t::VALUE_CHANGED) {
			auto value = ui.get_value(ui_object);
			ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		}
		break;
	}
	case ID_DROPDOWN: {
		if (event == ui_event_t::VALUE_CHANGED) {
			auto value = ui.get_value(ui_object);
			ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		}
		break;
	}
	case ID_TEXTAREA_INPUT: {
		if (event == ui_event_t::CLICKED) {
			printf("klicked TA");
		}
		if (event == ui_event_t::VALUE_CHANGED) {
			auto value = ui.get_value(ui_object);
			ui.label_add_text(ID_TERMINAL, value.char_value, TERMINAL_LOG_LENGTH);
		}
		break;
	}
	}
}

/*

*/
void list_handler(ui_object_t* ui_object, ui_event_t event) {
	if (event == ui_event_t::CLICKED) {
		ui_list_item_t* item = ui.object_id_in_list(list_items, ui_object);
		if (item) {
			ui.label_add_text(ID_TERMINAL, item->label, TERMINAL_LOG_LENGTH);
		}
	}
}

static const char* TAG_MAIN = "MAIN";

static void IRAM_ATTR lv_tick_task(void)
{
  lv_tick_inc(portTICK_RATE_MS);
}

void ui_create() {
	ui_create_styles();

	ui_object_t* tv = ui.create_tabview(ID_NONE, ID_TAB_VIEW, tabview_items, NUM_TAB_ITEMS, LV_HOR_RES, LV_VER_RES);
	ui.set_style(ID_TAB_VIEW, tabview_styles, NUM_TABVIEW_STYLE_ITEMS);

	ui.add_label(ID_TAB1, ID_LABEL_LEDS, "leds achter de bank", LV_ALIGN_IN_TOP_LEFT, ID_TAB1, 5, 10);
	ui.add_switch(ID_TAB1, ID_SWITCH_LEDS, false, ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB1, 150, 5);
	ui.add_label(ID_TAB1, ID_LABEL_GANG, "kleur in gang", LV_ALIGN_IN_TOP_LEFT, ID_TAB1, 5, 45);
	ui.add_switch(ID_TAB1, ID_SWITCH_GANG, false, ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB1, 150, 40);
	ui.add_slider(ID_TAB1, ID_SLIDER_LEDS, 0, ui_handler, LV_ALIGN_OUT_RIGHT_MID, ID_SWITCH_LEDS, 20, 0);
	ui.add_label(ID_TAB1, ID_LABEL_TIME, "10:00", LV_ALIGN_OUT_BOTTOM_LEFT, ID_SLIDER_LEDS, 5, 10);

	ui.add_roller(ID_TAB1, ID_ROLLER, roller_specs, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANG, 0, 60);
	ui.add_dropdown(ID_TAB1, ID_DROPDOWN, dropdown_spec, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_SWITCH_GANG, 0, 20);
	ui.set_size(ID_DROPDOWN, 130, 0);
	ui.set_size(ID_ROLLER, ID_DROPDOWN);

	ui.add_list(ID_TAB2, ID_LIST_SYMBOLS, list_items, 4, list_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB2, 5, 5);
	ui.add_list(ID_TAB2, ID_LIST2_SYMBOLS2, list_items2, 8, list_handler, LV_ALIGN_OUT_RIGHT_TOP, ID_LIST_SYMBOLS, 10, 0);
	ui.set_size(ID_LIST2_SYMBOLS2, ID_LIST_SYMBOLS);

	ui.add_label(ID_TAB_TERMINAL, ID_TERMINAL, "BYE");
	ui.add_button(ID_TAB_TERMINAL, ID_BUTTON_CLEAR, ID_BUTTON_CLEAR_LABEL, "Clear", ui_handler, LV_ALIGN_IN_BOTTOM_RIGHT, ID_TAB_TERMINAL, -10, 10);
	ui.set_floating(ID_BUTTON_CLEAR, ID_TAB_TERMINAL);
	ui.set_size(ID_BUTTON_CLEAR, 100, 40);
	ui.add_textarea(ID_TAB_TERMINAL, ID_TEXTFIELD_SEARCH, true, true,false, ui_handler, LV_ALIGN_IN_TOP_RIGHT, ID_TAB_TERMINAL, 0, 0);
	ui.set_floating(ID_TEXTFIELD_SEARCH, ID_TAB_TERMINAL);

	ui.add_checkbox(ID_TAB3, ID_CB_SCREENSAVER, "Screensaver", false, ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB3, 5, 10);
	ui.add_checkbox(ID_TAB3, ID_CB_BACKLIGHT, "Backlight", false, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_CB_SCREENSAVER, 0, 0);

	ui.add_led(ID_TAB3, ID_LED_SCREENSAVER, false, LV_ALIGN_OUT_BOTTOM_LEFT, ID_CB_BACKLIGHT, 0, 10, &style_led);
	ui.add_led(ID_TAB3, ID_LED_BACKLIGHT, false, LV_ALIGN_OUT_RIGHT_MID, ID_LED_SCREENSAVER, 20, 0, &style_led);
	ui.add_led(ID_TAB3, ID_LED_MQTT, false, LV_ALIGN_OUT_RIGHT_MID, ID_LED_BACKLIGHT, 20, 0, &style_led);

	ui.add_bar(ID_TAB3, ID_BAR_VALUE, 0, LV_ALIGN_OUT_BOTTOM_LEFT, ID_LED_SCREENSAVER, 0, 20);
	ui.set_size(ID_BAR_VALUE, 200, 20);
	//CALENDAR
	ui.add_calendar(ID_TAB3, ID_CALENDAR, ui_handler, LV_ALIGN_OUT_RIGHT_TOP, ID_CB_SCREENSAVER, 100, 5);
	ui.set_value(ID_CALENDAR, 4, TODAY_DATE, 14, 11, 2019);
	uint16_t party[] = { 18,11,2019 };	uint16_t church[] = { 21,11,2019 };	uint16_t kingsday[] = { 22,11,2019 };
	ui.set_value(ID_CALENDAR, 4, HIGHLIGHT_DATES, party, church, kingsday);
	ui.set_style(ID_CALENDAR, &cal_style, LV_CALENDAR_STYLE_HIGHLIGHTED_DAYS);

	ui.add_label(ID_TAB3, ID_LABEL_DATE, "date label", LV_ALIGN_OUT_BOTTOM_LEFT, ID_CALENDAR, 10, 10);
	//SPINBOX
	ui_spinbox_specs* spinbox_def = new ui_spinbox_specs(3, 1, 2, 0, 100, 0, 1); // digit_count, fract, left_padding,min,max,value,step
	ui.add_spinbox(ID_TAB3, ID_SPINBOX, spinbox_def, ui_handler, LV_ALIGN_OUT_BOTTOM_LEFT, ID_BAR_VALUE, 0, 20);

	ui.add_textarea(ID_TAB4, ID_TEXTAREA_INPUT, true, false, false ,ui_handler, LV_ALIGN_IN_TOP_LEFT, ID_TAB4, 0, 0);
	ui.set_size(ID_TEXTAREA_INPUT, ID_TAB4);
}

void setup()
{
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

  connectToWifi();

  static uint16_t buffer_size = 20;
  // static lv_color_t * buf1 = (lv_color_t *) ps_malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  // static lv_color_t * buf2 = (lv_color_t *) ps_malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  static lv_color_t *buf1 = (lv_color_t *)malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  // static lv_color_t * buf2 = (lv_color_t *) malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
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

  /*Initialize the graphics library's tick*/
  esp_register_freertos_tick_hook(lv_tick_task);

  ui_create();

#ifdef USE_MQTT
  connectToWifi();
	//set_ui_object_subscriber(ui_subscriber_list, NUM_SUBSCRIBERS);
#endif
}

void loop()
{
  vTaskDelay(1);
  if (!wifi_is_connected()) {
     terminal_add("Wifi disconnected\n",0);
  }
  //mqtt_loop();
  lv_task_handler();
}