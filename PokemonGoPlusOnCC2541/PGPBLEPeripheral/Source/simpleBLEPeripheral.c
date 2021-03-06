/******************************************************************************

 @file  simpleBLEPeripheral.c

 @brief This file contains the Simple BLE Peripheral sample application for use
        with the CC2540 Bluetooth Low Energy Protocol Stack.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2010-2016, Texas Instruments Incorporated
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
 PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "hal_buzzer.h"

#if (defined HAL_UART) && (HAL_UART == TRUE)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "pgpDeviceControl.h"
#include "pgpCertificate.h"

#include "peripheral.h"

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"
#include "battservice.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   0

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
// We don't want to advertise too long, we want it to enter sleep

#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
//#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL


// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15
   
// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL           6 

// Battery measurement period in ms
#define DEFAULT_BATT_PERIOD                   15000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name ,Pokemon GO Plus
  0x10,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x50, 0x6F, 0x6B, 0x65, 0x6D, 0x6F, 0x6E, 0x20, 0x47, 0x4F, 0x20, 0x50, 0x6C, 0x75, 0x73,

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( DEVICE_CONTROL_SERV_UUID ), //need to update
  HI_UINT16( DEVICE_CONTROL_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Pokemon GO Plus";

typedef enum  
{  
  PAIRSTATUS_PAIRED = 0,  
  PAIRSTATUS_NO_PAIRED,  
}PAIRSTATUS;  
static PAIRSTATUS gPairStatus = PAIRSTATUS_NO_PAIRED;//PAIRSTATUS��not paired by default 

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void simpleBLEPeripheral_ProcessGATTMsg( gattMsgEvent_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void pgpDeviceControlChangeCB( uint8 paramID );
static void pgpCertificateChangeCB( uint8 paramID );
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
static void pokemonGoPlusBattPeriodicTask( void );
static void pokemonGoPlusBattCB(uint8 event);
static void simpleBLEPeripheralBuzzerRing(uint8 *melody,uint8 len);
static void simpleBLEPeripheralBuzzerCompleteCback( void );

void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs );
static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  ProcessPasscodeCB,                     // Passcode callback 
  ProcessPairStateCB                      // Pairing / Bonding state Callback 
};

// Simple GATT Profile Callbacks
static pgpDeviceControlCBs_t simpleBLEPeripheral_PgpDeviceControlCBs =
{
  pgpDeviceControlChangeCB    // Charactersitic value change callback
};
static pgpCertificateCBs_t simpleBLEPeripheral_PgpCertificateCBs =
{
  pgpCertificateChangeCB    // Charactersitic value change callback
};
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
    uint16 tgap_LimitAdvertTimeout = 30/1;        //unit in seconds

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
    
    GAP_SetParamValue( TGAP_LIM_ADV_TIMEOUT, tgap_LimitAdvertTimeout ); //set timeout time

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    //uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 pairMode = GAPBOND_PAIRING_MODE_INITIATE;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }
  
  // Setup Battery Characteristic Values
  {
    uint8 critical = DEFAULT_BATT_CRITICAL_LEVEL;
    Batt_SetParameter( BATT_PARAM_CRITICAL_LEVEL, sizeof (uint8 ), &critical );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  PgpDeviceControl_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
  PgpCertificate_AddService( GATT_ALL_SERVICES );    // Simple GATT Profile
  Batt_AddService( );
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the PgpDeviceControl Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    PgpDeviceControl_SetParameter( LED_VIBRATE_CTRL_CHAR, sizeof ( uint8 ), &charValue1 );
    PgpDeviceControl_SetParameter( BUTTON_NOTIF_CHAR, sizeof ( uint8 ), &charValue2 );
    PgpDeviceControl_SetParameter( FW_UPDATE_REQUEST_CHAR, sizeof ( uint8 ), &charValue3 );
    PgpDeviceControl_SetParameter( FW_VERSION_CHAR, sizeof ( uint8 ), &charValue4 );
  }
  // Setup the PgpCertificate Characteristic Values
  {
    uint8 charValue1 = 5;
    uint8 charValue2 = 6;
    uint8 charValue3 = 7;
    PgpCertificate_SetParameter( CENTRAL_TO_SFIDA_CHAR, sizeof ( uint8 ), &charValue1 );
    PgpCertificate_SetParameter( SFIDA_COMMANDS_CHAR, sizeof ( uint8 ), &charValue2 );
    PgpCertificate_SetParameter( SFIDA_TO_CENTRAL_CHAR, sizeof ( uint8 ), &charValue3 );
  }
  
  
  HalLedSet( (HAL_LED_1_RED | HAL_LED_2_BLUE), HAL_LED_MODE_OFF );
  
  HalLedSet( (HAL_LED_1_RED ), HAL_LED_MODE_OFF );
  //HalLedSet( (HAL_LED_2_BLUE ), HAL_LED_MODE_ON );
  HalLedSet( (HAL_LED_3_GREEN ), HAL_LED_MODE_ON );
  
  RegisterForKeys( simpleBLEPeripheral_TaskID );
  
  P0SEL = 0; // Configure Port 0 as GPIO
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    P1SEL = 0|BV(6)|BV(7); // Configure Port 1 as GPIO , P1.6,P1.7 as UART 
  #else
    P1SEL = 0;
  #endif
  
  //But if DMA_PM=1 (When power saving), CTS is used. On test board there is a jumper
  P2SEL = 0|(0x60); // Configure Port 2 as GPIO, Give USART1 Priority over USART0, Timer 3 over USART1.
  
  P0DIR|=BV(0)|BV(2)|BV(3)|BV(4)|BV(5)|BV(6)|BV(7);    //OUTPUT GND to minimize power
  P0&=~(BV(0)|BV(2)|BV(3)|BV(4)|BV(5)|BV(6)|BV(7));

  P1DIR|=BV(2)|BV(3)|BV(4)|BV(5)|BV(6)|BV(7);    //OUTPUT GND to minimize power
  P1&=~(BV(2)|BV(3)|BV(4)|BV(5)|BV(6)|BV(7));

  // Register callback with SimpleGATTprofile
  VOID PgpDeviceControl_RegisterAppCBs( &simpleBLEPeripheral_PgpDeviceControlCBs );
  VOID PgpCertificate_RegisterAppCBs( &simpleBLEPeripheral_PgpCertificateCBs );
  
  // Register for Battery service callback;
  Batt_Register ( pokemonGoPlusBattCB );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

  // Setup a delayed profile startup
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );

}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
    {
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );

    // Set timer for first periodic event
    if ( SBP_PERIODIC_EVT_PERIOD ) osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );

    return ( events ^ SBP_START_DEVICE_EVT );
  }

  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    }

    // Perform periodic application task
    performPeriodicTask();

    return (events ^ SBP_PERIODIC_EVT);
  }
  
  if ( events & BATT_PERIODIC_EVT )
  {
    // Perform periodic battery task
    pokemonGoPlusBattPeriodicTask();
    
    return (events ^ BATT_PERIODIC_EVT);
  } 

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {     

    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, 
                                      ((keyChange_t *)pMsg)->keys );
      break;
 
    case GATT_MSG_EVENT:
      // Process GATT message
      simpleBLEPeripheral_ProcessGATTMsg( (gattMsgEvent_t *)pMsg );
      break;
      
    default:
      // do nothing
      break;
  }
}


/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  static uint8 handleKeysSavedKeys=0;
  static uint32 keyDownTime=0;

  VOID shift;  // Intentionally unreferenced parameter

  if ((keys & HAL_PUSH_BUTTON)!=(handleKeysSavedKeys & HAL_PUSH_BUTTON)){
    if ( keys & HAL_PUSH_BUTTON ){
      keyDownTime = osal_GetSystemClock();
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        HalUARTWrite ( HAL_UART_PORT_1, "KEY down\n", 9 );
      #endif
    }else{
      uint16 keyPressDuration=osal_GetSystemClock()-keyDownTime;
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        HalUARTWrite ( HAL_UART_PORT_1, "KEY UP\n", 7 );
        {
          char buf[16];
          sprintf(buf,"%d\n",keyPressDuration);
          uint8 strLength=strlen(buf);
          HalUARTWrite ( HAL_UART_PORT_1, (uint8 *)buf, strLength );
        }
      #endif
      if (keyPressDuration<300){
        uint8 buttonValue=0x0F;
        PgpDeviceControl_SetParameter( BUTTON_NOTIF_CHAR, sizeof ( uint8 ), &buttonValue );
 
        uint8 data[]={3,0,0,0};
        PgpCertificate_SetParameter(SFIDA_COMMANDS_CHAR, 4, data);  
          

      }
      if (keyPressDuration>2500){     //long press
        uint8 current_adv_enabled_status;
        GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );
        if (!current_adv_enabled_status){
          uint8 new_adv_enabled_status=true;
          GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
        }
      }
      { //test buzzer
        //tone,duration(25ms).....
        uint8 melody[]={
          HAL_BUZZER_TONE_C6,5,HAL_BUZZER_TONE_D6,5,HAL_BUZZER_TONE_E6,5,HAL_BUZZER_TONE_F6,5,HAL_BUZZER_TONE_G6,5,
          HAL_BUZZER_TONE_A6,5,HAL_BUZZER_TONE_B6,5,HAL_BUZZER_TONE_C7,5
        };
        simpleBLEPeripheralBuzzerRing( melody, 16 );
      }
    }
  }
  
  
  handleKeysSavedKeys=keys;
/*
  if ( keys & HAL_KEY_SW_2 )
  {

    SK_Keys |= SK_KEY_RIGHT;

    // if device is not in a connection, pressing the right key should toggle
    // advertising on and off
    // Note:  If PLUS_BROADCASTER is define this condition is ignored and
    //        Device may advertise during connections as well. 
#ifndef PLUS_BROADCASTER  
    if( gapProfileState != GAPROLE_CONNECTED )
    {
#endif // PLUS_BROADCASTER
      uint8 current_adv_enabled_status;
      uint8 new_adv_enabled_status;

      //Find the current GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &current_adv_enabled_status );

      if( current_adv_enabled_status == FALSE )
      {
        new_adv_enabled_status = TRUE;
      }
      else
      {
        new_adv_enabled_status = FALSE;
      }

      //change the GAP advertisement status to opposite of current status
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
#ifndef PLUS_BROADCASTER
    }
#endif // PLUS_BROADCASTER
  }
*/
  // Set the value of the keys state to the Simple Keys Profile;
  // This will send out a notification of the keys state if enabled
  //SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}


