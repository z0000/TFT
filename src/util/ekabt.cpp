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
#ifdef __AVR__
    #include <avr/pgmspace.h>
#endif // __AVR__
#include "../EKATFT.h"
#include "BluetoothSerial.h"
#include "ekabt.h"

uint8_t  BT::BTFile2JPEGFIFO(uint32_t fifoAddress, uint32_t byteSizev, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt) {
//Serial.println("BTFile2JPEGFIFO:");
    static bool frame = false;
    static bool ff = false;
    static bool d9 = false;
    static bool d8 = false;
    uint16_t shift_word = 0;
    uint16_t shift_prev = 0;
    uint16_t len = 0;
    static bool odd = false;
    uint16_t cnt = 1;
    static bool send_data = false;
    static bool send_end = false;
    SetAddress(fifoAddress);

//#define BUFF_MAX  5000
//uint8_t buff[BUFF_MAX];
//uint32_t n_wr = 0;
//uint32_t n_rd = 0;

    while(1) {
        uint16_t d;
        uint16_t d_odd;
        uint16_t sd;
        uint16_t temp;

        if (serial_bt->available()>0) {
//        if ((*n_wr) != (*n_rd)) {
            temp=serial_bt->read();
//            temp=buff[*n_rd];
//            (*n_rd)++;
//            if ((*n_rd) >= size_max) (*n_rd) = 0;    

//Serial.print(n_rd,DEC);
//Serial.print(":");
//  temp=buff[n_rd++];
//  if (n_rd >= BUFF_MAX) n_rd = 0;    
//Serial.println(temp, HEX);

            cnt++;
            if (cnt&1) { d |= temp; if (frame==true) send_data = true; }
            else       d = temp<<8;
            if (temp==0xFF) ff = true;
            else {
                if (ff==true) {
                    if (temp==0xD8) { frame = true; cnt = 1; d = 0xFFD8; send_data = true;}
                    if (temp==0xD9) {
                        frame = false;
                        if (cnt&1) { send_data = true;}
                        else { d = 0xD900; cnt++; send_data = true;}
                        send_end = true;
                    }
                    ff = false;
                }
            }
            if (send_data == true) {
                WriteData(d);
                send_data = false;
            }
            if (send_end==true) {
                for (uint16_t i=0; i<=(JPEG_FIFO_SIZE-cnt)/2; i++) WriteData(0);
                send_end = false;
Serial.println("break");
                break;
            }
        }
    }
Serial.println("return");
    return (TRUE);
}

JPEG_ERR  BT::JPEGReadFromBT(JPEG_DECODE * jpeg_decode, uint16_t left, uint16_t top, uint16_t right, uint16_t bot, uint32_t size, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt)
{
//Serial.println("JPEGReadFromBT:");
    JPEG_ERR err=BTJPEGRegsSetup(jpeg_decode, size); err=BTJPEGHeader(jpeg_decode, n_wr, n_rd, buff, serial_bt);
    if (err == NO_ERR) {
        BTJPEGResize(jpeg_decode, left, top, right, bot);
        wrReg8(0x382,7); wrReg8(0x383,0x30); wrReg8(0x38A,1);
        err = BTJPEGData(jpeg_decode, n_wr, n_rd, buff, serial_bt);
        wrReg8(0x380,0x10); wrReg8(0x38A,0);
    }
    return err;
}

JPEG_ERR  BT::BTJPEGRegsSetup(JPEG_DECODE * decode, uint32_t size) {
//Serial.println("BTJPEGRegsSetup:");
    uint32_t  sz; JPEG_ERR err = NO_ERR; decode->bytes_read = 0; wrReg8(0x41E, 0);
    wrReg8(0x3A4, (uint8_t) (JPEG_FIFO_BLK_NUMBER - 1)); decode->fifo_addr = JPEG_FIFO_START_ADDR;
    decode->fifo_addr_end = JPEG_FIFO_START_ADDR + (JPEG_FIFO_BLK_NUMBER * JPEG_FIFO_BLK_SIZE);
    sz = JPEG_FIFO_START_ADDR/4; wrReg32(0x414, sz); wrReg8(0x380, 0x11);
    wrReg8(0x402, 0x80); delay(1); wrReg8(0x402, 0); wrReg8(0x41C, 2); wrReg8(0x41E, 0);
//!    if (err)  return err;
    decode->size = size; wrReg32(0x3B8, size);
    wrReg8(0x400, 4); wrReg8(0x360, 1); wrReg8(0x402, 1);
    return err;
}

JPEG_ERR  BT::BTJPEGHeader(JPEG_DECODE * decode, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt) {
//Serial.println("BTJPEGHeader:");
    uint8_t  cnt = 0;
    uint8_t  cnt2 = 0;
    
//    while (cnt++ < 250)
    while (cnt++ < 250)
    {
        if (rdReg8(0x41E)) {
Serial.println("0x41E");
//!!! Закомментировал чтобы попробовать игнорировать ошибку 
            if(cnt2++ > 10)
                return (ERR_DECODE);
        }
        if (rdReg8(0x385) & 0x10) {
            uint16_t   size;
            size = (uint16_t)rdReg8(0x3D8)|((uint16_t)(rdReg8(0x3D8+1)))<<8;
            decode->image_width=size;
            size = (uint16_t)rdReg8(0x3DC)|((uint16_t)(rdReg8(0x3DC+1)))<<8;
            decode->image_height=size; decode->op_mode=rdReg8(0x401);
            return (NO_ERR);
        }
        if (!(rdReg8(0x384) & 0x01)) continue;
        if (decode->bytes_read < decode->size) {
            if (!BTFile2JPEGFIFO(JPEG_FIFO_START_ADDR, JPEG_FIFO_SIZE, n_wr, n_rd, buff, serial_bt)) { }
            decode->bytes_read+=JPEG_FIFO_SIZE;
        } else return (ERR_NO_DATA);
    }
Serial.println("BTJPEGHeader:");
    return (ERR_DECODE);
}

