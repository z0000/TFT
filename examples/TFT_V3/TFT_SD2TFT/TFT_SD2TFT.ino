/*
 * Arduino TFT_shield_SD2TFT example
 * 
 * Author: R. G. (2022)
 * 
 * Comments:
 * This sketch read jpeg pictures and rgb video from microSD card and show on TFT display
 * 
 * Before Uploading, check the required EKA_TFT_VER_X definition in the file EKATFT.h
 */

#include <EKATFT.h>                    // Hardware-specific library
#include <util/ekasdc.h>

EKATFT tft(0);
SDC    sdc;

/*******************************************************************************
*
*******************************************************************************/
void  setup(void)
{
    Serial.begin(115200);  // initialize the serial port
    Serial.println(F("TFT_shield_SD2TFT example!"));

    tft.begin(0,-8);       // initialize the display
    tft.SetColor(BLACK);
    tft.ClearDevice();     // Clear screen

    Serial.print(F("FS Init... "));
    uint8_t state = sdc.FSInit();
    if (state == 0)  Serial.println(F("unsuccess."));
    else             Serial.println(F("success."));
    sdc.currentFileType = DEMO_FILE_TYPE_RGB;
}

/*******************************************************************************
*
*******************************************************************************/
void  loop(void)
{
    if(-1 != sdc.FindFirst("*.*", ATTR_ARCHIVE, &sdc.nextFile))
    {
        do
        {
            if (sdc.GetFileType(sdc.nextFile.filename) == DEMO_FILE_TYPE_JPEG)
            {
                if (sdc.currentFileType != DEMO_FILE_TYPE_JPEG)
                {
                    // Set YUV mode to display JPEG
                    tft.SetColor(0x007F); // Black in YUV
                    tft.ClearDevice();
                    tft.SetYUV();         // Switching shows a little green flicker
                    sdc.currentFileType = DEMO_FILE_TYPE_JPEG;
                }
                Serial.print(F("View JPEG image: "));
                Serial.println(sdc.nextFile.filename);

                FSFILE * jpeg_file;
                JPEG_DECODE  jpeg_decode;
                jpeg_file = sdc.FSfopen(sdc.nextFile.filename, "r");
                if (!jpeg_file) { Serial.println(F("Open failed!")); return;}
                jpeg_decode.stream = (void *)jpeg_file;
                sdc.JPEGReadFromSD(&jpeg_decode, 0, 0, GetMaxX()+1, GetMaxY()+1);
                uint8_t err = sdc.FSfclose(jpeg_file);
                if (err) { Serial.println(F("Close failed!")); return;}

                delay(1000);
            }
            else if (sdc.GetFileType(sdc.nextFile.filename) == DEMO_FILE_TYPE_RGB)
            {
                if (sdc.currentFileType != DEMO_FILE_TYPE_RGB)
                {
                    // Set RGB mode to display RGB
                    tft.SetColor(BLACK);  // Black in RGB is dark green in YUV
                    tft.ClearDevice();
                    tft.SetRGB();         // Switching shows a little green flicker
                    sdc.currentFileType = DEMO_FILE_TYPE_RGB;
                }
                Serial.print(F("Play RGB video: "));
                Serial.println(sdc.nextFile.filename);

                FSFILE * pFile = sdc.FSfopen(sdc.nextFile.filename, "rb");
                if (!pFile) { Serial.println(F("Open failed!")); return;}
                tft.SetColor(BLACK); tft.ClearDevice();
                sdc.RGBReadFromSD(pFile, NULL);
                tft.SetColor(BLACK); tft.ClearDevice();
                sdc.FSfclose(pFile);
                delay(100);
            }
        } while(-1 != sdc.FindNext(&sdc.nextFile));
    }
}
