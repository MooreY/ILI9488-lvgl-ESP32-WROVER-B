#pragma once

#ifndef STYLES_H
#define STYLES_H

#include <lvgl/lvgl.h>

static lv_style_t style_bg;
static lv_style_t style_indic;
static lv_style_t style_knob;
static lv_style_t style_list;
static lv_style_t bar_bg;
static lv_style_t bar_indic;
static lv_style_t time_style;
static lv_style_t style1;
static lv_style_t style_tv_btn_pr;
static lv_style_t style_tv_btn_rel;
static lv_style_t style_tv_btn_bg;
static lv_style_t style_kb;
static lv_style_t style_kb_rel;
static lv_style_t style_kb_pr;
static lv_style_t style_led;
static lv_style_t cal_style;



void ui_create_styles() {
	//static lv_style_t style_tv_btn_bg;
	lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
	style_tv_btn_bg.body.main_color = lv_color_hex(0x487fb7);
	style_tv_btn_bg.body.grad_color = lv_color_hex(0x487fb7);
	style_tv_btn_bg.body.padding.top = 0;
	style_tv_btn_bg.body.padding.bottom = 0;

	//static lv_style_t style_tv_btn_rel;
	lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
	style_tv_btn_rel.body.opa = LV_OPA_TRANSP;
	style_tv_btn_rel.body.border.width = 0;

	//static lv_style_t style_tv_btn_pr;
	lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
	style_tv_btn_pr.body.radius = 0;
	style_tv_btn_pr.body.opa = LV_OPA_50;
//	style_tv_btn_pr.body.main_color = LV_COLOR_WHITE;
//	style_tv_btn_pr.body.grad_color = LV_COLOR_WHITE;
	style_tv_btn_pr.body.border.width = 0;
//	style_tv_btn_pr.text.color = LV_COLOR_GRAY;

		
	/*
	LED
	*/
	lv_style_copy(&style_led, &lv_style_pretty_color);
	style_led.body.radius = LV_RADIUS_CIRCLE;
	style_led.body.border.width = 3;
	style_led.body.border.opa = LV_OPA_30;
	style_led.body.shadow.width = 5;


	/*
	CALENDAR
	*/
	lv_style_copy(&cal_style, &lv_style_pretty_color);
	cal_style.text.color = lv_color_make(0xff, 0x00, 0x00);

	/*
	KEYBOARD
	*/
	lv_style_copy(&style_kb, &lv_style_plain);

	style_kb.body.opa = LV_OPA_70;
	style_kb.body.main_color = lv_color_hex3(0x333);
	style_kb.body.grad_color = lv_color_hex3(0x333);
	style_kb.body.padding.left = 0;
	style_kb.body.padding.right = 0;
	style_kb.body.padding.top = 0;
	style_kb.body.padding.bottom = 0;
	style_kb.body.padding.inner = 0;

	lv_style_copy(&style_kb_rel, &lv_style_plain);
	style_kb_rel.body.opa = LV_OPA_TRANSP;
	style_kb_rel.body.radius = 0;
	style_kb_rel.body.border.width = 1;
//	style_kb_rel.body.border.color = LV_COLOR_SILVER;
	style_kb_rel.body.border.opa = LV_OPA_50;
	style_kb_rel.body.main_color = lv_color_hex3(0x333);    /*Recommended if LV_VDB_SIZE == 0 and bpp > 1 fonts are used*/
	style_kb_rel.body.grad_color = lv_color_hex3(0x333);
//	style_kb_rel.text.color = LV_COLOR_WHITE;

	lv_style_copy(&style_kb_pr, &lv_style_plain);
	style_kb_pr.body.radius = 0;
	style_kb_pr.body.opa = LV_OPA_50;
//	style_kb_pr.body.main_color = LV_COLOR_WHITE;
//	style_kb_pr.body.grad_color = LV_COLOR_WHITE;
	style_kb_pr.body.border.width = 1;
//	style_kb_pr.body.border.color = LV_COLOR_SILVER;
};

#endif