void  BT::BTJPEGResize(JPEG_DECODE * decode, uint16_t left, uint16_t top, uint16_t right, uint16_t bot) {
//Serial.println("BTJPEGResize:");
    const uint32_t __align[] = { 8, 16, 16, 32}; uint16_t size; uint8_t div = 1;
    uint16_t _maxX=right-left, _maxY=bot-top;
    if (right<left) _maxX=left-right; if (bot<top) _maxY=top-bot;
    if ((decode->image_height>(_maxY))||(decode->image_width>(_maxX))) {
        div = 2;
        while (div<8) {
            if (((decode->image_height/div)<=_maxY)&&((decode->image_width/div)<=_maxX)) break;
            div<<=1;
        }
    }
    wrReg8(0x36C,div); wrReg8(0x36E,2); decode->display_height=div*(GetMaxY()+1);
    decode->display_width=div*(GetMaxX()+1); uint32_t destAddr; uint32_t x, y;
    x=(right+left-decode->image_width/div)/2; y=(bot+top-decode->image_height/div)/2;
    destAddr = ((y*(GetMaxX()+1)+x)>>1); destAddr &= ~0x0001; wrReg32(0x410, destAddr);
    decode->display_height=decode->display_height&(~(__align[decode->op_mode]-1));
    decode->display_width =decode->display_width &(~(__align[decode->op_mode]-1));
    size = decode->display_width-1; wrReg16(0x364,0); wrReg16(0x368,size);
    size = decode->display_height-1; wrReg16(0x366,0); wrReg16(0x36A,size);
}

JPEG_ERR  BT::BTJPEGData(JPEG_DECODE * decode, uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, BluetoothSerial *serial_bt) {
//Serial.println("BTJPEGData:");
    uint8_t  cnt = 0;

    while (1) {
        if (rdReg8(0x41E)) {
Serial.println("BTJPEGData:a");
//!!! Закомментировал чтобы попробовать игнорировать ошибку
            if(cnt++ > 10)
                return (ERR_DECODE);
        }
        if (rdReg8(0x404)&0xF8) {
Serial.println("BTJPEGData:b");
//!!! Закомментировал чтобы попробовать игнорировать ошибку 
            if(cnt++ > 10)
                return (ERR_DECODE);
        }
        if (rdReg8(0x385)==0x22) return (NO_ERR);
        if (!(rdReg8(0x384)&0x01)) continue;
        if (decode->bytes_read<decode->size) {
            if(!BTFile2JPEGFIFO(JPEG_FIFO_START_ADDR, JPEG_FIFO_SIZE, n_wr, n_rd, buff, serial_bt)) { }
            decode->bytes_read += JPEG_FIFO_SIZE;
        } else  continue;
    }
}

uint8_t  BT::BTParseStream(uint32_t *n_wr, uint32_t *n_rd, uint8_t *buff, uint32_t size_max, BluetoothSerial *serial_bt) {
//Serial.println("BTParseStream:");
    uint8_t ack = 0;

    time = millis();

    while (serial_bt->available()) {
//Serial.println(".");
        uint8_t r = serial_bt->read();
        if(!busy) {
            buff[(*n_wr)] = r;
        
//        if(cur_reg==0xD9 && prev_reg==0xFF)
//            ack = 1;
        
            (*n_wr)++;
            if ((*n_wr) >= size_max) (*n_wr) = 0;
//        prev_reg = cur_reg;
        }
        time_last = time;
    }

    if ((time_last + 5000) < time) {
        if(busy) {
            busy = false;
        }
 Serial.println("busy: ");
// Serial.println(buff[0], HEX);
        (*n_wr) = 0;
        (*n_rd) = 0;
        buff[0] = 0;
        ack = 0;
        //serial_bt->flush();
        time_last = time;
    }

    if(buff[(*n_wr)-1]==0xD9 && buff[(*n_wr)-2]==0xFF) {// && buff[0]==0x30) {
        ack = 0x30;
        busy = true;
 Serial.print("ack30: ");
 Serial.println((*n_wr),DEC);
    }
    if(buff[(*n_wr)-1]==0xD9 && buff[(*n_wr)-2]==0xFF && buff[0]==0x40) {
        ack = 0x40;
        busy = true;
 Serial.print("ack40: ");
 Serial.println((*n_wr),DEC);
    }

    //if ((*n_wr) != (*n_rd) && ack == 0x30) {
if (ack == 0x31) {
        for(uint32_t i = 0; i < (*n_wr); i++) {
            if (i%50==0) {
                Serial.println();
                Serial.print(i,DEC);
                Serial.print(":");
            }
            uint8_t temp=buff[i];
            Serial.print(temp, HEX);
            Serial.print(",");
delayMicroseconds(50);
        }
        Serial.println();
    }

    return ack;
}
