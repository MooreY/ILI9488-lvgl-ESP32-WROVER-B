/**
 * @file XPT2046.h
 *
 */

#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdbool.h>
#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/
#define XPT2046_IRQ 25

#define XPT2046_AVG 4
#define XPT2046_X_MIN 235
#define XPT2046_Y_MIN 154
#define XPT2046_X_MAX 3970
#define XPT2046_Y_MAX 3870
#define XPT2046_X_INV 0
#define XPT2046_Y_INV 0

/**********************
 *      TYPEDEFS
 **********************/
static const char* TAG = "xpt2046";
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void xpt2046_init(void);
bool xpt2046_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XPT2046_H */