#include <lvgl.h>
#include <EKATFT.h>          // Hardware-specific library
#include <SPI.h>             // Include SPI library

EKATFT tft(0);

#include <XPT2046_Touchscreen.h>         // Include Touchscreen library
// Touchscreen: MOSI=11, MISO=12, SCK=13, CS=2
#define CS_PIN   2
XPT2046_Touchscreen ts(CS_PIN, 255);

#define LVGL_TICK_PERIOD 60
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 100 ];

lv_obj_t * slider_label;
static lv_obj_t * meter;

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void profile_create(lv_obj_t * parent);
static void analytics_create(lv_obj_t * parent);
static void shop_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
                                   const char * text3);
static lv_obj_t * create_shop_item(lv_obj_t * parent, const void * img_src, const char * name, const char * category,
                                   const char * price);

static void color_changer_event_cb(lv_event_t * e);
static void color_event_cb(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static void birthday_event_cb(lv_event_t * e);
static void calendar_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void chart_event_cb(lv_event_t * e);
static void shop_chart_event_cb(lv_event_t * e);
static void meter1_indic1_anim_cb(void * var, int32_t v);
static void meter1_indic2_anim_cb(void * var, int32_t v);
static void meter1_indic3_anim_cb(void * var, int32_t v);
static void meter2_timer_cb(lv_timer_t * timer);
static void meter3_anim_cb(void * var, int32_t v);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
static lv_obj_t * calendar;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_obj_t * meter1;
static lv_obj_t * meter2;
static lv_obj_t * meter3;

static lv_obj_t * chart1;
static lv_obj_t * chart2;
static lv_obj_t * chart3;

static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;
static lv_chart_series_t * ser3;
static lv_chart_series_t * ser4;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static uint32_t session_desktop = 1000;
static uint32_t session_tablet = 1000;
static uint32_t session_mobile = 1000;

/**********************
 *      MACROS
 **********************/

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    int32_t x, y;
    /*It's a very slow but simple implementation.
     *`set_pixel` needs to be written by you to a set pixel on the screen*/
    for(y = area->y1; y <= area->y2; y++) {
        tft.PutPixels((y * screenWidth + area->x1) * 2, (uint16_t*)color_p, (area->x2 - area->x1 + 1));
        for(x = area->x1; x <= area->x2; x++) { color_p++; }
    }
    lv_disp_flush_ready( disp );
}
  
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t  x, y;
    SPI.begin();
    TS_Point p;

    bool touched = ts.touched();
    
    if (touched)
    {
        p = ts.getPoint();
            
        Serial.print(F("Pressure = "));
        Serial.print(p.z);
        Serial.print(F(", x = "));
        Serial.print(p.x);
        Serial.print(F(", y = "));
        Serial.print(p.y);
        Serial.println();
        SPI.end();  // Disable SPI for correct work DB2 (SS) pin 

        // Calculate coordinates x, y from code ADC
        if (p.x < 200) p.x = 200;
        if (p.y < 250) p.y = 250;

        // Variant 1 (X)
        x = (uint16_t)(320L - ((uint32_t)p.x - 200L)*10L/115L);
        // Variant 2 (X)
        //x = (uint16_t)(0 + ((uint32_t)p.x - 200L)*10L/115L);
        // Variant 1 (Y)
        //y = (uint16_t)(((uint32_t)p.y - 250L)/15L);
        // Variant 2 (Y)
        y = (uint16_t)(240L - ((uint32_t)p.y - 250L)/15L);
    }
    SPI.end();  // Disable SPI for correct work DB2 (SS) pin 

    data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 

    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->point.x = x;
    data->point.y = y;
  
    if (touched)
    {
      Serial.print("Data x");
      Serial.println(x);
      
      Serial.print("Data y");
      Serial.println(y);
    }
}
