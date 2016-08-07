/******************************************************************************

 @file  hal_buzzer.c

 @brief This file contains the interface to control the buzzer.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2006-2016, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: ble_sdk_1.4.2.2
 Release Date: 2016-06-09 06:57:09
 *****************************************************************************/

/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_board.h"
#include "hal_buzzer.h"
#include "hal_drivers.h"
#include "osal.h"

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/

/* Defines for Timer 3 */
#define HAL_T3_CC0_VALUE                125   /* provides pulse width of 125 usec */
#define HAL_T3_TIMER_CTL_DIV1           0x00  /* Clock pre-scaled by 1 */
#define HAL_T3_TIMER_CTL_DIV2           0x20  /* Clock pre-scaled by 2 */
#define HAL_T3_TIMER_CTL_DIV4           0x40  /* Clock pre-scaled by 4 */
#define HAL_T3_TIMER_CTL_DIV8           0x60  /* Clock pre-scaled by 8 */
#define HAL_T3_TIMER_CTL_DIV16          0x80  /* Clock pre-scaled by 16 */
#define HAL_T3_TIMER_CTL_DIV32          0xA0  /* Clock pre-scaled by 32 */
#define HAL_T3_TIMER_CTL_DIV64          0xC0  /* Clock pre-scaled by 64 */
#define HAL_T3_TIMER_CTL_DIV128         0xE0  /* Clock pre-scaled by 128 */
#define HAL_T3_TIMER_CTL_START          0x10
#define HAL_T3_TIMER_CTL_CLEAR          0x04
#define HAL_T3_TIMER_CTL_OPMODE_MODULO  0x02  /* Modulo Mode, Count from 0 to CompareValue */
#define HAL_T3_TIMER_CCTL_MODE_COMPARE  0x04
#define HAL_T3_TIMER_CCTL_CMP_TOGGLE    0x10


/* The following define which port pins are being used by the buzzer */
#define HAL_BUZZER_P1_GPIO_PINS  ( BV( 4 ) )

/* These defines indicate the direction of each pin */
#define HAL_BUZZER_P1_OUTPUT_PINS  ( BV( 4 ) )

/* Defines for each output pin assignment */
#define HAL_BUZZER_ENABLE_PIN  P1_4

#define MELODYMAXLEN 32

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/

#define HAL_BUZZER_DISABLE() ( HAL_BUZZER_ENABLE_PIN = 0 )

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
/* Function to call when ringing of buzzer is complete */
static halBuzzerCBack_t pHalBuzzerRingCompleteNotificationFunction;
static uint8 melodyPtr[MELODYMAXLEN];
static uint8 nowPlaying,melodyLen;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

extern void* memcpy(void *dest, const void *src, size_t len);

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      HalBuzzerInit
 *
 * @brief   Initilize buzzer hardware
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HalBuzzerInit( void )
{
  pHalBuzzerRingCompleteNotificationFunction = NULL;

  /* Initialize outputs */
  HAL_BUZZER_DISABLE();

  /* Configure direction of pins related to buzzer */
  P1DIR |= (uint8) HAL_BUZZER_P1_OUTPUT_PINS;
  
  //Set Timer3
  PERCFG &= ~(1<<5); //T3CFG=0; Alternative 1 location
  P2SEL |= 0|(0x60); //Timer 3 over USART1. P2SEL.PRI2P1 and P2SEL.PRI3P1 both has to be 1
  
  nowPlaying=0;
  melodyLen=0;
}

void HalBuzzerPlay(uint8 *melody,uint8 len,halBuzzerCBack_t buzzerCback){
  /* Register the callback fucntion */
  pHalBuzzerRingCompleteNotificationFunction = buzzerCback;

  if (len>MELODYMAXLEN) len=MELODYMAXLEN;
  memcpy(melodyPtr, melody, len);
  melodyLen=len;
  nowPlaying=0;
  
  HalBuzzerTone();
  (void)osal_set_event(Hal_TaskID, HAL_PWRMGR_HOLD_EVENT);//https://e2e.ti.com/support/wireless_connectivity/bluetooth_low_energy/f/538/t/315110
}


