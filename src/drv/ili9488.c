/**
 * @file ili9488.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9488.h"
#include "disp_spi.h"
#include "ili9488_defines.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9488_send_cmd(uint8_t cmd);
static void ili9488_send_data(void * data, uint16_t length);
static void ili9488_send_color(void * data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void ili9488_init(void) {

  lcd_init_cmd_t ili_init_cmds[]={
		{ILI9488_CMD_POSITIVE_GAMMA_CORRECTION, {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}, 15},
		{ILI9488_CMD_NEGATIVE_GAMMA_CORRECTION, {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F,}, 15},
    {ILI9488_CMD_POWER_CONTROL_1,           {0x17, 0x15}, 2},
    {ILI9488_CMD_POWER_CONTROL_2,           {0x41}, 1},
    {ILI9488_CMD_VCOM_CONTROL_1,            {0x00, 0x12, 0x80}, 3},

    {ILI9488_CMD_MEMORY_ACCESS_CONTROL,     {TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_MX}, 1},			  // Memory Access Control

		{ILI9488_CMD_COLMOD_PIXEL_FORMAT_SET,   {0x06}, 1},			  // Pixel Interface Format 18 bit colour for SPI

		{ILI9488_CMD_INTERFACE_MODE_CONTROL,    {0x00}, 1},       // Interface Mode Control
		{ILI9488_CMD_FRAME_RATE_CONTROL_NORMAL, {0xA0}, 1},       // Frame Rate Control
		{ILI9488_CMD_DISPLAY_INVERSION_CONTROL, {0x02}, 1},       // Display Inversion Control
		{ILI9488_CMD_DISPLAY_FUNCTION_CONTROL,  {0x02, 0x02, 0x3B}, 3},
		{ILI9488_CMD_ENTRY_MODE_SET,            {0xC6}, 1},
    {ILI9488_CMD_ADJUST_CONTROL_3,          {0xA9, 0x51, 0x2C, 0x82}, 4},

		{ILI9488_CMD_SLEEP_OUT,                 {0}, 0x80},
		{ILI9488_CMD_DISPLAY_ON,                {0}, 0x80},
		{0, {0}, 0xff},
	};

  
  // /uint8_t data[4];
	//Initialize non-SPI GPIOs
	gpio_set_direction(ILI9488_DC, GPIO_MODE_OUTPUT);
	gpio_set_direction(ILI9488_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(ILI9488_BCKL, GPIO_MODE_OUTPUT);

	//Reset the display
	gpio_set_level(ILI9488_RST, 0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(ILI9488_RST, 1);
	vTaskDelay(100 / portTICK_RATE_MS);


	printf("ILI9488 initialization.\n");

  ili9488_send_cmd(ILI9488_CMD_SOFTWARE_RESET);  //Exit Sleep
	vTaskDelay(100 / portTICK_RATE_MS);

 	//Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ili9488_send_cmd(ili_init_cmds[cmd].cmd);
		ili9488_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80) {
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		cmd++;
	}
  
	///Enable backlight
	printf("Enable backlight.\n");
	gpio_set_level(ILI9488_BCKL, 1024);
}

void ili9488_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
  uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

  lv_color32_t* buffer_32bit = (lv_color32_t*) color_map;
  lv_color_custom_t* buffer_24bit = (lv_color_custom_t*) color_map;
  for(int x=0; x < size; x++) {
    buffer_24bit[x].red = buffer_32bit[x].ch.blue;
    buffer_24bit[x].green = buffer_32bit[x].ch.green;
    buffer_24bit[x].blue = buffer_32bit[x].ch.red;
  }

  /*Column addresses*/
  uint8_t xb[] = { (uint8_t) (area->x1 >>8) & 0xFF, (uint8_t) (area->x1) & 0xFF, (uint8_t) (area->x2>>8) & 0xFF, (uint8_t) (area->x2) & 0xFF, };
  /*Page addresses*/
  uint8_t yb[] = { (uint8_t) (area->y1>>8) & 0xFF, (uint8_t) (area->y1) & 0xFF, (uint8_t) (area->y2>>8) & 0xFF, (uint8_t) (area->y2) & 0xFF, };

  ili9488_send_cmd(ILI9488_CMD_COLUMN_ADDRESS_SET);
  ili9488_send_data(xb, 4);
  ili9488_send_cmd(ILI9488_CMD_PAGE_ADDRESS_SET);
  ili9488_send_data(yb, 4);

  /*Memory write*/
  ili9488_send_cmd(ILI9488_CMD_MEMORY_WRITE);
  ili9488_send_color((void*)buffer_24bit, size*3);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void ili9488_send_cmd(uint8_t cmd)
{
	gpio_set_level(ILI9488_DC, 0);	 /*Command mode*/
	disp_spi_send_data(&cmd, 1);
}

static void ili9488_send_data(void * data, uint16_t length)
{
	gpio_set_level(ILI9488_DC, 1);	 /*Data mode*/
	disp_spi_send_data(data, length);
}

static void ili9488_send_color(void * data, uint16_t length)
{
    gpio_set_level(ILI9488_DC, 1);   /*Data mode*/
    disp_spi_send_colors(data, length);
}
