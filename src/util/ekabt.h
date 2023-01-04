/******************************************************************************
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL AUTHOR OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
*******************************************************************************/
#ifndef _EKABT_H_
#define _EKABT_H_

/************************************************************
 * Defines
 ************************************************************/
// JPEG FIFO Size and Location
#define JPEG_FIFO_BLK_SIZE      (uint32_t) (4 * 1024)
#define JPEG_FIFO_BLK_NUMBER    (uint32_t) 8 //16
#define JPEG_FIFO_SIZE          JPEG_FIFO_BLK_SIZE * JPEG_FIFO_BLK_NUMBER
#define JPEG_FIFO_START_ADDR    (uint32_t) (0x30000)

#ifndef _JPEG_DECODE_
#define _JPEG_DECODE_
typedef struct _JPEG_DECODE {
    void *stream; uint16_t image_width; uint16_t image_height; uint32_t display_width; uint32_t display_height;
    uint16_t op_mode; uint32_t fifo_addr; uint32_t fifo_addr_end; uint32_t size; uint32_t bytes_read;
} JPEG_DECODE;
#endif

#ifndef _JPEG_ERR_
#define _JPEG_ERR_	
typedef enum _JPEG_ERR {NO_ERR,ERR_TIMEOUT,ERR_IMCOMPLETE,ERR_NO_DATA,ERR_DECODE,ERR_CODEC_INT} JPEG_ERR;
#endif

/*********************************************************************
* Class BT (Bluetooth)
*********************************************************************/
class  BT:INTRFC {
  public:
    uint8_t     SDSectorRead(uint32_t sector_addr, uint8_t *buffer);
    //uint8_t     BTFile2JPEGFIFO(uint32_t fifoAddress, uint32_t byteSize);
    uint8_t     BTFile2JPEGFIFO(uint32_t fifoAddress, uint32_t byteSize, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt);
    JPEG_ERR    JPEGReadFromBT(JPEG_DECODE * jpeg_decode, uint16_t left, uint16_t top, uint16_t right, uint16_t bot, uint32_t size, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt);
    uint8_t     BTParseStream(uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, uint32_t size_max, BluetoothSerial *serial_bt);

  private:
    JPEG_ERR    BTJPEGRegsSetup(JPEG_DECODE * decode, uint32_t size);
    JPEG_ERR    BTJPEGHeader(JPEG_DECODE * decode, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt);
    void        BTJPEGResize(JPEG_DECODE * decode, uint16_t left, uint16_t top, uint16_t right, uint16_t bot);
    JPEG_ERR    BTJPEGData(JPEG_DECODE * decode, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt);
    uint8_t     prev_reg;
    uint8_t     cur_reg;
    uint32_t    time_last;
    uint32_t    time;
    bool        busy = false;
};

#endif // _EKABT_H_
