/******************************************************************************

 @file  simpleGATTprofile.h

 @brief This file contains the Simple GATT profile definitions and prototypes.

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
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

#ifndef PGPDEVICECONTROLPROFILE_H
#define PGPDEVICECONTROLPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

//public static final UUID UUID_DEVICE_CONTROL_SERVICE = UUID.fromString("21c50462-67cb-63a3-5c4c-82b5b9939aeb");  
//public static final UUID UUID_LED_VIBRATE_CTRL_CHAR = UUID.fromString("21c50462-67cb-63a3-5c4c-82b5b9939aec");
//public static final UUID UUID_BUTTON_NOTIF_CHAR = UUID.fromString("21c50462-67cb-63a3-5c4c-82b5b9939aed");
//public static final UUID UUID_FW_UPDATE_REQUEST_CHAR = UUID.fromString("21c50462-67cb-63a3-5c4c-82b5b9939aef");
//public static final UUID UUID_FW_VERSION_CHAR = UUID.fromString("21c50462-67cb-63a3-5c4c-82b5b9939af0");   

// Profile Parameters
#define LED_VIBRATE_CTRL_CHAR                 0  //RW? 3 ATTR
#define BUTTON_NOTIF_CHAR                     1  //N?  4 ATTR
#define FW_UPDATE_REQUEST_CHAR                2  //RW? 3 ATTR
#define FW_VERSION_CHAR                       3  //R?  3 ATTR
    
// UUID for DEVICE_CONTROL service                          
#define DEVICE_CONTROL_SERV_UUID                  0x9AEB    
/* The 16 bit UUID listen above is only a part of the 
 * full DEVICE CONTROL 128 bit UUID:                       
 * 21C50462-67CB-63A3-5C4C-82B5B993???? */   
   
                                                      
//  Device Control service characteristic UUID              
#define LED_VIBRATE_CTRL_CHAR_UUID          0x9AEC    
#define BUTTON_NOTIF_CHAR_UUID              0x9AED 
#define FW_UPDATE_REQUEST_CHAR_UUID         0x9AEF 
#define FW_VERSION_CHAR_UUID                0x9AF0 

/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */
   
// DEVICE_CONTROL_SERVICE Base 128-bit UUID: 21c50462-67cb-63a3-5c4c-82b5b993XXXX
#define DEVICE_CONTROL_SERVICE_BASE_UUID_128( uuid )  LO_UINT16( uuid ), HI_UINT16( uuid ), 0x93, 0xb9, 0xb5, 0x82, 0x4c, 0x5c, \
                                  0xa3, 0x63, 0xcb, 0x67, 0x62, 0x04, 0xC5, 0x21

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*pgpDeviceControlChange_t)( uint8 paramID );

typedef struct
{
  pgpDeviceControlChange_t        pfnPgpDeviceControlChange;  // Called when characteristic value changes
} pgpDeviceControlCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * PgpDeviceControl_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t PgpDeviceControl_AddService( uint32 services );

/*
 * PgpDeviceControl_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t PgpDeviceControl_RegisterAppCBs( pgpDeviceControlCBs_t *appCallbacks );

/*
 * PgpDeviceControl_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t PgpDeviceControl_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * PgpDeviceControl_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t PgpDeviceControl_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* PGPDEVICECONTROLPROFILE_H */
