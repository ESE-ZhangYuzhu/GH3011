/*
 * @Author: yz.zhang[zhang.yz2@yuyue.com.cn]
 * @Date: 2021-07-22 11:37:55
 * @LastEditors: yz.zhang
 * @LastEditTime: 2021-08-02 19:02:55
 * @FilePath: \STM32F10x_FWLIB_Template\User\TypeDef.h.txt
 * @Description: Personal settings
 */

#ifndef __TYPEDEF_H
#define __TYPEDEF_H

/*!< Bit types */
typedef struct
{
  unsigned char b0 : 1;
  unsigned char b1 : 1;
  unsigned char b2 : 1;
  unsigned char b3 : 1;
  unsigned char b4 : 1;
  unsigned char b5 : 1;
  unsigned char b6 : 1;
  unsigned char b7 : 1;
  unsigned char b8 : 1;
  unsigned char b9 : 1;
  unsigned char bA : 1;
  unsigned char bB : 1;
  unsigned char bC : 1;
  unsigned char bD : 1;
  unsigned char bE : 1;
  unsigned char bF : 1;
} _16_Bits;

typedef union
{
  _16_Bits Bits;
  unsigned char Byte[2];
  unsigned short int Word;
} Word_Type;

typedef struct
{
  unsigned char b0 : 1;
  unsigned char b1 : 1;
  unsigned char b2 : 1;
  unsigned char b3 : 1;
  unsigned char b4 : 1;
  unsigned char b5 : 1;
  unsigned char b6 : 1;
  unsigned char b7 : 1;
} _8_Bits;

typedef union
{
  _8_Bits Bits;
  unsigned char Byte;
} Byte_Type;
/*!< Bit types */

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0

#define VALID 1
#define INVALID 0

#endif
