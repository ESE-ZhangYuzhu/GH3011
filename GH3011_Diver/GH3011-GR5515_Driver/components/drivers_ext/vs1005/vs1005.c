/**
 *****************************************************************************************
 *
 * @file vs1005.c
 *
 * @brief vs1005 function Implementation.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "vs1005.h"
#include "vs1005_config.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
const uint16_t g_bit_rate[2][16]=
{
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
    {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};

vs_param_t g_vs_set_param =
{
    .voice_num = 240,
    .bf_limit = 6,
    .bass = 15,
    .tf_limit = 10,
    .treble = 15,
    .effect = 0,
    .save_flag = 0,
};

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
uint16_t vs_read_reg(uint8_t address);
void     vs_write_cmd    (uint8_t address,uint16_t data);
void     vs_write_data    (uint8_t data);

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void vs_init(void)
{
    vs_config_init();
}

void vs_soft_reset(void)
{
    uint8_t retry=0;
    while(vs_read_dq_pin()==BIT_RESET);
    vs_spi_write_byte(0Xff);
    retry=0;
    vs_delay_ms(2);
    
    while(vs_read_reg(SPI_MODE)!=0x0800)
    {
        vs_write_cmd(SPI_MODE,0x0804);
        vs_delay_ms(2);
        if(retry++>20)break;
    }
    
    while(vs_read_dq_pin()==BIT_RESET);
    
    retry=0;
    
    while(vs_read_reg(SPI_CLOCKF)!=0X9800)
    {
        vs_write_cmd(SPI_CLOCKF,0X9800);
        if(retry++>100)break;
    }
    vs_delay_ms(20);
}

uint8_t vs_hd_reset(void)
{
    uint8_t retry=0;
    vs_set_rst_pin(BIT_RESET);
    vs_delay_ms(20);
    vs_set_xdcs_pin(BIT_SET);
    vs_set_xcs_pin(BIT_SET);
    vs_set_rst_pin(BIT_SET);
    while((vs_read_dq_pin()==BIT_RESET)&&(retry<200))
    {
        retry++;
        vs_delay_us(50);
    };
    vs_delay_ms(20);
    if(retry>=200)
    {
        return 1;
    }
    else
    {
        return 0;
    } 
}

void vs_sine_test(void)
{
    vs_hd_reset();
    vs_write_cmd(0x0b,0X2020);
    vs_write_cmd(SPI_MODE,0x0820);
    while(vs_read_dq_pin()==BIT_RESET);

    vs_spi_speed_set(SPI_LOW);
    vs_set_xdcs_pin(BIT_RESET);
    vs_spi_write_byte(0x53);
    vs_spi_write_byte(0xef);
    vs_spi_write_byte(0x6e);
    vs_spi_write_byte(0x24);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_delay_ms(100);
    vs_set_xdcs_pin(BIT_SET);

    vs_set_xdcs_pin(BIT_RESET);
    vs_spi_write_byte(0x45);
    vs_spi_write_byte(0x78);
    vs_spi_write_byte(0x69);
    vs_spi_write_byte(0x74);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_delay_ms(100);
    vs_set_xdcs_pin(BIT_SET);

    vs_set_xdcs_pin(BIT_RESET);
    vs_spi_write_byte(0x53);
    vs_spi_write_byte(0xef);
    vs_spi_write_byte(0x6e);
    vs_spi_write_byte(0x44);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_delay_ms(100);
    vs_set_xdcs_pin(BIT_SET);

    vs_set_xdcs_pin(BIT_RESET);
    vs_spi_write_byte(0x45);
    vs_spi_write_byte(0x78);
    vs_spi_write_byte(0x69);
    vs_spi_write_byte(0x74);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_delay_ms(100);
    vs_set_xdcs_pin(BIT_SET);
}

uint16_t vs_ram_test(void)
{
    vs_hd_reset();
    vs_write_cmd(SPI_MODE,0x0820);
    while (vs_read_dq_pin() == BIT_RESET);
    vs_spi_speed_set(SPI_LOW);
    vs_set_xdcs_pin(BIT_RESET);

    vs_spi_write_byte(0x4d);
    vs_spi_write_byte(0xea);
    vs_spi_write_byte(0x6d);
    vs_spi_write_byte(0x54);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_spi_write_byte(0x00);
    vs_set_xdcs_pin(BIT_SET);
    vs_delay_ms(150);

    return vs_read_reg(SPI_HDAT0);
}

void vs_write_cmd(uint8_t address,uint16_t data)
{
    while(vs_read_dq_pin() == BIT_RESET);
    vs_spi_speed_set(SPI_LOW);
    vs_set_xdcs_pin(BIT_SET);
    vs_set_xcs_pin(BIT_RESET);

    vs_spi_write_byte(VS_WRITE_COMMAND);
    vs_spi_write_byte(address);
    vs_spi_write_byte(data >> 8);
    vs_spi_write_byte(data);
    vs_set_xcs_pin(BIT_SET);

    vs_spi_speed_set(SPI_HIGH);
}

void vs_write_data(uint8_t data)
{
    vs_spi_speed_set(SPI_HIGH);
    vs_set_xdcs_pin(BIT_RESET);
    vs_spi_write_byte(data);
    vs_set_xdcs_pin(BIT_SET);
}

uint16_t vs_read_reg(uint8_t address)
{
    uint16_t temp = 0;
    while(vs_read_dq_pin() == BIT_RESET);
    vs_spi_speed_set(SPI_LOW);
    vs_set_xdcs_pin(BIT_SET);
    vs_set_xcs_pin(BIT_RESET);
    vs_spi_write_byte(VS_READ_COMMAND);
    vs_spi_write_byte(address);
    temp = vs_spi_read_byte();
    temp = temp<<8;
    temp += vs_spi_read_byte();
    vs_set_xcs_pin(BIT_SET);
    vs_spi_speed_set(SPI_HIGH);
    return temp;
}

uint16_t vs_wram_read(uint16_t addr)
{
    uint16_t res;
    vs_write_cmd(SPI_WRAMADDR, addr);
    res = vs_read_reg(SPI_WRAM);
    return res;
}

void vs_set_speed(uint8_t speed)
{
    vs_write_cmd(SPI_WRAMADDR,0X1E04);
    while(vs_read_dq_pin() == BIT_RESET);
    vs_write_cmd(SPI_WRAM,speed);
}

uint16_t vs_get_head_info(void)
{
    uint16_t HEAD0;
    uint16_t HEAD1;
    HEAD0 = vs_read_reg(SPI_HDAT0);
    HEAD1 = vs_read_reg(SPI_HDAT1);
    switch(HEAD1)
    {
        case 0x7665://WAV
        case 0X4D54://MIDI
        case 0X4154://AAC_ADTS
        case 0X4144://AAC_ADIF
        case 0X4D34://AAC_MP4/M4A
        case 0X4F67://OGG
        case 0X574D://WMA
        case 0X664C://FLAC
        {
            HEAD1=HEAD0*2/25;
            if((HEAD1%10)>5)return HEAD1/10+1;
            else return HEAD1/10;
        }
        default://MP3
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03;
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return g_bit_rate[HEAD1][HEAD0>>12];
        }
    }
}

uint32_t vs_get_byte_rate(void)
{
    return vs_wram_read(0X1E05);
}


uint16_t vs_get_end_fill_byte(void)
{
    return vs_wram_read(0X1E06);
}

uint8_t vs_send_music_data(uint8_t *buf,uint8_t len)
{
    if(vs_read_dq_pin()!=BIT_RESET)
    {
        vs_set_xdcs_pin(BIT_RESET);
        vs_spi_write_buffer(buf, len);
        vs_set_xdcs_pin(BIT_SET);
    }
    else
    {
        return 1;
    }
    return 0;
}


void vs_restart_play(void)
{
    uint16_t temp;
    uint16_t i;
    uint8_t n;
    uint8_t vsbuf[32];
    for(n=0;n<32;n++)vsbuf[n]=0;
    temp = vs_read_reg(SPI_MODE);
    temp |= 1<<3;
    temp |= 1<<2;
    vs_write_cmd(SPI_MODE,temp);
    for(i = 0; i < 2048;)
    {
        if(vs_send_music_data(vsbuf,32)==0)
        {
            i += 32;
            temp = vs_read_reg(SPI_MODE);
            if((temp & (1<<3)) == 0)break;
        }
    }
    if(i < 2048)
    {
        temp = vs_get_end_fill_byte()&0xff;
        for(n = 0;n < 32 ;n++)vsbuf[n] = temp;
        for(i = 0; i < 2052;)
        {
            if(vs_send_music_data(vsbuf, 32) == 0)
            {
                i += 32;
            }
        }
    }
    else
    {
        vs_soft_reset();
    } 
    temp = vs_read_reg(SPI_HDAT0);
    temp += vs_read_reg(SPI_HDAT1);
    if(temp)
    {
        vs_hd_reset();
        vs_soft_reset();
    }
}


void vs_reset_decoe_time(void)
{
    vs_write_cmd(SPI_DECODE_TIME,0x0000);
    vs_write_cmd(SPI_DECODE_TIME,0x0000);
}

uint16_t vs_get_decode_time(void)
{
    uint16_t dt = 0;
    dt = vs_read_reg(SPI_DECODE_TIME);
    return dt;
}

void vs_load_patch(uint16_t *patch,uint16_t len)
{
    uint16_t i;
    uint16_t addr, n, val;
    
    for(i = 0;i < len;)
    {
        addr = patch[i++];
        n    = patch[i++];
        if(n & 0x8000U) //RLE run, replicate n samples
        {
            n  &= 0x7FFF;
            val = patch[i++];
            while(n--)vs_write_cmd(addr, val);
        }
        else //copy run, copy n sample
        {
            while(n--)
            {
                val = patch[i++];
                vs_write_cmd(addr, val);
            }
        }
    }
}


void vs_set_voice(uint8_t volx)
{
    uint16_t volt = 0;
    volt = 254 - volx;
    volt <<= 8;
    volt += 254 - volx;
    vs_write_cmd(SPI_VOL, volt);
}


void vs_set_bass(uint8_t bfreq,uint8_t bass,uint8_t tfreq,uint8_t treble)
{
    uint16_t bass_set = 0;
    signed char temp = 0;
    if(treble == 0)
    {
        temp = 0;
    }
    else if(treble > 8)
    {
        temp = treble - 8;
    }
    else
    {
        temp = treble - 9;
    } 
    bass_set = temp&0X0F;
    bass_set <<= 4;
    bass_set += tfreq&0xf;
    bass_set <<= 4;
    bass_set += bass&0xf;
    bass_set <<= 4;
    bass_set += bfreq&0xf;
    vs_write_cmd(SPI_BASS, bass_set);
}


void vs_set_effect(uint8_t eft)
{
    uint16_t temp;
    
    temp = vs_read_reg(SPI_MODE);
    if(eft & 0X01)
    {
        temp |= 1<<4;
    }
    else
    {
        temp &= ~(1<<5);
    }
    if(eft & 0X02)
    {
        temp |= 1<<7;
    }
    else
    {
        temp &= ~(1<<7);
    }
    vs_write_cmd(SPI_MODE, temp);
}

void vs_set_all(void)
{
    vs_set_voice(g_vs_set_param.voice_num);
    vs_set_bass(g_vs_set_param.bf_limit, g_vs_set_param.bass, g_vs_set_param.tf_limit, g_vs_set_param.treble);
    vs_set_effect(g_vs_set_param.effect);
}

uint8_t vs_check_busy(void)
{
    uint8_t state = 1;
    if(vs_read_dq_pin() == BIT_RESET)
    {
        state = 0;
    }
    return state;
}

