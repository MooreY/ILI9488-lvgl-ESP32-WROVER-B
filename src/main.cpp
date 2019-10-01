#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include <lvgl.h>

#include "drv/disp_spi.h"
#include "drv/ili9488.h"
#include "drv/tp_spi.h"
#include "drv/xpt2046.h"
#include <demo/demo.h>

#define LVGL_TICK_PERIOD 20

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif


static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);
}

void testram(int ramSize) {
  // Test SPI-RAM: write
  //uint8_t * mem = (uint8_t *) ps_malloc(ramSize);
  uint8_t * mem = (uint8_t *) heap_caps_malloc(ramSize, MALLOC_CAP_SPIRAM);
  if(! mem) {
    Serial.printf("ERROR no mem\n");
    return;
  }
  for(int i=0; i<(ramSize); i++) {
    mem[i] = (0xA5-i) & 0xFF;
  }
  // Test SPI-RAM: verify
  int e = 0;
  for(int i=0; i<(ramSize); i++) {
    if(mem[i] != ((0xA5-i) & 0xFF)) e++;
  }
  free(mem);
  Serial.printf(" %7d Bytes: %7d OK, %7d Errors.\n",ramSize,ramSize-e,e);
}

void setup()
{

  Serial.begin(115200); /* prepare for possible serial debug */
  // ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  // ledcAttachPin(TFT_LED, 10);
  // analogReadResolution(10);
  // ledcWrite(10,0);
  //psramInit();
  if(psramFound()) {
    Serial.printf("\nPS_RAM found.\n\n");
    Serial.printf("\n%d Bytes free.\n\n", ESP.getFreeHeap());
  }
  // test SPI RAM
  //Serial.print("Test  4MB:"); testram((4*1024*1024)-64);


#if USE_LV_LOG != 0
  lv_log_register_print(my_print); /* register print function for debugging */
#endif

	disp_spi_init();
	ili9488_init();
  tp_spi_init();
  xpt2046_init();

  lv_init();

  static uint16_t buffer_size = 20;
  // static lv_color_t * buf1 = (lv_color_t *) ps_malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  // static lv_color_t * buf2 = (lv_color_t *) ps_malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
  static lv_color_t * buf1 = (lv_color_t *) malloc(sizeof(lv_color_t) * LV_HOR_RES_MAX * buffer_size);
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

  demo_create();
  // ledcWrite(10,1024);
}

void loop() {
  vTaskDelay(1);
	lv_task_handler();
}