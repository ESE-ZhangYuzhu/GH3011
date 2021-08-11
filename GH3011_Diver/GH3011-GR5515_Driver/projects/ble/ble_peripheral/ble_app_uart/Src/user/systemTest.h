/*
******************************************************************************
* @file    system_test.h
* @author  Goodix
* @version V1.0.0
* @date    2019-05-22
* @brief   System test
******************************************************************************
* @attention
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEMTEST_H
#define __SYSTEMTEST_H

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef unsigned char  (* Write_fun)(unsigned short reg,unsigned short value);
typedef unsigned short (* Read_fun)(unsigned short reg);

typedef struct 
{
	short int nLpNoise;//set noise Level
	int nNoiseRawDataMax;//set rawdata min
	int nNoiseRawDataMin;//set rawdata max
	int nNoiseRawDataAvr;
}ROMAHBData;

typedef struct
{
	Write_fun WR_Fun;
	Read_fun RD_Fun;
}ROMAHBD_Interfcae;

/**
  * @function		OTP_Test
  * @description	check OTPdata crc function.
  * @input   	    OTP reg wr rd fun.
  * @output		    check result.
  * @return  		0:ok   1:err  2:parma err
  * @note
  */
extern unsigned char OTP_Check( ROMAHBD_Interfcae *ROMAHBD_Str,unsigned char *retbuf );

/**
  * @function		Check_Rawdata_Noise
  * @description	check Rawdata and noise function.
  * @input   	    Rawdata and check thold value.
  * @output		    check result.
* @return  		    0:ok 1:rawdata err 2:noise err 3:para err
  * @note
  */
extern unsigned char Check_Rawdata_Noise( int *_inDatabuff,const unsigned char lenth,const ROMAHBData *_inRawdataStr );

#endif

