#pragma once

#include "gui/ui.h" 
#include "resource-id.h"

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
	//	{ID_TAB3,"Spiegel"},
		{ID_TAB4,LV_SYMBOL_DOWNLOAD},
};

static const int NUM_TAB_ITEMS = 4;


