#pragma once

#include <lvgl/lvgl.h>
#include "resource-id.h"
#include <stdarg.h>
#include <time.h>
#include <math.h>

enum class ui_object_type {
	PAGE = 0,
	CONTAINER,
	LABEL,
	TABVIEW,
	TAB,
	BUTTON,
	CHECKBOX,
	SLIDER,
	SWITCH,
	LIST,
	CHART,
	GAUGE,
	LED,
	BAR,
	CALENDAR,
	SPINBOX,
	DROPDOWN_LIST,
	ROLLER,
	TEXTAREA,
	KEYBOARD,
};

enum class ui_event_t {
	CLICKED,
	LONG_CLICKED,
	VALUE_CHANGED,
	DRAG_BEGIN,
	DRAG_END,
	PRESSED,
	RELEASED,
	PRESSED_REPEAT,
};

enum calendar_value_type {
	TODAY_DATE,
	HIGHLIGHT_DATES,
};

typedef struct {
	ui_object_id id;
	const char* symbol;
	const char* label;
} ui_list_item_t;

typedef struct {
	ui_object_id id;
	const char* label;
} ui_tab_item_t;

typedef struct {
	uint8_t style_type;
	lv_style_t* style;
} ui_styles_t;

typedef struct ui_object_t ui_object_t;
typedef void (*ui_object_handler_cb_ptr)(ui_object_t*, ui_event_t);

class ui_object_t {
public:
	ui_object_id id;
	ui_object_type type;
	lv_obj_t* lv_obj;
	ui_object_handler_cb_ptr ui_callback;
	void* object_data;
};

static const char* VALUE_ON = "ON";
static const char* VALUE_OFF = "OFF";

class ui_dropdown_specs {
public:
	ui_dropdown_specs(const char* items, bool arrow) {
		this->items = items;
		this->arrow = arrow;
	}
	const char* items;
	bool arrow = true;

	void apply_specs(lv_obj_t* lv_obj_dropdown) {
		lv_ddlist_set_options(lv_obj_dropdown, items);
		lv_ddlist_set_draw_arrow(lv_obj_dropdown, arrow);
	}
};

class ui_roller_specs {
public:
	ui_roller_specs(const char* items, uint16_t vissible_rows, bool infinite = true) {
		this->items = items;
		this->infinite = infinite;
		this->visible_rows = vissible_rows;
	}
	const char* items;
	bool infinite = true;
	uint16_t visible_rows;

	void apply_specs(lv_obj_t* lv_obj_roller) {
		lv_ddlist_set_options(lv_obj_roller, items);
		if (infinite)
			lv_roller_set_options(lv_obj_roller, items, LV_ROLLER_MODE_INIFINITE);
		else
			lv_roller_set_options(lv_obj_roller, items, LV_ROLLER_MODE_NORMAL);

		lv_roller_set_visible_row_count(lv_obj_roller, visible_rows);
	}
};

class ui_textarea_data {
public:
	ui_object_t* ui_textarea;
	ui_object_t* ui_keyboard;
	bool show_kb_on_click = true;
	bool hide_kb_on_done = true;
	void set_text_area(ui_object_t* ta) {
		ta->object_data = this;
		this->ui_textarea = ta;
	}	
	void set_keyboard(ui_object_t* kb) {
		kb->object_data = this;
		this->ui_keyboard = kb;
	}
};

class ui_spinbox_specs {
public:
	ui_spinbox_specs(int count, int sep, int left, int min, int max, int val, int step) {
		digit_count = count;
		seperator_pos = sep;
		left_number_padding = left;
		this->min = min;
		this->max = max;
		this->value = val;
		this->step = step;
		this->fract = 1;
		if (seperator_pos > 0 && seperator_pos < count)
			fract = pow(10, (digit_count - seperator_pos));

	}
	int digit_count;
	int seperator_pos;
	int left_number_padding;
	int min;
	int max;
	int value;
	int step;
	int fract;

	void apply_specs(lv_obj_t* lv_obj_spinner) {
		lv_spinbox_set_digit_format(lv_obj_spinner, digit_count, seperator_pos);
		lv_spinbox_set_padding_left(lv_obj_spinner, left_number_padding);
		lv_spinbox_set_range(lv_obj_spinner, min, max);
		lv_spinbox_set_step(lv_obj_spinner, step);
	}
};

class ui_object_value {
public:
	int int_value = 0;
	float float_value = 0.0;
	const char* char_value = new char[10];
	double double_value = 0;
	bool bool_value = false;
	tm* date_time = new tm();

	static const int buff_size = 120;

	void set_value(char* string) {
		const char* tmp = char_value;
		char* buffer = new char[buff_size];
		strcpy(buffer, string);
		char_value = buffer;
		delete(tmp);
	}

	void set_value(bool value) {
		bool_value = value;
		if (value)
			char_value = VALUE_ON;
		else
			char_value = VALUE_OFF;
	}

	void set_value(int value) {
		int_value = value;
		float_value = (float)0.0 + value;
		sprintf((char*)char_value, "%d", value);
	}

	void set_date(int day, int month, int year) {
		date_time->tm_mday = day;
		date_time->tm_mon = month;
		date_time->tm_year = year - 1900;
		set_date_str();
	}

	void set_time(int h, int m, int s) {
		date_time->tm_hour = h;
		date_time->tm_min = m;
		date_time->tm_sec = s;
		set_time_str();
	}

	void set_date_str() {
		const char* tmp = char_value;
		char* buffer = new char[buff_size];
		strftime(buffer, buff_size, "%d %B %Y", date_time);
		char_value = buffer;
		delete(tmp);
	}

