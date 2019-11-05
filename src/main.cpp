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
#include <ui/ui.h>


#define LVGL_TICK_PERIOD 20


static const char* TAG_MAIN = "MAIN";
UI ui;

static void IRAM_ATTR lv_tick_task(void)
{
  lv_tick_inc(portTICK_RATE_MS);
}

void testram(int ramSize)
{
  // Test SPI-RAM: write
  //uint8_t * mem = (uint8_t *) ps_malloc(ramSize);
  uint8_t *mem = (uint8_t *)heap_caps_malloc(ramSize, MALLOC_CAP_SPIRAM);
  if (!mem)
  {
    ESP_LOGD(TAG_MAIN, "ERROR no mem\n");
    return;
  }
  for (int i = 0; i < (ramSize); i++)
  {
    mem[i] = (0xA5 - i) & 0xFF;
  }
  // Test SPI-RAM: verify
  int e = 0;
  for (int i = 0; i < (ramSize); i++)
  {
    if (mem[i] != ((0xA5 - i) & 0xFF))
      e++;
  }
  free(mem);
  ESP_LOGD(TAG_MAIN, " %7d Bytes: %7d OK, %7d Errors.\n", ramSize, ramSize - e, e);
}

void mqtt_cb(const std::string &topic, const std::string &payload) {
   Serial.printf( "MQTT CB %s, payload %s", topic.c_str(), payload.c_str());
   UI_Object *obj = ui.find_object_by_id(100);
   if(obj!=nullptr){
     obj->set_value(topic);
   }
}

void ui_callback(int id) {
  Serial.printf( "UI CB id=%d", id);
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  esp_log_level_set("*", ESP_LOG_INFO);
  // ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  // ledcAttachPin(TFT_LED, 10);
  // analogReadResolution(10);
  // ledcWrite(10,0);
  if (psramFound())
  {
    ESP_LOGI(TAG_MAIN, "\nPS_RAM found.\n");
    ESP_LOGI(TAG_MAIN, "%d PSRam size.\n", ESP.getPsramSize());
    ESP_LOGI(TAG_MAIN, "%d Bytes free.\n\n", ESP.getFreeHeap());
    //ESP_LOGD(TAG_MAIN, "Test  4MB:"); 
    //testram((4*1024*1024)-64);
  }
 
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

  //tpcal_create();
  demo_create();
  //ledcWrite(10,1024);
  String data = "IP adress: " + get_local_ip();
  // terminal_add(data.c_str(), data.length());
  // add_list_button(data.c_str());
 
  mqtt_subscribe("tft/switch", mqtt_cb,1);
  //UI_Object *obj = ui.add_ui_object(100,1,lv_disp_get_scr_act(NULL),ui_callback,"label");
  //obj = ui.add_ui_object(101,2,lv_disp_get_scr_act(NULL),ui_callback,"label");
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