/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessGATTMsg( gattMsgEvent_t *pMsg )
{  
  GATT_bm_free( &pMsg->msg, pMsg->method );
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
#ifdef PLUS_BROADCASTER
  static uint8 first_conn_flag = 0;
#endif // PLUS_BROADCASTER
  
  
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

      }
      break;

    case GAPROLE_ADVERTISING:
      {
        //Called when ADVERTISING Starts
        HalLedBlink( HAL_LED_2_BLUE, 0, 5, 2000 );
        #if (defined HAL_UART) && (HAL_UART == TRUE)
          HalUARTWrite ( HAL_UART_PORT_1, "GAPROLE_ADVERTISING\n", 20 );
        #endif
      }
      break;

#ifdef PLUS_BROADCASTER   
    /* After a connection is dropped a device in PLUS_BROADCASTER will continue
     * sending non-connectable advertisements and shall sending this change of 
     * state to the application.  These are then disabled here so that sending 
     * connectable advertisements can resume.
     */
    case GAPROLE_ADVERTISING_NONCONN:
      {
        uint8 advertEnabled = FALSE;
      
        // Disable non-connectable advertising.
        GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8),
                           &advertEnabled);
        
        // Reset flag for next connection.
        first_conn_flag = 0;
      }
      break;
#endif //PLUS_BROADCASTER         
      
    case GAPROLE_CONNECTED:
      {        
        HalLedBlink( HAL_LED_2_BLUE, 0, 2, 5000 );
        #if (defined HAL_UART) && (HAL_UART == TRUE)
          HalUARTWrite ( HAL_UART_PORT_1, "GAPROLE_CONNECTED\n", 18 );
        #endif
          
#ifdef PLUS_BROADCASTER
        // Only turn advertising on for this state when we first connect
        // otherwise, when we go from connected_advertising back to this state
        // we will be turning advertising back on.
        if ( first_conn_flag == 0 ) 
        {
            uint8 advertEnabled = FALSE; // Turn on Advertising

            // Disable connectable advertising.
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8),
                                 &advertEnabled);
            
            // Set to true for non-connectabel advertising.
            advertEnabled = TRUE;
            
            // Enable non-connectable advertising.
            GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8),
                                 &advertEnabled);
            
            first_conn_flag = 1;
        }
