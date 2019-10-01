/**
 * @file lv_templ.h
 *
 */

#ifndef ILI9488_H
#define ILI9488_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
#define ILI9488_DC   2
#define ILI9488_RST  4
#define ILI9488_BCKL 5

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} lv_color_custom_t;
/**
 * Display direction option
 */
enum ili9488_display_direction{
	LANDSCAPE  = 0,
	PORTRAIT   = 1
};

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void ili9488_init(void);
void ili9488_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9488_H*/
