#pragma once

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

enum ui_keyboard_type {
	UI_KEYB_NONE,
	UI_KEYB_NUM,
	UI_KEYB_TEXT,
};

class ui_keyboard_data {
public:
	ui_keyboard_type type;
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

typedef struct {
	ui_object_id id;
	const char* topic;
} mqtt_ui_object_t;

typedef int (*update_cb_t) (uint32_t);

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

class ui_update_object_c {
public:
	ui_update_object_c(ui_object_t* ui_obj, uint32_t ms_time) {
		this->ui_obj = ui_obj;
		this->ms_time = ms_time;
	}

	bool go(unsigned long  now) {
		if ((this->last_time + this->ms_time) < now) {
			this->last_time = now;
			return true;
		}
		return false;
	}

	uint32_t ms_time;
	unsigned long  last_time;
	ui_object_t* ui_obj;
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