#endif // PLUS_BROADCASTER
      }
      break;

    case GAPROLE_CONNECTED_ADV:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          //HalLcdWriteString( "Connected Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;      
    case GAPROLE_WAITING:
      {
        //Called when ADVERTISING Ends
        HalLedSet(HAL_LED_2_BLUE, HAL_LED_MODE_OFF );
        #if (defined HAL_UART) && (HAL_UART == TRUE)
          HalUARTWrite ( HAL_UART_PORT_1, "GAPROLE_WAITING\n", 16 );
        #endif
          
#ifdef PLUS_BROADCASTER                
        uint8 advertEnabled = TRUE;
      
        // Enabled connectable advertising.
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8),
                             &advertEnabled);
#endif //PLUS_BROADCASTER
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          //HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
#ifdef PLUS_BROADCASTER
        // Reset flag for next connection.
        first_conn_flag = 0;
#endif //#ifdef (PLUS_BROADCASTER)
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          //HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          //HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}

/*********************************************************************
 * @fn      pokemonGoPlusBattCB
 *
 * @brief   Callback function for battery service.
 *
 * @param   event - service event
 *
 * @return  none
 */
static void pokemonGoPlusBattCB(uint8 event)
{
  if (event == BATT_LEVEL_NOTI_ENABLED)
  {
    // if connected start periodic measurement
    if (gapProfileState == GAPROLE_CONNECTED)
    {
      osal_start_timerEx( simpleBLEPeripheral_TaskID, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
    } 
  }
  else if (event == BATT_LEVEL_NOTI_DISABLED)
  {
    // stop periodic measurement
    osal_stop_timerEx( simpleBLEPeripheral_TaskID, BATT_PERIODIC_EVT );
  }
}

/*********************************************************************
 * @fn      pokemonGoPlusBattPeriodicTask
 *
 * @brief   Perform a periodic task for battery measurement.
 *
 * @param   none
 *
 * @return  none
 */
static void pokemonGoPlusBattPeriodicTask( void )
{
  if (gapProfileState == GAPROLE_CONNECTED)
  {
    // perform battery level check
    Batt_MeasLevel( );
    
    // Restart timer
    osal_start_timerEx( simpleBLEPeripheral_TaskID, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
  }
}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
  //do nothing
}

/*********************************************************************
 * @fn      pgpDeviceControlChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void pgpDeviceControlChangeCB( uint8 paramID )
{
  uint8 newValue;

  switch( paramID )
  {
    case LED_VIBRATE_CTRL_CHAR:
      PgpDeviceControl_GetParameter( LED_VIBRATE_CTRL_CHAR, &newValue );



      break;

    case FW_UPDATE_REQUEST_CHAR:
      PgpDeviceControl_GetParameter( FW_UPDATE_REQUEST_CHAR, &newValue );


      break;

    default:
      // should not reach here!
      break;
  }
}

/*********************************************************************
 * @fn      pgpCertificateChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void pgpCertificateChangeCB( uint8 paramID )
{
  uint8 newValue[36];

  switch( paramID )
  {
    case CENTRAL_TO_SFIDA_CHAR:
      PgpCertificate_GetParameter( CENTRAL_TO_SFIDA_CHAR, newValue );
      {
        uint8 data_test1[]={4,0,0,0};
        uint8 data_test2[]={5,0,0,0};
        uint8 data_test3[]={3,0,0,0};
        
        
        if (memcmp(newValue,data_test1,4)==0){
          uint8 data[]={4,0,1,0}; 
          PgpCertificate_SetParameter(SFIDA_TO_CENTRAL_CHAR, 4, data); 
          PgpCertificate_SetParameter(SFIDA_COMMANDS_CHAR, 4, data);  
        }else if (memcmp(newValue,data_test2,4)==0){
          uint8 data[]={5,0,0,0}; 
          PgpCertificate_SetParameter(SFIDA_TO_CENTRAL_CHAR, 4, data); 
          PgpCertificate_SetParameter(SFIDA_COMMANDS_CHAR, 4, data);  
        }else if (memcmp(newValue,data_test3,4)==0){
          uint8 data[]={4,0,2,0}; 
          PgpCertificate_SetParameter(SFIDA_TO_CENTRAL_CHAR, 4, data); 
          PgpCertificate_SetParameter(SFIDA_COMMANDS_CHAR, 4, data);  
        }
      }
      break;

    case SFIDA_COMMANDS_CHAR:
      PgpCertificate_GetParameter( SFIDA_COMMANDS_CHAR, newValue );
      break;

    case SFIDA_TO_CENTRAL_CHAR:
      PgpCertificate_GetParameter( SFIDA_TO_CENTRAL_CHAR, newValue );
      break;
    case SFIDA_COMMANDS_NOTIFY_SET:     //GATT_CLIENT_CHAR_CFG_UUID, set notification
      {
        uint8 data[]={3,0,0,0}; //SFIDA_RESPONSE_CERTIFICATION_NOTIFY
        PgpCertificate_SetParameter(SFIDA_TO_CENTRAL_CHAR, 4, data); 
        PgpCertificate_SetParameter(SFIDA_COMMANDS_CHAR, 4, data);  
      }
      break;
      
    default:
      // should not reach here!
      break;
  }
}

static void simpleBLEPeripheralBuzzerRing(uint8 *melody,uint8 len)
{
  /* Provide feedback that calibration is complete */
#if (defined HAL_BUZZER) && (HAL_BUZZER == TRUE)
  /* Tell OSAL to not go to sleep because buzzer uses T3 */
#if defined ( POWER_SAVING )
  osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
#endif
  /* Ring buzzer */
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_DISABLE_CLK_DIVIDE_ON_HALT );
  HalBuzzerPlay( melody, len , simpleBLEPeripheralBuzzerCompleteCback );
