# ILI9488-lvgl-ESP32-WROVER-B

- [ILI9488-lvgl-ESP32-WROVER-B](#ili9488-lvgl-esp32-wrover-b)
- [Hardware connectie TFT to ESP32 WROVER B:](#hardware-connectie-tft-to-esp32-wrover-b)
- [PS-RAM usage:](#ps-ram-usage)autoauto


## Hardware connectie TFT to ESP32 WROVER B: 
![hardware](https://github.com/mvturnho/ILI9488-lvgl-ESP32-WROVER-B/blob/master/doc/images/ili9488-esp32.png?raw=true "Hardware connections")

## PS-RAM usage:

```
build_flags =
  -DBOARD_HAS_PSRAM
  -mfix-esp32-psram-cache-issue
```

```cpp
uint8_t * mem = (uint8_t *) ps_malloc(ramSize);
```