/**************************************************************************************************
 * @fn          HalBuzzerRing
 *
 * @brief       This function rings the buzzer once.
 *
 * input parameters
 *
 * @param       msec - Number of msec to ring the buzzer
 * @param       tone - Type of tone (low or high)
 * @param       buzzerCback - Callback function to call when ringing of buzzer is finished
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
void HalBuzzerRing( uint16 msec,uint8 tone)
{
  if (tone == HAL_BUZZER_TONE_NONE){
    T3CTL = HAL_T3_TIMER_CTL_CLEAR | HAL_T3_TIMER_CTL_OPMODE_MODULO;
    P1SEL &= (uint8) ~HAL_BUZZER_P1_GPIO_PINS;
  }else{
    /* Configure output pin as peripheral since we're using T3 to generate */
    P1SEL |= (uint8) HAL_BUZZER_P1_GPIO_PINS;     //buzzer in P1.4 T3 output1
    uint8 buf_T3CTL,buf_T3CC0;
    buf_T3CTL=HAL_T3_TIMER_CTL_OPMODE_MODULO;
    if (tone>=HAL_BUZZER_TONE_B4&&tone<=HAL_BUZZER_TONE_A5){
      buf_T3CTL|=HAL_T3_TIMER_CTL_DIV128;
    }else if(tone>=HAL_BUZZER_TONE_B5&&tone<=HAL_BUZZER_TONE_A6){
      buf_T3CTL|=HAL_T3_TIMER_CTL_DIV64;
    }else if(tone>=HAL_BUZZER_TONE_B6&&tone<=HAL_BUZZER_TONE_C7){
      buf_T3CTL|=HAL_T3_TIMER_CTL_DIV32;
    }
    switch (tone%7){
      case HAL_BUZZER_TONE_B4%7:
        buf_T3CC0=253-1;break;
      case HAL_BUZZER_TONE_C5%7:
        buf_T3CC0=239-1;break;
      case HAL_BUZZER_TONE_D5%7:
        buf_T3CC0=213-1;break;
      case HAL_BUZZER_TONE_E5%7:
        buf_T3CC0=190-1;break;
      case HAL_BUZZER_TONE_F5%7:
        buf_T3CC0=179-1;break;
      case HAL_BUZZER_TONE_G5%7:
        buf_T3CC0=159-1;break;
      case HAL_BUZZER_TONE_A5%7:
        buf_T3CC0=142-1;break; 
    }
    T3CCTL1 = HAL_T3_TIMER_CCTL_MODE_COMPARE | HAL_T3_TIMER_CCTL_CMP_TOGGLE;
    T3CTL = buf_T3CTL;
    T3CC0 = buf_T3CC0; //Modulo, repeatedly count from 0x00 to !!T3CC0!!
    /* Start it */
    T3CTL |= HAL_T3_TIMER_CTL_START;
  }

  osal_stop_timerEx(Hal_TaskID,HAL_BUZZER_EVENT);
  /* Setup timer that will end the buzzing */
  osal_start_timerEx( Hal_TaskID,
                      HAL_BUZZER_EVENT,
                      msec );
}

void HalBuzzerTone( void ){
  if (nowPlaying<melodyLen){
    HalBuzzerRing(((uint16)(melodyPtr[nowPlaying+1]))*25,melodyPtr[nowPlaying]);
    nowPlaying+=2;
  }else{
    HalBuzzerStop();
  }
}

/**************************************************************************************************
 * @fn      HalBuzzerStop
 *
 * @brief   Halts buzzer
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalBuzzerStop( void )
{
  /* Setting T3CTL to 0 disables it and masks the overflow interrupt */
  T3CTL = 0;

  /* Return output pin to GPIO */
  P1SEL &= (uint8) ~HAL_BUZZER_P1_GPIO_PINS;
  
  osal_stop_timerEx(Hal_TaskID,HAL_BUZZER_EVENT);
  
  (void)osal_set_event(Hal_TaskID, HAL_PWRMGR_CONSERVE_EVENT);//https://e2e.ti.com/support/wireless_connectivity/bluetooth_low_energy/f/538/t/315110
  /* Inform application that buzzer is done */
  if (pHalBuzzerRingCompleteNotificationFunction != NULL)
  {
    pHalBuzzerRingCompleteNotificationFunction();
  }
}

/**************************************************************************************************
**************************************************************************************************/
