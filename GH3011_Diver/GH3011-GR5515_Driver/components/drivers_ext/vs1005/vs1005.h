/**
 *****************************************************************************************
 *
 * @file vs1005.h
 *
 * @brief Header file - User Function
 *
 *****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */
#ifndef _VS1005_H__
#define _VS1005_H__

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <stdint.h>

/*
 * DEFINES
 *****************************************************************************************
 */
typedef struct
{
    uint8_t voice_num;      // voice(0-254)
    uint8_t bf_limit;       //low fre (2-15)(10hz)
    uint8_t bass;
    uint8_t tf_limit;
    uint8_t treble;
    uint8_t effect;

    uint8_t save_flag;
}vs_param_t;

#define VS_WRITE_COMMAND    0x02
#define VS_READ_COMMAND     0x03
#define SPI_MODE            0x00
#define SPI_STATUS          0x01
#define SPI_BASS            0x02
#define SPI_CLOCKF          0x03
#define SPI_DECODE_TIME     0x04
#define SPI_AUDATA          0x05
#define SPI_WRAM            0x06
#define SPI_WRAMADDR        0x07
#define SPI_HDAT0           0x08
#define SPI_HDAT1           0x09
#define SPI_AIADDR          0x0a
#define SPI_VOL             0x0b
#define SPI_AICTRL0         0x0c
#define SPI_AICTRL1         0x0d
#define SPI_AICTRL2         0x0e
#define SPI_AICTRL3         0x0f
#define SM_DIFF             0x01
#define SM_JUMP             0x02
#define SM_RESET            0x04
#define SM_OUTOFWAV         0x08
#define SM_PDOWN            0x10
#define SM_TESTS            0x20
#define SM_STREAM           0x40
#define SM_PLUSV            0x80
#define SM_DACT             0x100
#define SM_SDIORD           0x200
#define SM_SDISHARE         0x400
#define SM_SDINEW           0x800
#define SM_ADPCM            0x1000
#define SM_ADPCM_HP         0x2000

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
void     vs_init(void);
void     vs_soft_reset(void);
uint8_t  vs_hd_reset(void);
void     vs_sine_test(void);
uint16_t vs_ram_test(void);
void     vs_write_cmd(uint8_t address,uint16_t data);
void     vs_write_data(uint8_t data);
uint16_t vs_read_reg(uint8_t address);
uint16_t vs_wram_read(uint16_t addr);
void     vs_set_speed(uint8_t speed);
uint16_t vs_get_head_info(void);
uint32_t vs_get_byte_rate(void);
uint16_t vs_get_end_fill_byte(void);
uint8_t  vs_send_music_data(uint8_t *buf,uint8_t len);
void     vs_restart_play(void);
void     vs_reset_decoe_time(void);
uint16_t vs_get_decode_time(void);
void     vs_load_patch(uint16_t *patch,uint16_t len);
void     vs_set_voice(uint8_t volx);
void     vs_set_bass(uint8_t bfreq,uint8_t bass,uint8_t tfreq,uint8_t treble);
void     vs_set_effect(uint8_t eft);
void     vs_set_all(void);
uint8_t  vs_check_busy(void);

#endif