	void set_time_str() {
		const char* tmp = char_value;
		char* buffer = new char[buff_size];
		strftime(buffer, buff_size, "%h:%m:%s", date_time);
		char_value = (const char*)buffer;
		delete(tmp);
	}

	void set_date_time_str() {
		const char* tmp = char_value;
		char* buffer = new char[buff_size];
		strftime(buffer, buff_size, "%h:%m:%s %d %B %Y", date_time);
		char_value = (const char*)buffer;
		delete(tmp);
	}

	std::string* get_string() {
		return new std::string(char_value);
	}


};

static lv_obj_t* full_keyboard;



class UI {

public:

	ui_object_t* find_object_by_id(ui_object_id id);

	ui_object_value get_value(ui_object_t* ui_obj);

	void set_value(ui_object_id id, int sOne, ...);
	void set_size(ui_object_id id, lv_coord_t width, lv_coord_t height = 0);
	void set_size(ui_object_id id, ui_object_id ref_id);
	void set_style(ui_object_id id, lv_style_t* lv_style, int style_type = 0);
	void set_style(ui_object_id id, ui_styles_t* ui_styles, uint16_t num_items);
	void set_align(ui_object_id id, lv_align_t align, ui_object_id base_id, int x_offset = 0, int y_offset = 0);
	void set_floating(ui_object_id id, ui_object_id parent_id);

	void spinbox_increment(ui_object_id id, bool increment);

	ui_object_t* create_page(ui_object_id parent_id, ui_object_id id, uint32_t width = LV_HOR_RES, uint32_t height = LV_VER_RES);

	ui_object_t* create_container(ui_object_id parent_id, ui_object_id id, uint32_t width = LV_HOR_RES, uint32_t height = LV_VER_RES);

	ui_object_t* add_textarea(ui_object_id parent_id, ui_object_id id, bool use_keyboard, bool oneline, bool pw_mode, ui_object_handler_cb_ptr callback = nullptr, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);

	ui_object_t* add_label(ui_object_id parent_id, ui_object_id id, const char* text, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	void label_add_text(ui_object_id parent_id, const char* text_in, uint32_t max_length);

	ui_object_t* add_switch(ui_object_id parent_id, ui_object_id id, bool state = false, ui_object_handler_cb_ptr callback = nullptr, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	ui_object_t* add_button(ui_object_id parent_id, ui_object_id id, ui_object_id label_id, const char* label, ui_object_handler_cb_ptr callback, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	ui_object_t* add_checkbox(ui_object_id parent_id, ui_object_id id, const char* label, bool state, ui_object_handler_cb_ptr callback, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	ui_object_t* add_led(ui_object_id parent_id, ui_object_id id, bool state = false, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	ui_object_t* add_bar(ui_object_id parent_id, ui_object_id id, int value = 0, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0);


	ui_object_t* add_calendar(ui_object_id parent_id, ui_object_id id, ui_object_handler_cb_ptr callback, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0);

	ui_object_t* create_tabview(ui_object_id parent_id, ui_object_id id, ui_tab_item_t* tabs, uint32_t num_tabs, uint32_t width = LV_HOR_RES, uint32_t height = LV_VER_RES);
	ui_object_t* add_tab(ui_object_id parent_id, ui_object_id id, const char* text);

	ui_object_t* add_list(ui_object_id parent_id, ui_object_id id, ui_list_item_t* items, uint32_t num_items, ui_object_handler_cb_ptr callback,
		lv_align_t align = 999, ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0);
	ui_object_t* add_dropdown(ui_object_id parent_id, ui_object_id id, ui_dropdown_specs* specs, ui_object_handler_cb_ptr callback,
		lv_align_t align = 999, ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0);
	ui_object_t* add_roller(ui_object_id parent_id, ui_object_id id, ui_roller_specs* items, ui_object_handler_cb_ptr callback,
		lv_align_t align = 999, ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0);

	ui_object_t* add_slider(ui_object_id parent_id, ui_object_id id, int value, ui_object_handler_cb_ptr callback, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);
	ui_object_t* add_spinbox(ui_object_id parent_id, ui_object_id id, ui_spinbox_specs* value, ui_object_handler_cb_ptr callback, lv_align_t align = 999,
		ui_object_id base_id = ID_NONE, uint32_t x_offset = 0, uint32_t y_offset = 0, const lv_style_t* lv_style = nullptr);

	ui_list_item_t* object_id_in_list(ui_list_item_t* list, ui_object_t* ui_object);

protected:
	static std::vector<ui_object_t*> ui_object_list_;

	lv_obj_t* find_lv_object_by_id(ui_object_id id);
	lv_obj_t* find_parent_object(ui_object_id id);
	static ui_object_t* find_object_by_ref(lv_obj_t* ref);

	ui_object_t* add_tab(lv_obj_t* parent, ui_object_id id, const char* text);

	ui_object_t* add_object(lv_obj_t* lv_obj, ui_object_id id, ui_object_type object_type, ui_object_handler_cb_ptr callback = nullptr);
	//bool delete_ui_object(const ui_object_t* ui_obj);

	static void event_handler(lv_obj_t* lv_obj, lv_event_t lv_event);
	static void text_area_event_handler(lv_obj_t* text_area, lv_event_t event);
	static void keyboard_event_cb(lv_obj_t* keyboard, lv_event_t event);
	static void kb_hide_anim_end(lv_anim_t* a);

	void set_size(ui_object_t* ui_obj, lv_coord_t lv_width, lv_coord_t lv_height);
};