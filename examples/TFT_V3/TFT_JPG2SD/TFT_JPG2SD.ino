/*
 * Arduino TFT_shield JPG to SD card write example
 * 
 * Author: R. G. (2022)
 * 
 * Comments:
 * This sketch shut JPG photo from Camera and write to microSD card.
 * 
 * Before Uploading, check the required EKA_TFT_VER_X definition in the file EKATFT.h
 */

#include <EKATFT.h>
#include <util/ekasdc.h>
#include <util/ekacam.h>
#include <util/ekacodec.h>
#include "TCM8210_regs.h"

EKATFT tft(0);
INTRFC ifc;
CAM    cam;
SDC    sdc;
CODEC  codec;

uint16_t  err, counter, ts_x, ts_y;
uint32_t  file_size;
uint16_t  count_photo = 0;
bool      fail_sd = false; // SD fail or disable

#define IMG_VGA       // VGA format image
//#define IMG_QVGA       // QVGA format image

#ifdef IMG_VGA
#define IMG_SizeX    640
#define IMG_SizeY    480
#else // IMG_QVGA
#define IMG_SizeX    320
#define IMG_SizeY    240
#endif

/*************************************************************************************************/

void setup() {

    uint8_t state;

    // initialize the serial port
    Serial.begin(115200);
    Serial.println(F("TFT_shield_Video2SD example!"));

    // initialize the display
    tft.begin(0,-8);

    tft.SetColor(BRIGHTRED);
    tft.ClearDevice();

    delay(100);

    Serial.print(F("FS Init... "));
    state = sdc.FSInit();
    if (state == 0) {
        Serial.println(F("unsuccess."));
        fail_sd = true;
    }
    else             Serial.println(F("success."));

    Serial.print("CamInit");
#ifdef IMG_VGA
    err = cam.CamInit(&TCM8210_VGA[0][0]);
#else // IMG_QVGA
    err = cam.CamInit(&TCM8210_QVGA[0][0]);
#endif

    cam.CamVideoPreview(0, 0, 1, true);

    codec.JPEGInit();
    codec.JPEGSetRegs(IMG_SizeX, IMG_SizeY);
}

void loop()
{
    char  file_ext[5]=".jpg"; char file_name[9]="J";
    char  file_index[3];
    sprintf(file_index, "%d", count_photo);
    strcat(file_name, file_index);
    strcat(file_name, file_ext);

    file_size = codec.JPEGStart();
    if (file_size && fail_sd==false) {
        Serial.print("Write ");
        Serial.print(file_name);
        Serial.print(" file (size): ");
        Serial.print(file_size);
        Serial.print(",");
        Serial.print(count_photo);
        if (sdc.JPEGWriteToSD(file_size, file_name)) Serial.println(" (ok)");
        else                                         Serial.println(" (error)");
        count_photo++;
    }
    delay(1000);
}