#endif
}

static void simpleBLEPeripheralBuzzerCompleteCback( void )
{
#if (defined HAL_BUZZER) && (HAL_BUZZER == TRUE)
  /* Tell OSAL it's OK to go to sleep */
#if defined ( POWER_SAVING )
  osal_pwrmgr_device( /*PWRMGR_ALWAYS_ON*/ PWRMGR_BATTERY );
#endif
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );
#endif
}

//Passcode callback in bonding process
static void ProcessPasscodeCB(uint8 *deviceAddr,uint16 connectionHandle,uint8 uiInputs,uint8 uiOutputs )
{
  uint32  passcode;

  #if (defined HAL_UART) && (HAL_UART == TRUE)
    HalUARTWrite ( HAL_UART_PORT_1, "ProcessPasscodeCB\n", 18 );
  #endif

  // Create random passcode
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;

  // Display passcode to user
  if ( uiOutputs != 0 )
  {
    //HalLcdWriteString( "Passcode:",  HAL_LCD_LINE_1 );
    //HalLcdWriteString( (char *) _ltoa(passcode, str, 10),  HAL_LCD_LINE_2 );
  }
  
  // Send passcode response
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
}

static void ProcessPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  
  #if (defined HAL_UART) && (HAL_UART == TRUE)
    //HalUARTWrite ( HAL_UART_PORT_1, "ProcessPairStateCB\n", 19 );
  #endif
     
  if ( state == GAPBOND_PAIRING_STATE_STARTED )  
  {  
    #if (defined HAL_UART) && (HAL_UART == TRUE)
      HalUARTWrite ( HAL_UART_PORT_1, "Pairing started\n", 16 );
    #endif 
    gPairStatus = PAIRSTATUS_NO_PAIRED;  
  }  
     
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE ){  
    if ( status == SUCCESS ){  
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        HalUARTWrite ( HAL_UART_PORT_1, "Pairing success\n", 16 );
      #endif 
      gPairStatus = PAIRSTATUS_PAIRED;  
    }  
      
    else if(status == SMP_PAIRING_FAILED_UNSPECIFIED) { 
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        HalUARTWrite ( HAL_UART_PORT_1, "Paired device\n", 14 );
      #endif 
      gPairStatus = PAIRSTATUS_PAIRED;  
    }  
      
    else  {  
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        {
          char buf[32];
          sprintf(buf,"Pairing fail: %d\n",status);
          uint8 strLength=strlen(buf);
          HalUARTWrite ( HAL_UART_PORT_1, (uint8 *)buf, strLength );
        }
      #endif  
      gPairStatus = PAIRSTATUS_NO_PAIRED;  
    }  
       
    if(gPairStatus == PAIRSTATUS_NO_PAIRED)  
    {  
      GAPRole_TerminateConnection();  
    }  
  }  
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )  
  {  
    if ( status == SUCCESS )  
    {  
      #if (defined HAL_UART) && (HAL_UART == TRUE)
        HalUARTWrite ( HAL_UART_PORT_1, "Bonding success\n", 16 );
      #endif
    }  
  }  
}

/*********************************************************************
*********************************************************************/
